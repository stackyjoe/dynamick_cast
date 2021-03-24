#ifndef ACTIVE_DOWNLOAD_HPP
#define ACTIVE_DOWNLOAD_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <future>
#include <mutex>
#include <optional>
#include <utility>

#include "contrib/function2/function2.hpp"

#include "url_parser.hpp"

class beastly_connection
{
public:
    struct HandlerStorage {
        fu2::unique_function<void(size_t,size_t)> progress_handler;
        fu2::unique_function<void(boost::beast::error_code const &, size_t, beastly_connection &)> completion_handler;
    };
private:
    size_t completed;
    size_t total;
    parsed_url url;
    boost::asio::ip::tcp::socket socket_;
    std::optional<boost::asio::ssl::context> ssl_context_;
    std::optional<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>> ssl_stream_;
    boost::beast::flat_buffer buffer_;
    boost::beast::http::response_parser<boost::beast::http::string_body> parser_;
    std::promise<bool> promise;
    fu2::unique_function<void(size_t,size_t)> progress_handler_;
    fu2::unique_function<void(boost::beast::error_code const &, size_t, beastly_connection &)> completion_handler_;

    void read_some(boost::system::error_code const &ec,
                   size_t bytes_read,
                   std::unique_ptr<beastly_connection> &&owning_ptr);

public:
    beastly_connection(parsed_url url, boost::asio::io_context &io_context, boost::asio::ip::tcp::resolver &resolver);
    beastly_connection(parsed_url url,
                              boost::asio::io_context &io_context, boost::asio::ip::tcp::resolver &resolver,
                              size_t body_size,
                              fu2::unique_function<void(size_t,size_t)> &&progress_handler_,
                              fu2::unique_function<void(boost::beast::error_code const &, size_t, beastly_connection &)> &&completion_handler_);

    /*
    beastly_connection(parsed_url url,
                              boost::asio::ip::tcp::socket &&socket,
                              size_t body_size
                              );

    beastly_connection(parsed_url url,
                              boost::asio::ip::tcp::socket &&socket,
                              size_t body_size,
                              std::function<void(size_t,size_t)> &&progress_handler_,
                              std::function<void(boost::beast::error_code const &, size_t, beastly_connection &)> &&completion_handler_
                              );

    beastly_connection(parsed_url,
                              boost::asio::ip::tcp::socket &&socket,
                              boost::beast::http::response_parser<boost::beast::http::empty_body> &&parser);
                              */

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

    [[nodiscard]] boost::beast::error_code set_up_ssl(boost::asio::ssl::context_base::method method, boost::asio::ssl::verify_mode mode);

    void send_request(boost::beast::http::request<boost::beast::http::string_body> hdr_request);
    void read_file(boost::beast::http::request<boost::beast::http::string_body> file_request, std::unique_ptr<beastly_connection> hoc);

    decltype (progress_handler_) take_progress_handler();

    boost::beast::http::status get_status() noexcept;

};

#endif // ACTIVE_DOWNLOAD_HPP
