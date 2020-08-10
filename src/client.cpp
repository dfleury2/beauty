#include <beauty/client.hpp>

#include <boost/system/error_code.hpp>

namespace beauty
{

// --------------------------------------------------------------------------
// Performs an HTTP GET and prints the response
// --------------------------------------------------------------------------
class session_client : public std::enable_shared_from_this<session_client>
{
public:
    explicit
    session_client(asio::io_context& ioc) :
        _ioc(ioc),
        _resolver(_ioc),
        _socket(_ioc),
        _timer(_ioc)
    {}

    // Start the asynchronous operation
    void run(const beauty::url& url, const beauty::duration& d, beast::http::verb verb, std::string&& body)
    {
        // Set up an HTTP GET request message
        _request.version(11);
        _request.method(verb);
        _request.target(std::string(url.path()) + std::string(url.query()));
        _request.set(beast::http::field::host, url.host());
        _request.set(beast::http::field::user_agent, BEAUTY_PROJECT_VERSION);
        _request.body() = std::move(body);

        // Look up the domain name
        _resolver.async_resolve(
            url.host(),
            url.port(),
            [me = shared_from_this()](const boost::system::error_code& ec,
                    asio::ip::tcp::resolver::results_type results) {
                me->on_resolve(ec, results);
            });

        if (d.count()) {
            _timer.expires_after(d);

            _timer.async_wait(
                [me = shared_from_this()](const boost::system::error_code& ec) {
                    me->on_timer(ec);
                });
        }
    }

    void on_resolve(const boost::system::error_code& ec, asio::ip::tcp::resolver::results_type results)
    {
        if(ec) {
            return fail(ec, "resolve");
        }

        // Make the connection on the IP address we get from a lookup
        asio::async_connect(
            _socket,
            results.begin(),
            results.end(),
            [me = shared_from_this()](const boost::system::error_code& ec,
                    asio::ip::tcp::resolver::iterator it) {
                me->on_connect(ec);
            }
        );
    }

    void
    on_connect(const boost::system::error_code& ec)
    {
        if(ec) {
            return fail(ec, "connect");
        }

        // Send the HTTP request to the remote host
        beast::http::async_write(_socket, _request,
                [me = shared_from_this()](boost::system::error_code ec,
                        std::size_t bytes_transferred) {
            me->on_write(ec, bytes_transferred);
        });
    }

    void on_write(boost::system::error_code ec, std::size_t)
    {
        if(ec) {
            return fail(ec, "write");
        }

        // Receive the HTTP response
        beast::http::async_read(_socket, _buffer, _response,
                [me = shared_from_this()](boost::system::error_code ec,
                        std::size_t bytes_transferred) {
            me->on_read(ec);
        });
    }

    void on_read(boost::system::error_code ec)
    {
        if(ec) {
            return fail(ec, "read");
        }

        _timer.cancel(); // will call on_timer with operator_cancelled

        // Gracefully close the socket
        _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes so don't bother reporting it.
        if(ec && ec != boost::system::errc::not_connected)
            return fail(ec, "shutdown");
    }

    void on_timer(const boost::system::error_code& ec)
    {
        if (!ec) {
            throw boost::system::system_error(boost::system::errc::timed_out,
                    boost::system::system_category());
        }
    }

    beauty::response& response() { return _response; }

private:
    asio::io_context&       _ioc;
    asio::ip::tcp::resolver _resolver;
    asio::ip::tcp::socket   _socket;
    asio::steady_timer      _timer;

    beast::flat_buffer          _buffer;
    beauty::request             _request;
    beauty::response            _response;
};

// --------------------------------------------------------------------------
std::pair<boost::system::error_code, beauty::response>
client::get_before(const beauty::duration& d, const std::string& url)
{
    boost::system::error_code ec;
    beauty::response response;

    try {
        _url = beauty::url(url);

        asio::io_context ioc;

        if (!_session) {
            // Create the session on first call...
            _session = std::make_shared<session_client>(ioc);
        }

        _session->run(_url, d, beast::http::verb::get, "");

        ioc.run();

        response = std::move(_session->response());
    }
    catch(const boost::system::system_error& ex) {
        ec = ex.code();
    }
    catch(const std::exception& ex) {
        ec = boost::system::error_code(boost::system::errc::bad_address,
                boost::system::system_category());
    }

    return std::make_pair(ec, response);
}

// --------------------------------------------------------------------------
std::pair<boost::system::error_code, beauty::response>
client::post_before(const beauty::duration& d, const std::string& url, std::string&& body)
{
    boost::system::error_code ec;
    beauty::response response;

    try {
        _url = beauty::url(url);

        asio::io_context ioc;

        if (!_session) {
            // Create the session on first call...
            _session = std::make_shared<session_client>(ioc);
        }

        _session->run(_url, d, beast::http::verb::post, std::move(body));

        ioc.run();

        response = std::move(_session->response());
    }
    catch(const boost::system::system_error& ex) {
        ec = ex.code();
    }
    catch(const std::exception& ex) {
        ec = boost::system::error_code(boost::system::errc::bad_address,
                boost::system::system_category());
    }

    return std::make_pair(ec, response);
}

}
