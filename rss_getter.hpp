#ifndef RSS_GETTER_HPP
#define RSS_GETTER_HPP

#include <string>
#include <tuple>

#include "http_connection_resources.hpp"

class rss_getter
{
public:
    rss_getter();

    std::tuple<std::string, std::string,std::string_view> get_feed(std::string url, int port);

    void download_file(std::string hostname,
                       std::string target,
                       int port,
                       std::string file_destination,
                       std::unique_lock<std::mutex> &&download_rights);

protected:
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard;
};

#endif // RSS_GETTER_HPP
