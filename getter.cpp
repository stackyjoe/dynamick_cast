#include <algorithm>
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

auto get_time_stamp() {
    return std::chrono::high_resolution_clock::now();
}

void getter::coro_download(std::string url,
                           std::function<void (size_t, size_t)> progress_handler,
                           std::function<void (const boost::beast::error_code &, size_t, beastly_connection &)> completion_handler,
                           std::promise<bool> promise,
                           boost::asio::yield_context yield_ctx) {
    boost::beast::error_code ec;
    auto protocols = {ssl::context::tlsv11, ssl::context::tlsv12, ssl::context::tlsv13};
    constexpr size_t max_redirects = 10;
    auto parsed = parse(url);

    if(parsed.host.empty()) {
        promise.set_value(false);
        return;
    }

    size_t length_hint = 0;

    std::unique_ptr<beastly_connection> network_resources;

    if(parsed.protocol == "https") {
        for(auto p : protocols) {
            network_resources = std::make_unique<beastly_connection>(parsed, ioc, resolver);
            auto ec = network_resources->set_up_ssl(p, ssl::verify_none);
            if(!ec) {
                break;
            }
            else {
                network_resources.release();
                fmt::print("Algorithm ID {}: An error occurred attempting to set up SSL: {}\n", p, ec.message());
            }
        }
    }
    else {
        network_resources = std::make_unique<beastly_connection>(parsed, ioc, resolver);
    }

    if(!network_resources) {
        fmt::print("Was not able to set up an SSL connection.\n");
        return;
    }

    fmt::print("Getting the header.\n");

    // Find the file + get a length hint to use later.
    for(size_t i = 0; i < max_redirects; ++i) {
        http::request<http::string_body> hdr_req {http::verb::head, parsed.total_path, 11};
        hdr_req.set(http::field::host, parsed.host.c_str());
        hdr_req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        network_resources->write_request(hdr_req, yield_ctx[ec]);
        if(ec) {
            fmt::print("write_request failed: {}\n", ec.message());
            return;
        }

        network_resources->read_header(yield_ctx[ec]);
        if(ec) {
            fmt::print("read_header failed: {}\n", ec.message());
        }

        auto status = network_resources->get_status();

        switch(status) {
            case http::status::ok: {
                //std::cout << "HTTP status is OK" << std::endl;
                auto s = network_resources->parser().base()["Content-Length"].to_string();
                length_hint = s.empty()? 0 : std::stoi(s);
                break;
            }
            case http::status::found: {
                parsed = parse(network_resources->parser().base()["Location"].to_string());
                continue;
            }
            case http::status::moved_permanently: {
                parsed = parse(network_resources->parser().base()["Location"].to_string());
                continue;

            }
            default: {
                fmt::print("Received unexpected HTTP status: {}\n", status);
                return;
            }

        }

    }

    network_resources.release();

    if(parsed.protocol == "https") {
        for(auto p : protocols) {
            network_resources = std::make_unique<beastly_connection>(parsed,
                                                                     ioc,
                                                                     resolver);
            auto ec = network_resources->set_up_ssl(p, ssl::verify_none);
            if(!ec) {
                break;
            }
            else {
                network_resources.release();
                fmt::print("Algorithm ID {}: An error occurred attempting to set up SSL: {}\n", p, ec.message());
            }

        }
    }
    else {
        network_resources = std::make_unique<beastly_connection>(parsed,
                                                                 ioc,
                                                                 resolver);
    }

    if(!network_resources) {
        fmt::print("Was not able to set up an SSL connection.\n");
        return;
    }

    // At this point we've found the true url and (possibly) gotten a size hint. We just need to download the file!
    for(size_t i = 0; i < max_redirects; ++i) {

        auto const start_time = get_time_stamp();
        auto print_time_taken = [start_time]() {
            auto end_time = get_time_stamp();
            std::chrono::duration<double> dur = end_time - start_time;

            fmt::print("Download took {}\n", dur.count());
        };

        network_resources->set_parser_body_limit(length_hint);

        http::request<http::string_body> file_req {http::verb::get, parsed.total_path, 11};
        file_req.set(http::field::host, parsed.host.c_str());
        file_req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        network_resources->write_request(std::move(file_req), yield_ctx[ec]);
        if(ec) {
            fmt::print("write_request failed: {}\n", ec.message());
            return;
        }

        size_t completed = 0;
        while(1) {
            size_t bytes_read = network_resources->async_read_some(yield_ctx[ec]);

            if(ec || network_resources->parser_is_done()) {
                auto status = network_resources->get_status();
                parsed_url old = parsed;

                switch(status) {
                    case http::status::ok: {
                        completion_handler(ec, bytes_read, *network_resources);
                        print_time_taken();
                        promise.set_value(true);
                        return;
                    }
                    case http::status::found: {
                        parsed = parse(network_resources->parser().base()["Location"].to_string());

                        if(old == parsed) {
                            goto final_attempt;
                        }

                        goto break_read_loop;
                    }
                    case http::status::temporary_redirect: {
                        parsed = parse(network_resources->parser().base()["Location"].to_string());
                        goto break_read_loop;
                    }

                    case http::status::moved_permanently: {
                        parsed = parse(network_resources->parser().base()["Location"].to_string());
                        goto break_read_loop;
                    }
                    default: {
                        fmt::print("Received unexpected HTTP status: {}\n", status);
                        return;
                    }
                }
            }

            completed += bytes_read;
            progress_handler(completed, length_hint);

        }
        break_read_loop:;
    }

final_attempt:
    network_resources.release();

    if(parsed.protocol == "https") {
        for(auto p : protocols) {
            network_resources = std::make_unique<beastly_connection>(parsed,
                                                                     ioc,
                                                                     resolver);
            auto ec = network_resources->set_up_ssl(p, ssl::verify_none);
            if(!ec) {
                break;
            }
            else {
                network_resources.release();
                fmt::print("Algorithm ID {}: An error occurred attempting to set up SSL: {}\n", p, ec.message());
            }

        }
    }
    else {
        network_resources = std::make_unique<beastly_connection>(parsed,
                                                                 ioc,
                                                                 resolver);
    }

    auto const start_time = get_time_stamp();
    auto print_time_taken = [start_time]() {
        auto end_time = get_time_stamp();
        std::chrono::duration<double> dur = end_time - start_time;

        fmt::print("Download took {}\n", dur.count());
    };

    network_resources->set_parser_body_limit(length_hint);

    http::request<http::string_body> file_req {http::verb::get, parsed.total_path, 11};
    file_req.set(http::field::host, parsed.host.c_str());
    file_req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    network_resources->write_request(std::move(file_req), yield_ctx[ec]);
    if(ec) {
        fmt::print("write_request failed: {}\n", ec.message());
        return;
    }

    size_t completed = 0;
    while(1) {
        size_t bytes_read = network_resources->async_read_some(yield_ctx[ec]);

        if(ec || network_resources->parser_is_done()) {
            completion_handler(ec, bytes_read, *network_resources);
            print_time_taken();
            promise.set_value(true);
            return;
        }

        completed += bytes_read;
        progress_handler(completed, length_hint);
    }


}

std::future<bool> getter::get(std::string url,
                 std::function<void (size_t, size_t)> progress_handler,
                 std::function<void (const boost::beast::error_code &, size_t, beastly_connection &)> completion_handler) {
    std::promise<bool> promise;
    auto future = promise.get_future();

    boost::asio::spawn(this->ioc,
                       [this, url, ph=std::move(progress_handler), ch=std::move(completion_handler), promise = std::move(promise)](boost::asio::yield_context yield_ctx) mutable { coro_download(url, std::move(ph), std::move(ch), std::move(promise), yield_ctx);});
    return future;
}
