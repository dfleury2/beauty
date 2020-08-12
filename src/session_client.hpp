#pragma once

// Only there to split the client.cpp file
// Should be included only there

#include <deque>
#include <atomic>

namespace beauty
{
// --------------------------------------------------------------------------
template<bool SSL>
class session_client : public std::enable_shared_from_this<session_client<SSL>>
{
public:
    using stream_type = std::conditional_t<SSL,
            asio::ssl::stream<asio::ip::tcp::socket>,
            void*>;

private:
    // Store all informations on a request
    struct request_context {
        request_context(asio::io_context& ioc) :
            timer(ioc)
        {}
        asio::steady_timer      timer;
        bool                    too_late{false};

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
    template<bool U = SSL, typename std::enable_if_t<!U, int> = 0>
    session_client(asio::io_context& ioc) :
            _ioc(ioc),
            _resolver(_ioc),
            _socket(_ioc),
            _strand(_socket.get_executor())
    {}

    template<bool U = SSL, typename std::enable_if_t<U, int> = 0>
    session_client(asio::io_context& ioc, asio::ssl::context& ctx) :
            _ioc(ioc),
            _resolver(_ioc),
            _socket(_ioc),
            _stream(ioc, ctx),
            _strand(_socket.get_executor())
    {}

    ~session_client() {
        // Gracefully close the socket
        boost::system::error_code ec;
        if constexpr(SSL) {
            _stream.shutdown(ec);
        }
        else {
            _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        }
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
                [me = this->shared_from_this(), req_ctx](const boost::system::error_code& ec) {
                    me->on_timer(ec, req_ctx);
                });
        }

        std::lock_guard guard{_requests_mtx};
        bool connection_required = false;
        if constexpr(SSL) {
            connection_required = (_requests.empty() && !_stream.next_layer().is_open());
        } else {
            connection_required = (_requests.empty() && !_socket.is_open());
        }
        //std::cout << "Start a request" << std::endl;
        _requests.push_back(req_ctx);

        if (connection_required) {
            //std::cout << "Try to resolve the host" << std::endl;

            // Set SNI Hostname (many hosts need this to handshake successfully)
            if constexpr(SSL) {
                std::string host{url.host()};
                if (!SSL_set_tlsext_host_name(_stream.native_handle(), host.c_str()))
                {
                    boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                            boost::asio::error::get_ssl_category()};
                    return fail(**_requests.begin(), ec, "set_tlsext_hostname");
                }
            }

            _pending_request = true;

            // Look up the domain name
            _resolver.async_resolve(
                url.host(),
                url.port(),
                [me = this->shared_from_this()](const boost::system::error_code& ec,
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
        if constexpr(SSL) {
            asio::async_connect(
                _stream.next_layer(),
                results.begin(),
                results.end(),
                [me = this->shared_from_this()](const boost::system::error_code& ec,
                        asio::ip::tcp::resolver::iterator it) {
                    me->on_connect(ec);
                }
            );
        }
        else {
            asio::async_connect(
                _socket,
                results.begin(),
                results.end(),
                [me = this->shared_from_this()](const boost::system::error_code& ec,
                        asio::ip::tcp::resolver::iterator it) {
                    me->on_connect(ec);
                }
            );
        }
    }

    void
    on_connect(const boost::system::error_code& ec)
    {
        //std::cout << "on connect" << std::endl;
        if(ec) {
            std::lock_guard guard{_requests_mtx};
            return fail(**_requests.begin(), ec, "connect");
        }

        if constexpr(SSL) {
            // Perform the SSL handshake
            _stream.async_handshake(
                asio::ssl::stream_base::client,
                [me = this->shared_from_this()](boost::system::error_code ec) {
                    me->on_handshake(ec);
                });
        }
        else {
            std::lock_guard guard{_requests_mtx};
            do_write();
        }
    }

    void
    on_handshake(boost::system::error_code ec) {
        if(ec) {
            std::lock_guard guard{_requests_mtx};
            return fail(**_requests.begin(), ec, "handshake");
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
       if constexpr(SSL) {
           beast::http::async_write(_stream, req_ctx->request,
               asio::bind_executor(_strand,
                       [me = this->shared_from_this(), req_ctx](boost::system::error_code ec,
                               std::size_t bytes_transferred) {
                           me->on_write(ec, bytes_transferred, req_ctx);
                        })
            );
       }
       else {
           beast::http::async_write(_socket, req_ctx->request,
               asio::bind_executor(_strand,
                       [me = this->shared_from_this(), req_ctx](boost::system::error_code ec,
                               std::size_t bytes_transferred) {
                           me->on_write(ec, bytes_transferred, req_ctx);
                        })
            );
       }
    }

    void
    on_write(boost::system::error_code ec, std::size_t, std::shared_ptr<request_context> req_ctx)
    {
        //std::cout << "on write" << std::endl;
        if(ec) {
            return fail(*req_ctx, ec, "write");
        }

        // Receive the HTTP response
        if constexpr(SSL) {
            beast::http::async_read(_stream, _buffer, req_ctx->response,
                    asio::bind_executor(_strand,
                        [me = this->shared_from_this(), req_ctx](boost::system::error_code ec,
                                std::size_t bytes_transferred) {
                    me->on_read(ec, req_ctx);
                })
            );
        }
        else {
            beast::http::async_read(_socket, _buffer, req_ctx->response,
                    asio::bind_executor(_strand,
                        [me = this->shared_from_this(), req_ctx](boost::system::error_code ec,
                                std::size_t bytes_transferred) {
                    me->on_read(ec, req_ctx);
                })
            );
        }
    }

    void
    on_read(boost::system::error_code ec, std::shared_ptr<request_context> req_ctx)
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

    void
    on_timer(boost::system::error_code ec, std::shared_ptr<request_context> req_ctx)
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
    stream_type                                   _stream = {};
    asio::strand<asio::io_context::executor_type> _strand;

    beast::flat_buffer      _buffer;

    // Synchronous response
    beauty::response        _response;

    // Asynchronous request - wait for connection
    std::mutex              _requests_mtx;
    std::deque<std::shared_ptr<request_context>> _requests;
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

}
