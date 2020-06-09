#ifndef ACTIVE_DOWNLOAD_HPP
#define ACTIVE_DOWNLOAD_HPP

#include <future>
#include <mutex>
#include <optional>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include "contrib/function2/function2.hpp"

class http_connection_resources
{
public:
    http_connection_resources(boost::asio::ip::tcp::socket &&socket,
                              size_t body_size
                              );

    http_connection_resources(boost::asio::ip::tcp::socket &&socket,
                              size_t body_size,
                              fu2::unique_function<void(size_t,size_t)> &&progress_handler_,
                              fu2::unique_function<void(boost::beast::error_code const &, size_t, http_connection_resources &)> &&completion_handler_
                              );

    http_connection_resources(boost::asio::ip::tcp::socket &&socket,
                              boost::beast::http::response_parser<boost::beast::http::empty_body> &&parser);

    http_connection_resources(const http_connection_resources &) = delete;
    http_connection_resources &operator=(const http_connection_resources &) = delete;

    http_connection_resources(http_connection_resources &&) = delete;
    http_connection_resources &operator=(http_connection_resources &&) = delete;

    void read_some(boost::system::error_code const &ec,
                   size_t bytes_read,
                   std::unique_ptr<http_connection_resources> &&owning_ptr);

    void set_ssl_stream(boost::asio::ip::tcp::socket& socket, boost::asio::ssl::context &&context);

    std::atomic<size_t> completed;
    size_t total;
    size_t bytecount_when_progress_handler_was_last_called;

    boost::asio::ip::tcp::socket socket_;
    std::optional<boost::asio::ssl::context> ssl_context_;
    std::optional<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>> ssl_stream_;
    boost::beast::flat_buffer buffer_;
    boost::beast::http::response_parser<boost::beast::http::string_body> parser_;

    std::mutex progress_handler_lock;
    fu2::unique_function<void(size_t, size_t)> progress_handler_;
    fu2::unique_function<void(boost::beast::error_code const &, size_t, http_connection_resources &)> completion_handler_;

    constexpr static size_t default_download_limit = 64000000;
    constexpr static size_t bytes_between_updates = 1000000;
};

#endif // ACTIVE_DOWNLOAD_HPP
