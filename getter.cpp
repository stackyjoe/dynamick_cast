

#include <iostream>
#include <fstream>
#include <functional>
#include <thread>
#include <variant>

#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/beast/version.hpp>

#include "string_functions.hpp"
#include "getter.hpp"
#include "url_parser.hpp"


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

bool getter::async_download(std::string url,
                            fu2::unique_function<void(size_t,size_t)> &&progress_handler,
                            fu2::unique_function<void(boost::beast::error_code const &, size_t, http_connection_resources &)> &&completion_handler) {

    auto [protocol, hostname, target, query_] = parse_url(url);
    (void)hostname;
    (void)target;
    (void)query_;

    auto [total, real_path] = sync_get_header(url);

    if(real_path.empty())
        throw std::logic_error("URL parse failed or inappropriate url passed to getter::async_download. Before parsing the URL was: " + url);


    auto [prot, host, path, query] = parse_url(real_path);

    (void)prot;

    std::string port;

    protocol == "https" ? port = "443" : port = "80";

    auto const results = resolver.resolve(host.c_str(),
                                          port,
                                          boost::asio::ip::resolver_query_base::numeric_service);

    tcp::socket socket { ioc };
    boost::asio::connect(socket, results.begin(), results.end());

    std::unique_ptr<http_connection_resources> network_resources =
            std::make_unique<http_connection_resources>(std::move(socket),
                                                        total,
                                                        std::move(progress_handler),
                                                        std::move(completion_handler)
    );

    auto & skt = network_resources->socket_;
    auto & bfr = network_resources->buffer_;
    auto & prs = network_resources->parser_;
    auto & strm = network_resources->ssl_stream_;


    //auto result = network_resources->promise.get_future();

    if(protocol == "https") {
        try {
            ssl::context ctx{ ssl::context::tlsv12};
            ctx.set_options(ssl::context_base::default_workarounds |
                            ssl::context_base::no_sslv3);
            ctx.set_default_verify_paths();

            network_resources->set_ssl_stream(network_resources->socket_, std::move(ctx));
            network_resources->ssl_stream_->set_verify_mode(ssl::verify_peer);

            network_resources->ssl_stream_->handshake(ssl::stream_base::client);

        //std::cout << "ssl handshake completed." << std::endl;
        }
        catch(std::runtime_error const &e) {
            std::cout << "An exception occurred: " << e.what();
            return false;
        }
    }

    http::request<http::string_body> req {http::verb::get, path+query, 11};
    req.set(http::field::host, host.c_str());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    //std::cout << "write request to socket" << std::endl;

    if(network_resources->ssl_stream_.has_value()) {
        http::write(*strm, req);
        http::async_read_some(*strm,
                              bfr,
                              prs,
                              [network_resources=std::move(network_resources)](boost::beast::error_code const &ec, size_t bytes_transferred) mutable {
                network_resources->read_some(ec, bytes_transferred, std::move(network_resources));
            });
    }
    else {
        http::write(skt, req);

        http::async_read_some(skt,
                              bfr,
                              prs,
                              [network_resources=std::move(network_resources)](boost::beast::error_code const &ec, size_t bytes_transferred) mutable {
                network_resources->read_some(ec, bytes_transferred, std::move(network_resources));
            });
    }

    return true;
}

std::tuple<size_t, std::string> getter::sync_get_header(std::string url) {
    auto [protocol, hostname, target, query] = parse_url(url);

    const std::string port = protocol == "https" ? "443" : "80";

    auto const results = resolver.resolve(hostname.c_str(),
                                          port,
                                          boost::asio::ip::resolver_query_base::numeric_service);

    tcp::socket socket { ioc };
    boost::asio::connect(socket, results.begin(), results.end());

    boost::beast::flat_buffer buffer;
    boost::beast::http::response_parser<boost::beast::http::empty_body> parser;
    auto network_resources = std::make_unique<http_connection_resources>(std::move(socket), std::move(parser));

    if(port == "443") {
        ssl::context ctx{ ssl::context::sslv23 };
        ctx.set_options(ssl::context_base::default_workarounds);
        ctx.set_default_verify_paths();

        network_resources->set_ssl_stream(network_resources->socket_, std::move(ctx));
        network_resources->ssl_stream_->set_verify_mode(ssl::verify_none);

        // Fixes issues with CDN's such as Cloudflare, see issue #262 here:
        // https://github.com/chriskohlhoff/asio/issues/262
        SSL_set_tlsext_host_name( network_resources->ssl_stream_->native_handle(), hostname.c_str() );

        try {
            network_resources->ssl_stream_->handshake(ssl::stream_base::client);
        }
        catch(boost::system::system_error const &e) {
            throw e;
        }

    }

    http::request<http::string_body> hdr_req {http::verb::head, target+query, 11};
    hdr_req.set(http::field::host, hostname.c_str());
    hdr_req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);


    try {

        if(network_resources->ssl_stream_.has_value()) {
            http::write(network_resources->ssl_stream_.value(), hdr_req);
            http::read_header(network_resources->ssl_stream_.value(),
                              network_resources->buffer_,
                              network_resources->parser_);
        }
        else {
            http::write(network_resources->socket_, hdr_req);
            http::read_header(network_resources->socket_,
                              network_resources->buffer_,
                              network_resources->parser_);
        }




        http::status status = network_resources->parser_.get().base().result();
        switch(status) {
        case http::status::ok: {
            //std::cout << "HTTP status is OK" << std::endl;
            auto s = network_resources->parser_.get().base()["Content-Length"].to_string();
            int size = s.empty()? 0 : std::stoi(s);
            return std::make_tuple(static_cast<size_t>(size),
                                       url);
        }
        case http::status::found: {
            return this->sync_get_header(network_resources->parser_.get().base()["Location"].to_string());
        }
        case http::status::moved_permanently: {
            return this->sync_get_header(network_resources->parser_.get().base()["Location"].to_string());

        }
        default: {
            std::cout << "Received unexpected HTTP status: " << status << std::endl;
            return std::make_tuple(size_t {0}, std::string { });
        }
        }

    }
    catch(boost::system::system_error const &e) {
        std::cout << "An exception occurred: " << e.what() << std::endl;
        return std::make_tuple(size_t{0}, std::string {});
    }
}


std::tuple<std::string, std::string, size_t, size_t> getter::get_feed(std::string url, int port) {
    auto [protocol, hostname, pathname, query ] = parse_url(url);
    (void)protocol;

    auto const results = resolver.resolve(hostname.c_str(),
                                          std::to_string(port),
                                          boost::asio::ip::resolver_query_base::numeric_service);

    //auto socket = std::make_unique<tcp::socket>(ioc);
    //boost::asio::connect(*socket, results.begin(), results.end());




    //tcp::socket socket { ioc };

    boost::beast::flat_buffer buffer;
    boost::beast::http::response_parser<boost::beast::http::empty_body> parser;
    auto network_resources = std::make_unique<http_connection_resources>(tcp::socket(ioc), std::move(parser));
    boost::asio::connect(network_resources->socket_, results.begin(), results.end());


    if(port == 443) {
        ssl::context ctx{ ssl::context::sslv23 };
        ctx.set_options(ssl::context_base::default_workarounds);
        ctx.set_default_verify_paths();

        network_resources->set_ssl_stream(network_resources->socket_, std::move(ctx));
        network_resources->ssl_stream_->set_verify_mode(ssl::verify_none);

        // Fixes issues with CDN's such as Cloudflare, see issue #262 here:
        // https://github.com/chriskohlhoff/asio/issues/262
        SSL_set_tlsext_host_name( network_resources->ssl_stream_->native_handle(), hostname.c_str() );

        try {
            network_resources->ssl_stream_->handshake(ssl::stream_base::client);
        }
        catch(boost::system::system_error const &e) {
            throw e;
        }

    }


    http::request<http::string_body> req {http::verb::get, pathname+query, 11 };
    req.set(http::field::host, hostname.c_str());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    std::string full_response;

    if(!network_resources->ssl_stream_.has_value()) {
        http::write(network_resources->socket_, req);

        //boost::beast::flat_buffer buffer;
        http::response<http::dynamic_body> response;

        http::read(network_resources->socket_, buffer, response);
        full_response = boost::beast::buffers_to_string(response.body().data());
    }
    else {
        http::write( network_resources->ssl_stream_.value(), req);
        http::response<http::dynamic_body> response;
        http::read(network_resources->ssl_stream_.value(), buffer, response);
        full_response = boost::beast::buffers_to_string(response.body().data());
    }

    //std::cout << full_response << std::endl;

    // Want to trim off unneeded text with a minimal amount of moving data. Testing with a toy
    // RSS feed with approximately 950000 bytes. The copies add up!
    constexpr const char * channel_begin = "<channel>";
    constexpr const char * channel_end = "</channel>";

    size_t lpos = full_response.find(channel_begin);
    size_t rpos = full_response.find(channel_end, full_response.size()-1000);

    if(lpos == std::string_view::npos or rpos == std::string_view::npos)
        return {hostname+pathname, std::string(), 0,0};

    rpos += length(channel_end);
    //std::string_view xml_segment(std::string_view(full_response).substr(lpos,rpos-lpos));

    return {hostname+pathname, full_response, lpos, rpos-lpos};
}

