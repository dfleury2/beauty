#pragma once

// Only there to split the client.cpp file
// Should be included only there

#include <boost/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>

#include <deque>
#include <atomic>
#include <memory>

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
    // Store all information on a request
    struct request_context {
        explicit request_context(asio::io_context& ioc) :
            timer(ioc)
        {
            response.body_limit(1024 * 1024 * 1024); // 1Go
        }
        asio::steady_timer      timer;
        bool                    too_late{false};

        beauty::url             url;
        beauty::request         request;
        beast::http::response_parser<beast::http::string_body> response;

        client::client_cb       cb;

        static
        std::shared_ptr<request_context> Create(asio::io_context& ioc, beauty::request&& req,
                const beauty::url& url, const beauty::duration& d, std::optional<client::client_cb> cb = {}) {

            // Create a request context to pass on each callback
            auto req_ctx = std::make_shared<request_context>(ioc);

            req_ctx->url = url;

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
    explicit session_client(asio::io_context& ioc) :
            _ioc(ioc),
            _resolver(_ioc),
            _socket(_ioc),
#if   (BOOST_VERSION < 107000)
          _strand(_socket.get_executor())
#else
          _strand(asio::make_strand(ioc))
#endif
    {}

    template<bool U = SSL, typename std::enable_if_t<U, int> = 0>
    session_client(asio::io_context& ioc, asio::ssl::context& ctx) :
            _ioc(ioc),
            _resolver(_ioc),
            _socket(_ioc),
            _stream(ioc, ctx),
#if   (BOOST_VERSION < 107000)
          _strand(_socket.get_executor())
#else
          _strand(asio::make_strand(ioc))
#endif
    {}

    ~session_client() {
        // Gracefully close the socket
        boost::system::error_code ec;
        if constexpr(SSL) {
            _stream.shutdown(ec);
            _stream.lowest_layer().close();
        }
        else {
            _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            _socket.close();
        }
    }

    void run(beauty::request&& req, const beauty::url& url, const beauty::duration& d)
    {
        // Create a request context to pass on each callback
        auto req_ctx = request_context::Create(_ioc, std::move(req), url, d);

        run(req_ctx);
    }

    // Start the asynchronous request
    void run(beauty::request&& req, const beauty::url& url, const beauty::duration& d,
            client::client_cb&& cb)
    {
        // Create a request context to pass on each callback
        auto req_ctx = request_context::Create(_ioc, std::move(req), url, d, std::move(cb));

        run(req_ctx);
    }

    // Start the asynchronous request
    void run(std::shared_ptr<request_context> req_ctx)
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
            //std::cout << "session_client:" << __LINE__ << " : Connection required ? " << _requests.empty() << " && " << !_stream.next_layer().is_open() << std::endl;
            connection_required = (_requests.empty() && !_stream.next_layer().is_open());
        } else {
            //std::cout << "session_client:" << __LINE__ << " : Connection required ? " << _requests.empty() << " && " << !_socket.is_open() << std::endl;
            connection_required = (_requests.empty() && !_socket.is_open());
        }
        //std::cout << "session_client:" << __LINE__ << " : Start a request, with new connection: " << connection_required << std::endl;
        _requests.push_back(req_ctx);

        if (connection_required) {
            do_resolve();
        }

        if (_pending_request) {
            //std::cout << "session_client:" << __LINE__ << " : There is a waiting connection or read ack, nothing to do to rearm" << std::endl;
        } else {
            //std::cout << "session_client:" << __LINE__ << " : No waiting connection ack or read ack, we must rearm" << std::endl;
            do_write();
        }
    }

    void do_resolve()
    {
        //std::cout << "session_client:" << __LINE__ << " : Try to resolve the host" << std::endl;

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if constexpr(SSL) {
            std::string host{(*_requests.begin())->url.host()};
            if (!SSL_set_tlsext_host_name(_stream.native_handle(), host.c_str()))
            {
                boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                             boost::asio::error::get_ssl_category()};
                return fail(**_requests.begin(), ec, "set_tlsext_hostname");
            }
        }

        _pending_request = true;

        // Get port or the default one
        auto port_view = (*_requests.begin())->url.port_view();
        if (port_view.empty()) {
            if ((*_requests.begin())->url.is_http()) {
                port_view = "80";
            }
            else if ((*_requests.begin())->url.is_https()) {
                port_view = "443";
            }
        }

        // Look up the domain name
        _resolver.async_resolve(
                (*_requests.begin())->url.host(),
                port_view,
                [me = this->shared_from_this()](const boost::system::error_code& ec,
                                                const asio::ip::tcp::resolver::results_type& results) {
                    me->on_resolve(ec, results);
                });
    }

    void on_resolve(const boost::system::error_code& ec,
            const asio::ip::tcp::resolver::results_type& results)
    {
        //std::cout << "session_client:" << __LINE__ << " : on_resolve" << std::endl;
        if (ec) {
            std::lock_guard guard{_requests_mtx};
            return fail(**_requests.begin(), ec, "resolve");
        }

        // Make the connection on the IP address we get from a lookup
        if constexpr(SSL) {
            //std::cout << "session_client:" << __LINE__ << " : Try a ssl connection" << std::endl;
            asio::async_connect(
                _stream.next_layer(),
                results.begin(),
                results.end(),
                [me = this->shared_from_this()](const boost::system::error_code& ec,
                        const asio::ip::tcp::resolver::iterator& it) {
                    me->on_connect(ec);
                }
            );
        }
        else {
            //std::cout << "session_client:" << __LINE__ << " : Try a connection" << std::endl;
            asio::async_connect(
                _socket,
                results.begin(),
                results.end(),
                [me = this->shared_from_this()](const boost::system::error_code& ec,
                        const asio::ip::tcp::resolver::iterator& it) {
                    me->on_connect(ec);
                }
            );
        }
    }

    void
    on_connect(const boost::system::error_code& ec)
    {
        //std::cout << "session_client:" << __LINE__ << " : on_connect" << std::endl;
        if (ec) {
            std::lock_guard guard{_requests_mtx};

            auto req_ctx = *_requests.begin();
            req_ctx->timer.cancel(); // will call on_timer with operator_cancelled

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
        //std::cout << "session_client:" << __LINE__ << " :do write" << std::endl;

        // No lock here, done before the call of do_write
        if (_requests.empty()) {
            //std::cout << "session_client:" << __LINE__ << " : ... nothing to do" << std::endl;
            _pending_request = false;
            return;
        }

       //std::cout << "session_client:" << __LINE__ << " :start a write" << std::endl;
       _pending_request = true;

       // Send the HTTP request to the remote host
       if constexpr(SSL) {
           beast::http::async_write(_stream, (*_requests.begin())->request,
               asio::bind_executor(_strand,
                       [me = this->shared_from_this()](boost::system::error_code ec,
                               std::size_t bytes_transferred) {
                           me->on_write(ec, bytes_transferred);
                        })
            );
       }
       else {
           beast::http::async_write(_socket, (*_requests.begin())->request,
               asio::bind_executor(_strand,
                       [me = this->shared_from_this()](boost::system::error_code ec,
                               std::size_t bytes_transferred) {
                           me->on_write(ec, bytes_transferred);
                        })
            );
       }
    }

    void
    on_write(boost::system::error_code ec, std::size_t)
    {
        //std::cout << "session_client:" << __LINE__ << " : on_write" << std::endl;
        std::lock_guard guard{_requests_mtx};

        if (ec) {
            return fail(**_requests.begin(), ec, "write");
        }

        // Receive the HTTP response
        if constexpr(SSL) {
            beast::http::async_read(_stream, _buffer, (*_requests.begin())->response,
                    asio::bind_executor(_strand,
                        [me = this->shared_from_this()](boost::system::error_code ec,
                                std::size_t bytes_transferred) {
                    me->on_read(ec);
                })
            );
        }
        else {
            beast::http::async_read(_socket, _buffer, (*_requests.begin())->response,
                    asio::bind_executor(_strand,
                        [me = this->shared_from_this()](boost::system::error_code ec,
                                std::size_t bytes_transferred) {
                    me->on_read(ec);
                })
            );
        }
    }

    void
    on_read(boost::system::error_code ec)
    {
        //std::cout << "session_client:" << __LINE__ << " : on_read" << std::endl;

        std::lock_guard guard{_requests_mtx};
        auto req_ctx = *_requests.begin();

        if (ec) {
            if (ec == beauty::http::error::end_of_stream) {
                // This is the only way to get an error on connection close from server
                // Like a keep alive which was closed
                //std::cout << "session_client:" << __LINE__ << " : on_read - end of stream detected, trying to reconnect" << std::endl;

                // Try to reconnect
                do_resolve();

                return;
            }
            else {
                return fail(**_requests.begin(), ec, "read");
            }
        }

        _requests.pop_front();
        _pending_request = false;

        req_ctx->timer.cancel(); // will call on_timer with operator_cancelled

        if (req_ctx->cb) {
            if (!req_ctx->too_late) {
                _response = req_ctx->response.release();
                req_ctx->cb(ec, std::move(_response));
                req_ctx->cb = nullptr;
            }
        }
        else {
            _response = req_ctx->response.release();
        }

        do_write();
    }

    void
    on_timer(boost::system::error_code ec, std::shared_ptr<request_context> req_ctx)
    {
        //std::cout << "session_client:" << __LINE__ << " : on_timer" << std::endl;
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
    void fail(request_context& req_ctx, boost::system::error_code ec, const char* msg /* not used */) {
        if (req_ctx.cb) {
            //std::cout << "session_client:" << __LINE__ << " : !!! FAILED !!! " << ec << " with " << msg << std::endl;
            if (!req_ctx.too_late) {
                req_ctx.cb(ec, {});
                req_ctx.cb = nullptr;
            }
        } else {
            throw boost::system::system_error(ec);
        }
    }
};

}
