#include <iostream>
#include <fstream>
#include <functional>
#include <thread>
#include <variant>

#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/beast/version.hpp>

#include <fmt/core.h>

#include "string_functions.hpp"
#include "getter.hpp"


namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

getter::getter() :
    ioc(),
    resolver(ioc),
    guard(boost::asio::make_work_guard(ioc)),
    run_thread([this](){this->ioc.run();})
{

}

getter::~getter() {
    guard.reset();
    run_thread.join();
}

std::future<bool> getter::async_download(std::string url,
                                         HandlerStorage && handlers) {
    fmt::print("getter::async_download() called on {}\n", url);

    //auto [total, real_path] = sync_get_header(url);

    //if(real_path.empty()) {
    //    real_path = url;
    //}

    auto p = parse(url);


    auto completion_handler_wrapper = [this, completion_handler = std::move(handlers.completion_handler)]
      (boost::beast::error_code const &ec, size_t bytes_read, beastly_connection & resources) mutable -> void {
            auto status = resources.get_status();

            if(status == http::status::ok) {
                completion_handler(ec, bytes_read, resources);
                resources.set_future(!ec);
            }
            else {
                std::string url = resources.parser().base()["Location"].to_string();
                if(!url.empty()) {
                    fmt::print("Redirecting, attempting to download from: {}\n", url);
                    async_download(url, {resources.take_progress_handler(), std::move(completion_handler)});
                }
                else {
                    fmt::print("Could not find file. HTTP error code: {}\n", status);
                }
            }
    };

    auto network_resources = std::make_unique<beastly_connection>(p,
                                                                  ioc,
                                                                  resolver,
                                                                  0,//total,
                                                                  std::move(handlers.progress_handler),
                                                                  std::move(completion_handler_wrapper));

    auto result = network_resources->get_future();

    if(p.protocol == "https") {
        auto protocols = {ssl::context::tlsv11, ssl::context::tlsv12, ssl::context::tlsv13};
        for(auto a : protocols) {
            auto ec = network_resources->set_up_ssl(a, ssl::verify_peer);
            if(!ec) {
                break;
            }
            else {
                fmt::print("Algorithm ID {}: An error occurred attempting to set up SSL: {}\n", a, ec.message());
            }

        }
    }

    http::request<http::string_body> req {http::verb::get, p.total_path, 11};
    req.set(http::field::host, p.host.c_str());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    network_resources->read_file(req, std::move(network_resources));

    return result;
}

std::tuple<size_t, std::string> getter::sync_get_header(std::string url) {
    fmt::print("sync_get_header() called on {}\n", url);
    auto p = parse(url);

    auto network_resources = std::make_unique<beastly_connection>(p, ioc, resolver);

    if(p.protocol == "https") {
        auto protocols = {ssl::context::tlsv11, ssl::context::tlsv12, ssl::context::tlsv13};
        for(auto p : protocols) {
            auto ec = network_resources->set_up_ssl(p, ssl::verify_peer);
            if(!ec) {
                break;
            }
            else {
                fmt::print("Algorithm ID {}: An error occurred attempting to set up SSL: {}\n", p, ec.message());
            }

        }

    }

    http::request<http::string_body> hdr_req {http::verb::head, p.total_path, 11};
    hdr_req.set(http::field::host, p.host.c_str());
    hdr_req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);


    try {
        network_resources->send_request(std::move(hdr_req));

        http::status status = network_resources->get_status();
        switch(status) {
        case http::status::ok: {
            //std::cout << "HTTP status is OK" << std::endl;
            auto s = network_resources->parser().base()["Content-Length"].to_string();
            int size = s.empty()? 0 : std::stoi(s);
            return std::make_tuple(static_cast<size_t>(size),
                                       url);
        }
        case http::status::found: {
            //fmt::print("Found: {}\n", network_resources->parser().base()["Location"].to_string());
                    return this->sync_get_header(network_resources->parser().base()["Location"].to_string());
        }
        case http::status::moved_permanently: {
            //fmt::print("Moved permanently: {}\n", network_resources->parser().base()["Location"].to_string());
            return this->sync_get_header(network_resources->parser().base()["Location"].to_string());
        }
        case http::status::temporary_redirect: {
            //fmt::print("Temporary redirect: {}\n", network_resources->parser().base()["Location"].to_string());
            return this->sync_get_header(network_resources->parser().base()["Location"].to_string());
        }
        default: {
            //fmt::print("Received unexpected HTTP status: {}\n",status);
            return std::make_tuple(size_t {0}, std::string { });
        }
        }

    }
    catch(boost::system::system_error const &e) {
        fmt::print("An exception occurred: {},\n\t in file {} at line {}\n", e.what(), __FILE__, __LINE__);
        //std::cout << "An exception occurred: " << e.what() << std::endl;
        return std::make_tuple(size_t{0}, std::string {});
    }
}
