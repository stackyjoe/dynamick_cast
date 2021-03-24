#include "fmt/core.h"

#include "beastly_connection.hpp"


using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;

namespace {
    constexpr size_t default_download_limit = 1024*1024*512;
}


beastly_connection::beastly_connection(parsed_url url,
                                       boost::asio::io_context &io_context,
                                       boost::asio::ip::tcp::resolver & resolver)
    : completed(0),
      total(0),
      url(url),
      socket_(io_context),
      ssl_context_({}),
      buffer_(),
      parser_()
{
    parser_.body_limit(default_download_limit);

    auto const results = resolver.resolve(url.host.c_str(),
                                          url.port,
                                          boost::asio::ip::resolver_query_base::numeric_service);

    boost::asio::connect(socket_, results.begin(), results.end());

}

beastly_connection::beastly_connection(parsed_url url,
                                       boost::asio::io_context &io_context,
                                       boost::asio::ip::tcp::resolver & resolver,
                                       size_t body_size,
                                       fu2::unique_function<void(size_t,size_t)> &&progress_handler_,
                                       fu2::unique_function<void(boost::beast::error_code const &, size_t, beastly_connection &)> &&completion_handler_
                                       )
    : completed(0),
      total(body_size),
      url(url),
      socket_(io_context),
      ssl_context_({}),
      buffer_(),
      parser_(),
      progress_handler_(std::move(progress_handler_)),
      completion_handler_(std::move(completion_handler_))
{
    parser_.body_limit(std::max(body_size, default_download_limit));

    auto const results = resolver.resolve(url.host.c_str(),
                                          url.port,
                                          boost::asio::ip::resolver_query_base::numeric_service);

    boost::asio::connect(socket_, results.begin(), results.end());

}

void beastly_connection::set_ssl_stream() {
    ssl_stream_.emplace(socket_, *ssl_context_);
}

void beastly_connection::read_some(boost::system::error_code const &ec,
                                          size_t bytes_read,
                                          std::unique_ptr<beastly_connection> &&resources) {
    if(ec || resources->parser_.is_done()) {

        completion_handler_(ec, bytes_read, *resources);
        return;
    }

    completed += bytes_read;
    progress_handler_(completed, total);

    auto & skt = resources->socket_;
    auto & bfr = resources->buffer_;
    auto & prs = resources->parser_;
    auto & strm = resources->ssl_stream_;

    if(strm.has_value()) {
        http::async_read_some(*strm,
                              bfr,
                              prs,
                              [resources = std::move(resources)]
                              (boost::beast::error_code const &ec, size_t bytes_transferred) mutable {
                                  resources->read_some(ec, bytes_transferred, std::move(resources));
                              }
        );
    }
    else {
        http::async_read_some(skt,
                              bfr,
                              prs,
                              [resources = std::move(resources)]
                              (boost::beast::error_code const &ec, size_t bytes_transferred) mutable {
                                  resources->read_some(ec, bytes_transferred, std::move(resources));
                              }
        );
    }
}

std::string beastly_connection::take_body() noexcept {
    return std::exchange(parser_.get().body(), std::string());
}

std::string const & beastly_connection::body() const noexcept {
    return parser_.get().body();
}

bool beastly_connection::set_future(bool v) {
    promise.set_value(v);
    return v;
}

std::future<bool> beastly_connection::get_future() {
    return promise.get_future();
}

void beastly_connection::send_request(http::request<http::string_body> hdr_request) {
    if(ssl_stream_.has_value()) {
        http::write(ssl_stream_.value(), hdr_request);
        http::read_header(ssl_stream_.value(),
                          buffer_,
                          parser_);
    }
    else {
        http::write(socket_, hdr_request);
        http::read_header(socket_,
                          buffer_,
                          parser_);
    }

}

void beastly_connection::read_file(http::request<http::string_body> file_request, std::unique_ptr<beastly_connection> hoc) {
    if(ssl_stream_.has_value()) {
        http::write(*ssl_stream_, file_request);
        http::async_read_some(*ssl_stream_,
                              buffer_,
                              parser_,
                              [hoc=std::move(hoc)](boost::beast::error_code const &ec, size_t bytes_transferred) mutable {
                hoc->read_some(ec,bytes_transferred, std::move(hoc));
            });
    }
    else {
        http::write(socket_, file_request);

        http::async_read_some(socket_,
                              buffer_,
                              parser_,
                              [hoc=std::move(hoc)](boost::beast::error_code const &ec, size_t bytes_transferred) mutable {
                hoc->read_some(ec,bytes_transferred, std::move(hoc));
            });
    }
}

http::status beastly_connection::get_status() noexcept {
    return parser_.get().base().result();
}

boost::beast::error_code beastly_connection::set_up_ssl(boost::asio::ssl::context_base::method method, boost::asio::ssl::verify_mode mode) {
    ssl_context_.emplace(method);

    ssl_context_->set_options(ssl::context_base::default_workarounds);
    ssl_context_->set_default_verify_paths();

    set_ssl_stream();
    ssl_stream_->set_verify_mode(mode);

    // Fixes issues with CDN's such as Cloudflare, see issue #262 here:
    // https://github.com/chriskohlhoff/asio/issues/262
    SSL_set_tlsext_host_name( ssl_stream_->native_handle(), url.host.c_str() );

    boost::beast::error_code ec;
    ssl_stream_->handshake(ssl::stream_base::client, ec);
    return ec;
}

auto beastly_connection::take_progress_handler() -> decltype (progress_handler_) {
    return std::exchange(progress_handler_, [](size_t, size_t){});
}
