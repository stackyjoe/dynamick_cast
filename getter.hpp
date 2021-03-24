#ifndef GETTER_HPP
#define GETTER_HPP

#include <future>
#include <string>
#include <thread>
#include <tuple>

#include "contrib/function2/function2.hpp"

#include "beastly_connection.hpp"
#include "url_parser.hpp"

class getter
{
public:
    getter();
    ~getter();
    using HandlerStorage = beastly_connection::HandlerStorage;

    std::future<bool> async_download(std::string url,
                                     HandlerStorage &&storage);

    // returns size of requested file (if that is in the header provided by the server)
    // and also returns url to file
    std::tuple<size_t, std::string> sync_get_header(std::string url);
    std::tuple<size_t, std::string> sync_get_header(parsed_url url);



protected:
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard;
    std::thread run_thread;
};

#endif // GETTER_HPP
