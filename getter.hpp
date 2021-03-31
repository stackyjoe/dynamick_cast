#ifndef GETTER_HPP
#define GETTER_HPP

#include <future>
#include <string>
#include <thread>
#include <tuple>

#include <boost/asio/spawn.hpp>

#include "beastly_connection.hpp"
#include "url_parser.hpp"

class getter
{
public:
    getter();
    ~getter();

    //std::future<bool> async_download(std::string url,
    //                                 std::function<void(size_t, size_t)> &&progress_handler,
    //                                 std::function<void(boost::beast::error_code const &, size_t, beastly_connection &)> &&completion_handler);


    [[nodiscard]] std::future<bool> get(std::string url,
             std::function<void(size_t, size_t)> progress_handler,
             std::function<void(boost::beast::error_code const &, size_t, beastly_connection &)> completion_handler);

    // returns size of requested file (if that is in the header provided by the server)
    // and also returns url to file
    //std::tuple<size_t, std::string> sync_get_header(std::string url);
    //std::tuple<size_t, std::string> sync_get_header(parsed_url url);

private:
    void coro_download(std::string url,
                       std::function<void(size_t, size_t)> progress_handler,
                       std::function<void(boost::beast::error_code const &, size_t, beastly_connection &)> completion_handler,
                       std::promise<bool> future,
                       boost::asio::yield_context yield_ctx);

    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard;
    std::thread run_thread;
};

#endif // GETTER_HPP
