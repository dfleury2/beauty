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

    session_client(asio::io_context& ioc, client::client_cb&& cb) :
        session_client(ioc)
    {
        _cb = std::move(cb);
    }

    ~session_client()
    {
        // Gracefully close the socket
        boost::system::error_code ec;
        _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);

//        // not_connected happens sometimes so don't bother reporting it.
//        if(ec && ec != boost::system::errc::not_connected) {
//            return fail(ec, "shutdown");
//        }
    }

    // Start the asynchronous operation
    void run(beauty::request&& req, const beauty::url& url, const beauty::duration& d)
    {
        // Set up an HTTP GET request message
        _request = std::move(req);
        _request.version(11);
        _request.target(std::string(url.path()) + std::string(url.query()));
        _request.set(beast::http::field::host, url.host());
        _request.set(beast::http::field::user_agent, BEAUTY_PROJECT_VERSION);
        _request.prepare_payload();

        if (d.count()) {
            _timer.expires_after(d);

            _timer.async_wait(
                [me = shared_from_this()](const boost::system::error_code& ec) {
                    me->on_timer(ec);
                });
        }

        if (_socket.is_open()) {
            std::cout << "Reuse opened socket" << std::endl;

            // Send the HTTP request to the remote host
            beast::http::async_write(_socket, _request,
                    [me = shared_from_this()](boost::system::error_code ec,
                            std::size_t bytes_transferred) {
                me->on_write(ec, bytes_transferred);
            });
        }
        else {
            // Look up the domain name
            _resolver.async_resolve(
                url.host(),
                url.port(),
                [me = shared_from_this()](const boost::system::error_code& ec,
                        asio::ip::tcp::resolver::results_type results) {
                    me->on_resolve(ec, results);
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
        std::cout << "on write" << std::endl;
        if(ec) {
            return fail(ec, "write");
        }

        // Clear the response
        _response = {};

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

//        // Gracefully close the socket
//        _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
//
//        // not_connected happens sometimes so don't bother reporting it.
//        if(ec && ec != boost::system::errc::not_connected) {
//            return fail(ec, "shutdown");
//        }

        if (_cb && !_too_late) {
            _cb(ec, std::move(_response));
        }
    }

    void on_timer(boost::system::error_code ec)
    {
        if (!ec && !_too_late) {
            fail(boost::system::error_code(boost::system::errc::timed_out,
                    boost::system::system_category()),
                    "timeout");

            _too_late = true;

//            // Gracefully close the socket
//            _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        }
    }

    beauty::response& response() { return _response; }

private:
    asio::io_context&       _ioc;
    asio::ip::tcp::resolver _resolver;
    asio::ip::tcp::socket   _socket;
    asio::steady_timer      _timer;
    bool                    _too_late{false};

    beast::flat_buffer      _buffer;
    beauty::request         _request;
    beauty::response        _response;

    client::client_cb   _cb;

private:
    void fail(boost::system::error_code ec, const char* msg /* not used */) {
        if (_cb) {
            //std::cout << " !!! FAILED !!! " << ec << " with " << msg << std::endl;
            if (!_too_late) {
                _cb(ec, {});
            }
        } else {
            throw boost::system::system_error(ec);
        }
    }
};

// --------------------------------------------------------------------------
client::client_response
client::get_before(const beauty::duration& d, const std::string& url)
{
    // Set up an HTTP POST request message
    beauty::request request;
    request.method(beast::http::verb::get);

    return send_request(std::move(request), d, url);
}

// --------------------------------------------------------------------------
void
client::get_before(const beauty::duration& d, const std::string& url, client_cb&& cb)
{
    beauty::request request;
    request.method(beast::http::verb::get);

    send_request(std::move(request), d, url, std::move(cb));
}

// --------------------------------------------------------------------------
client::client_response
client::del_before(const beauty::duration& d, const std::string& url)
{
    beauty::request request;
    request.method(beast::http::verb::delete_);

    return send_request(std::move(request), d, url);
}

// --------------------------------------------------------------------------
void
client::del_before(const beauty::duration& d, const std::string& url, client_cb&& cb)
{
    beauty::request request;
    request.method(beast::http::verb::delete_);

    send_request(std::move(request), d, url, std::move(cb));
}

// --------------------------------------------------------------------------
client::client_response
client::post_before(const beauty::duration& d, const std::string& url,
        std::string&& body)
{
    beauty::request request;
    request.method(beast::http::verb::post);
    request.body() = std::move(body);

    return send_request(std::move(request), d, url);
}

// --------------------------------------------------------------------------
void
client::post_before(const beauty::duration& d, const std::string& url,
        std::string&& body, client_cb&& cb)
{
    beauty::request request;
    request.method(beast::http::verb::post);
    request.body() = std::move(body);

    send_request(std::move(request), d, url, std::move(cb));
}

// --------------------------------------------------------------------------
client::client_response
client::put_before(const beauty::duration& d, const std::string& url,
        std::string&& body)
{
    // Set up an HTTP POST request message
    beauty::request request;
    request.method(beast::http::verb::put);
    request.body() = std::move(body);

    return send_request(std::move(request), d, url);
}

// --------------------------------------------------------------------------
void
client::put_before(const beauty::duration& d, const std::string& url,
        std::string&& body, client_cb&& cb)
{
    beauty::request request;
    request.method(beast::http::verb::put);
    request.body() = std::move(body);

    send_request(std::move(request), d, url, std::move(cb));
}

// --------------------------------------------------------------------------
// Synchronous request
// --------------------------------------------------------------------------
client::client_response
client::send_request(beauty::request&& req, const beauty::duration& d,
        const std::string& url)
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

        _session->run(std::move(req), _url, d);

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
// Asynchronous request
// --------------------------------------------------------------------------
void
client::send_request(beauty::request&& req, const beauty::duration& d,
        const std::string& url, client_cb&& cb)
{
    try {
        _url = beauty::url(url);
    } catch(const std::exception& ex) {
        cb(boost::system::error_code(boost::system::errc::bad_address,
                boost::system::system_category()), {});
        return;
    }

    if (!_session) {
        // Create the session on first call...
        _session = std::make_shared<session_client>(
                beauty::application::Instance().ioc(),
                std::move(cb));
    }

    _session->run(std::move(req), _url, d);
}

}
