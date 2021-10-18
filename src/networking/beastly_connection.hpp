#ifndef BEASTLY_CONNECTION_HPP
#define BEASTLY_CONNECTION_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <future>
#include <mutex>
#include <optional>
#include <utility>

#include "url_parser.hpp"

class beastly_connection
{
private:

    parsed_url url;
    boost::asio::ip::tcp::socket socket_;
    std::optional<boost::asio::ssl::context> ssl_context_;
    std::optional<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>> ssl_stream_;
    boost::beast::flat_buffer buffer_;
    boost::beast::http::response_parser<boost::beast::http::string_body> parser_;
    std::promise<bool> promise;

public:
    static constexpr size_t default_download_limit = 1024*1024*128;


    beastly_connection(parsed_url url,
                       boost::asio::io_context &io_context,
                       boost::asio::ip::tcp::resolver &resolver);



    beastly_connection(const beastly_connection &) = delete;
    beastly_connection &operator=(const beastly_connection &) = delete;

    beastly_connection(beastly_connection &&) = delete;
    beastly_connection &operator=(beastly_connection &&) = delete;


    void set_ssl_stream();

    std::future<bool> get_future();
    bool set_future(bool v);

    std::string take_body() noexcept;
    std::string const & body() const noexcept;
    auto const & parser() const noexcept {
        return parser_.get();
    }

    void set_parser_body_limit(size_t size_hint) {
        size_t desired = std::template max<size_t>(default_download_limit, size_hint);
        parser_.body_limit(desired);
    }

    [[nodiscard]] boost::beast::error_code set_up_ssl(boost::asio::ssl::context_base::method method, boost::asio::ssl::verify_mode mode);

    void write_request(boost::beast::http::request<boost::beast::http::string_body> hdr_request, boost::asio::yield_context yield_ctx);
    void read_header(boost::asio::yield_context yield_ctx);
    size_t async_read_some(boost::asio::yield_context yield_ctx);
    bool parser_is_done() const noexcept;

    boost::beast::http::status get_status() const noexcept;
    void set_up_certificates();
};

#endif // BEASTLY_CONNECTION_HPP
