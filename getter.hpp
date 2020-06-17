#ifndef GETTER_HPP
#define GETTER_HPP

#include <iostream>
#include <fstream>
#include <functional>
#include <future>
#include <thread>
#include <tuple>
#include <variant>

#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/beast.hpp>
#include <boost/beast/version.hpp>

#include "http_connection_resources.hpp"
#include "string_functions.hpp"
#include "url_parser.hpp"


class getter
{
public:
    getter();
    ~getter();

    getter(getter const &) = delete;
    getter &operator=(getter const &) = delete;

    //getter(getter &&) = default;
    //getter &operator=(getter &&) = default;

    bool async_download(std::string url,
                        fu2::unique_function<void(size_t, size_t)> &&progress_handler,
                        fu2::unique_function<void(boost::beast::error_code const &, size_t, http_connection_resources &)> &&completion_handler);

    // returns size of requested file (if that is in the header provided by the server)
    // and also returns url to file
    std::tuple<size_t, std::string> sync_get_header(std::string url);
    std::tuple<std::string, std::string, size_t, size_t> get_feed(std::string url, int port);

protected:
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard;
    std::thread run_thread;

    void ssl_handshake(http_connection_resources *network_resources, std::string hostname);

};

#endif // GETTER_HPP
