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
    constexpr static size_t number_of_threads=2;

    getter();
    ~getter();

    std::unique_ptr<beastly_connection> make_connection(parsed_url uri);

    template<class Callable>
    void post(Callable && c) {
        boost::asio::post(ioc.get_executor(), c);
    }

    [[nodiscard]] std::future<bool> get(std::string url,
             std::function<void(size_t, size_t)> progress_handler,
             std::function<void(boost::beast::error_code const &, size_t, beastly_connection &)> completion_handler);

private:
    void coro_download(std::string url,
                       std::function<void(size_t, size_t)> progress_handler,
                       std::function<void(boost::beast::error_code const &, size_t, beastly_connection &)> completion_handler,
                       std::promise<bool> future,
                       boost::asio::yield_context yield_ctx);

    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard;
    std::unique_ptr<std::thread[]>  networking_threads;
};

#endif // GETTER_HPP
