#include "fmt/core.h"

#include "beastly_connection.hpp"


using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;


beastly_connection::beastly_connection(parsed_url url,
                                       boost::asio::io_context &io_context,
                                       boost::asio::ip::tcp::resolver & resolver)
    : url(url),
      socket_(io_context),
      ssl_context_({}),
      buffer_(),
      parser_()
{
    parser_.body_limit(default_download_limit);

    auto const results = resolver.resolve(url.host.c_str(),
                                          url.port,
                                          boost::asio::ip::resolver_query_base::numeric_service);

    boost::asio::connect(socket_, results.begin(), results.end());

}

void beastly_connection::set_ssl_stream() {
    ssl_stream_.emplace(socket_, *ssl_context_);
}


std::string beastly_connection::take_body() noexcept {
    return std::exchange(parser_.get().body(), std::string());
}

std::string const & beastly_connection::body() const noexcept {
    return parser_.get().body();
}

bool beastly_connection::set_future(bool v) {
    promise.set_value(v);
    return v;
}

std::future<bool> beastly_connection::get_future() {
    return promise.get_future();
}

void beastly_connection::write_request(boost::beast::http::request<boost::beast::http::string_body> request, boost::asio::yield_context yield_ctx) {
    if(ssl_stream_.has_value()) {
        http::async_write(*ssl_stream_, request, yield_ctx);
    }
    else {
        http::async_write(socket_, request, yield_ctx);
    }
}

http::status beastly_connection::get_status() const noexcept {
    return parser_.get().base().result();
}

boost::beast::error_code beastly_connection::set_up_ssl(boost::asio::ssl::context_base::method method, boost::asio::ssl::verify_mode mode) {
    ssl_context_.emplace(method);
    ssl_context_->set_options(ssl::context_base::default_workarounds);
    set_up_certificates();

    set_ssl_stream();
    ssl_stream_->set_verify_mode(mode);

    // Fixes issues with CDN's such as Cloudflare, see issue #262 here:
    // https://github.com/chriskohlhoff/asio/issues/262
    SSL_set_tlsext_host_name( ssl_stream_->native_handle(), url.host.c_str() );

    boost::beast::error_code ec;
    ssl_stream_->handshake(ssl::stream_base::client, ec);
    return ec;
}

void beastly_connection::read_header(boost::asio::yield_context yield_ctx) {
    if(ssl_stream_.has_value()) {
        http::async_read_header(ssl_stream_.value(),
                                buffer_,
                                parser_,
                                yield_ctx);
    }
    else {
        http::async_read_header(socket_,
                          buffer_,
                          parser_,
                          yield_ctx);
    }
}

size_t beastly_connection::async_read_some(boost::asio::yield_context yield_ctx) {

    if(ssl_stream_.has_value()) {
        return http::async_read_some(*ssl_stream_,
                                     buffer_,
                                     parser_,
                                     yield_ctx);
    }
    else {
        return http::async_read_some(socket_,
                                     buffer_,
                                     parser_,
                                     yield_ctx);
    }

}

#ifdef _WIN64
#include <wincrypt.h>

#pragma comment (lib, "crypt32")
#endif

void beastly_connection::set_up_certificates() {
#ifdef __unix__
    ssl_context_->set_default_verify_paths();
    return;
#endif
#ifdef _WIN64
    fmt::print("Windows cert\n");
    // Starting from code at this link
    // https://stackoverflow.com/a/40046425
    HCERTSTORE hStore = CertOpenSystemStore(0, "ROOT");
    if (hStore == nullptr) {
        return;
    }

    X509_STORE* store = X509_STORE_new();
    PCCERT_CONTEXT pContext = NULL;
    while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != nullptr) {
        X509* x509 = d2i_X509(nullptr,
            (const unsigned char**)&pContext->pbCertEncoded,
            pContext->cbCertEncoded);
        if (x509 != nullptr) {
            X509_STORE_add_cert(store, x509);
            X509_free(x509);
        }
    }

    CertFreeCertificateContext(pContext);
    CertCloseStore(hStore, 0);

    SSL_CTX_set_cert_store(ssl_context_->native_handle(), store);
    return;
#endif
    fmt::print("Should not run!\n");
}

bool beastly_connection::parser_is_done() const noexcept {
    return parser_.is_done();
}
