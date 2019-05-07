//#include <QThread>

#include <boost/asio/connect.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <fstream>
#include <thread>

#include "string_functions.hpp"
#include "rss_getter.hpp"
#include "url_parser.hpp"


namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;

rss_getter::rss_getter() :
    ioc(),
    resolver(ioc),
    guard(boost::asio::make_work_guard(ioc))
{
    // guard object keeps io_context form finishing working. We just want a simple network daemon so we
    // won't worry about telling it to stop.
    std::thread thd([this](){this->ioc.run();});
    thd.detach();
}


///
/// \brief rss_getter::get makes a http get requehttps://legion.stanford.edu/st for a target at a hostname.
/// \param hostname
/// \param target
/// \param port
/// \return Either throws a std::exception, or returns a string containing the result.
///

std::tuple<std::string, std::string, std::string_view> rss_getter::get_feed(std::string url, int port) {
    auto [protocol, hostname, pathname, query ] = parse_url(url);


    auto const results = resolver.resolve(hostname.c_str(),
                                          std::to_string(port),
                                          boost::asio::ip::resolver_query_base::numeric_service);

    auto socket = std::make_unique<tcp::socket>(ioc);
    boost::asio::connect(*socket, results.begin(), results.end());

    http::request<http::string_body> req {http::verb::get, pathname+query, 11 };
    req.set(http::field::host, hostname.c_str());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(*socket, req);

    boost::beast::flat_buffer buffer;
    http::response<http::dynamic_body> response;

    http::read(*socket, buffer, response);
    std::string full_response = boost::beast::buffers_to_string(response.body().data());

    // Want to trim off unneeded text with a minimal amount of moving data. Testing with a toy
    // RSS feed with approximately 950000 bytes. The copies add up!
    constexpr const char * channel_begin = "<channel>";
    constexpr const char * channel_end = "</channel>";

    size_t lpos = full_response.find(channel_begin);
    size_t rpos = full_response.find(channel_end, full_response.size()-1000);

    if(lpos == std::string_view::npos or rpos == std::string_view::npos)
        return {hostname+pathname, std::string(), std::string_view()};

    rpos += length(channel_end);
    std::string_view xml_segment(std::string_view(full_response).substr(lpos,rpos-lpos));

    return {hostname+pathname, full_response, xml_segment};
}

void rss_getter::download_file(std::string hostname,
                               std::string target,
                               int port,
                               std::string file_destination,
                               std::unique_lock<std::mutex> &&download_rights) {

    std::cout << "Attempting to resolve " << hostname << target << std::endl;
    auto const results = resolver.resolve(hostname.c_str(),
                                          std::to_string(port),
                                          boost::asio::ip::resolver_query_base::numeric_service);
    auto socket = tcp::socket(ioc);

    boost::asio::connect(socket, results.begin(), results.end());

    http::request<http::string_body> req {http::verb::get, target, 11 };
    req.set(http::field::host, hostname.c_str());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(socket,req);


    // We're transferring the ownership of the socket.
    auto network_resources = std::make_unique<http_connection_resources>(std::move(socket), 1024*1024*1024, std::move(download_rights));

    http::async_read(network_resources->_socket,
                     network_resources->_buffer,
                     network_resources->_parser,
                     [file_destination, this, network_resources = std::move(network_resources)]
                     (boost::beast::error_code const &ec, [[maybe_unused]] size_t bytes_transferred )
                     // Let's the lambda capture by non-const reference
                     // (needed to pass lock along recursively)
                     mutable
    {
        if(ec)
            std::cout << "Error in boost::beast::http::async_read." << std::endl;

        using std::string_literals::operator""s;
        http::status status = network_resources->_parser.get().base().result();
        switch(status) {
        case http::status::ok: {
            std::cout << "Received response, size is: " << network_resources->_parser.get().body().size() << std::endl;

            std::ofstream output_file(file_destination, std::ios::binary);

            output_file << network_resources->_parser.get().body();
            output_file.close();

            std::cout << "Done writing to file." << std::endl;
            return;
        }
        case http::status::found: {
            std::cout << "Desired file found at " << network_resources->_parser.get().base()["Location"].to_string() << std::endl;
            auto [protocol, hostname, pathname, query] = parse_url(network_resources->_parser.get().base()["Location"].to_string());
            this->download_file(hostname,
                                query.empty() ? pathname : pathname + query,
                                80,
                                file_destination,
                                std::move(network_resources->_download_rights)
                                );

            return;
        }
        case http::status::moved_permanently: {
            std::cout << "URL permanently moved to " << network_resources->_parser.get().base()["Location"].to_string() << std::endl;
            auto [protocol, hostname, pathname, query] = parse_url(network_resources->_parser.get().base()["Location"].to_string());
            this->download_file(hostname,
                                query.empty() ? pathname : pathname + query,
                                80,
                                file_destination,
                                std::move(network_resources->_download_rights)
                                );
            return;
        }
        default: {
            std::cout << "Received unexpected HTTP status " << status << std::endl;

            return;
        }
        }
    });
}
