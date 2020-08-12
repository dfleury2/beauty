#include <beauty/client.hpp>

#include <boost/system/error_code.hpp>

#include <list>
#include <atomic>

namespace beauty
{

// --------------------------------------------------------------------------
// Performs an HTTP GET and prints the response
// --------------------------------------------------------------------------
class session_client : public std::enable_shared_from_this<session_client>
{
private:
    // Store all informations mandatory to be asynchronous
    struct request_context {
        request_context(asio::io_context& ioc) :
            timer(ioc)
        {}
        asio::steady_timer      timer;
        bool                    too_late{false};

        beast::flat_buffer      buffer;
        beauty::request         request;
        beauty::response        response;

        client::client_cb       cb;

        static
        std::shared_ptr<request_context> Create(asio::io_context& ioc, beauty::request&& req,
                const beauty::url& url, const beauty::duration& d, std::optional<client::client_cb> cb = {}) {

            // Create a request context to pass on each callback
            auto req_ctx = std::make_shared<request_context>(ioc);

            // Set up an HTTP GET request message
            req_ctx->request = std::move(req);
            req_ctx->request.version(11);
            req_ctx->request.target(std::string(url.path()) + std::string(url.query()));
            req_ctx->request.set(beast::http::field::host, url.host());
            req_ctx->request.set(beast::http::field::user_agent, BEAUTY_PROJECT_VERSION);
            req_ctx->request.prepare_payload();

            if (cb) {
                req_ctx->cb = std::move(*cb);
            }

            if (d.count()) {
                req_ctx->timer.expires_after(d);
            }

            return req_ctx;
        }
    };

public:
    explicit
    session_client(asio::io_context& ioc) :
        _ioc(ioc),
        _resolver(_ioc),
        _socket(_ioc),
        _strand(_socket.get_executor())
    {}

    ~session_client() {
        // Gracefully close the socket
        boost::system::error_code ec;
        _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    }

    void run(beauty::request&& req, const beauty::url& url, const beauty::duration& d)
    {
        // Create a request context to pass on each callback
        auto req_ctx = request_context::Create(_ioc, std::move(req), url, d);

        run(req_ctx, url);
    }

    // Start the asynchronous request
    void run(beauty::request&& req, const beauty::url& url, const beauty::duration& d,
            client::client_cb&& cb)
    {
        // Create a request context to pass on each callback
        auto req_ctx = request_context::Create(_ioc, std::move(req), url, d, std::move(cb));

        run(req_ctx, url);
    }

    // Start the asynchronous request
    void run(std::shared_ptr<request_context> req_ctx, const beauty::url& url)
    {
        if (req_ctx->timer.expiry() != asio::steady_timer::time_point()) {
            req_ctx->timer.async_wait(
                [me = shared_from_this(), req_ctx](const boost::system::error_code& ec) {
                    me->on_timer(ec, req_ctx);
                });
        }

        std::lock_guard guard{_requests_mtx};
        bool connection_required = (_requests.empty() && !_socket.is_open());
        //std::cout << "Start a request" << std::endl;
        _requests.push_back(req_ctx);

        if (connection_required) {
            //std::cout << "Try to resolve the host" << std::endl;

            _pending_request = true;

            // Look up the domain name
            _resolver.async_resolve(
                url.host(),
                url.port(),
                [me = shared_from_this()](const boost::system::error_code& ec,
                        asio::ip::tcp::resolver::results_type results) {
                    me->on_resolve(ec, results);
                });
        }

        if (!_pending_request) {
            //std::cout << "should do something to rearm" << std::endl;
            do_write();
        } else {
            //std::cout << "nothing to do to rearm" << std::endl;
        }

    }

    void on_resolve(const boost::system::error_code& ec,
            asio::ip::tcp::resolver::results_type results)
    {
        //std::cout << "on resolve" << std::endl;
        if(ec) {
            std::lock_guard guard{_requests_mtx};
            return fail(**_requests.begin(), ec, "resolve");
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
        //std::cout << "on connect" << std::endl;
        if(ec) {
            std::lock_guard guard{_requests_mtx};
            return fail(**_requests.begin(), ec, "connect");
        }

        std::lock_guard guard{_requests_mtx};
        do_write();
    }

    void
    do_write() {
        //std::cout << "do write" << std::endl;

        if (_requests.empty()) {
            //std::cout << " ... nothing to do" << std::endl;
            _pending_request = false;
            return;
        }

        auto req_ctx = *_requests.begin();
       _requests.pop_front();

       //std::cout << "start a write" << std::endl;
       _pending_request = true;

       // Send the HTTP request to the remote host
        beast::http::async_write(_socket, req_ctx->request,
            asio::bind_executor(_strand,
                [me = shared_from_this(), req_ctx](boost::system::error_code ec,
                        std::size_t bytes_transferred) {
                    me->on_write(ec, bytes_transferred, req_ctx);
                })
        );
    }

    void
    on_write(boost::system::error_code ec, std::size_t,
            std::shared_ptr<request_context> req_ctx)
    {
        //std::cout << "on write" << std::endl;
        if(ec) {
            return fail(*req_ctx, ec, "write");
        }

        // Receive the HTTP response
        beast::http::async_read(_socket, req_ctx->buffer, req_ctx->response,
                asio::bind_executor(_strand,
                    [me = shared_from_this(), req_ctx](boost::system::error_code ec,
                            std::size_t bytes_transferred) {
                me->on_read(ec, req_ctx);
            })
        );
    }

    void on_read(boost::system::error_code ec,
            std::shared_ptr<request_context> req_ctx)
    {
        //std::cout << "on read" << std::endl;
        if(ec) {
            return fail(*req_ctx, ec, "read");
        }

        req_ctx->timer.cancel(); // will call on_timer with operator_cancelled

        if (req_ctx->cb) {
            if (!req_ctx->too_late) {
                req_ctx->cb(ec, std::move(req_ctx->response));
            }
        } else {
            _response = std::move(req_ctx->response);
        }

        std::lock_guard guard{_requests_mtx};
        do_write();
    }

    void on_timer(boost::system::error_code ec,
            std::shared_ptr<request_context> req_ctx)
    {
        //std::cout << "on timer" << std::endl;
        if (!ec && !req_ctx->too_late) {
            fail(*req_ctx, boost::system::error_code(boost::system::errc::timed_out,
                    boost::system::system_category()),
                    "timeout");

            req_ctx->too_late = true;
        }
    }

    beauty::response& response() { return _response; }

private:
    asio::io_context&       _ioc;
    asio::ip::tcp::resolver _resolver;
    asio::ip::tcp::socket   _socket;
    asio::strand<asio::io_context::executor_type> _strand;

    // Synchronous response
    beauty::response        _response;

    // Asynchronous request - wait for connection
    std::mutex              _requests_mtx;
    std::list<std::shared_ptr<request_context>> _requests;
    std::atomic_bool        _pending_request{false};

private:
    void fail(const request_context& req_ctx, boost::system::error_code ec, const char* msg /* not used */) {
        if (req_ctx.cb) {
            //std::cout << " !!! FAILED !!! " << ec << " with " << msg << std::endl;
            if (!req_ctx.too_late) {
                req_ctx.cb(ec, {});
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

        if (!_session) {
            // Create the session on first call...
            _session = std::make_shared<session_client>(_sync_ioc);
        }

        _session->run(std::move(req), _url, d);

        _sync_ioc.run();
        _sync_ioc.restart();

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

        if (!_session) {
            // Create the session on first call...
            _session = std::make_shared<session_client>(
                    beauty::application::Instance().ioc());
        }

        _session->run(std::move(req), _url, d, std::move(cb));
    }
    catch(const boost::system::system_error& ex) {
        cb(ex.code(), {});
    }
    catch(const std::exception& ex) {
        cb(boost::system::error_code(boost::system::errc::bad_address,
                boost::system::system_category()), {});
        return;
    }

}

}
