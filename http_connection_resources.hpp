#ifndef ACTIVE_DOWNLOAD_HPP
#define ACTIVE_DOWNLOAD_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <mutex>

///
/// \brief The active_download class functions to manage the lifetime of the resources needed for an active download.
///
/// It uses a Qt thread for simplicity, since signals can't be sent or received from std::threads.
class http_connection_resources
{
public:
    http_connection_resources(boost::asio::ip::tcp::socket &&socket,
                              size_t body_size,
                              std::unique_lock<std::mutex> &&download_rights);

    http_connection_resources(const http_connection_resources &) = delete;
    http_connection_resources &operator=(const http_connection_resources &) = delete;

    http_connection_resources(http_connection_resources &&) = delete;
    http_connection_resources &operator=(http_connection_resources &&) = delete;

    boost::asio::ip::tcp::socket _socket;
    boost::beast::flat_buffer _buffer;
    boost::beast::http::response_parser<boost::beast::http::string_body> _parser;
    std::unique_lock<std::mutex> _download_rights;
};

#endif // ACTIVE_DOWNLOAD_HPP
