#include "http_connection_resources.hpp"

#include <iostream>


using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;

http_connection_resources::http_connection_resources(tcp::socket &&socket,
                                                    size_t total
                                                    )
    : completed(0),
      total(total),
      socket_(std::move(socket)),
      ssl_context_(std::nullopt)
      //promise()
{
    parser_.body_limit(std::max(total, default_download_limit));
}

http_connection_resources::http_connection_resources(tcp::socket &&socket,
                                                     size_t total,
                                                     fu2::unique_function<void(size_t,size_t)> &&progress_handler,
                                                     fu2::unique_function<void(boost::beast::error_code const &, size_t, http_connection_resources &)> &&completion_handler
                                                    )
    : completed(0),
      total(total),
      socket_(std::move(socket)),
      ssl_context_(std::nullopt),
      progress_handler_(std::move(progress_handler)),
      completion_handler_(std::move(completion_handler))
{
    parser_.body_limit(std::max(total, default_download_limit));
}

http_connection_resources::http_connection_resources(tcp::socket &&socket,
                                                     http::response_parser<http::empty_body> &&parser)
    : socket_(std::move(socket)),
      ssl_context_(std::nullopt),
      parser_(std::move(parser))
      //promise()
{
    parser_.skip(true);
}

void http_connection_resources::set_ssl_stream(tcp::socket &socket, ssl::context &&context) {
    ssl_context_.emplace(std::move(context));
    ssl_stream_.emplace(socket, ssl_context_.value());
}

void http_connection_resources::read_some(boost::system::error_code const &ec,
                                          size_t bytes_read,
                                          std::unique_ptr<http_connection_resources> &&resources) {
    if(ec || resources->parser_.is_done()) {
        completion_handler_(ec, bytes_read, *resources);
        return;
    }

    completed = std::max(completed+0, completed+bytes_read);
    size_t cmpl = completed;

    if(std::unique_lock<std::mutex> l(progress_handler_lock, std::try_to_lock); l.owns_lock() and cmpl >= bytecount_when_progress_handler_was_last_called + bytes_between_updates) {

        bytecount_when_progress_handler_was_last_called = std::max(bytecount_when_progress_handler_was_last_called+0, cmpl);

        progress_handler_(cmpl, total);
    }

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
