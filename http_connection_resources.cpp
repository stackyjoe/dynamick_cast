#include "http_connection_resources.hpp"

http_connection_resources::http_connection_resources(boost::asio::ip::tcp::socket &&socket,
                                                    size_t parser_body_size,
                                                    std::unique_lock<std::mutex> &&download_rights)
    : _socket(std::move(socket)),
      _download_rights(std::move(download_rights))
{
    _parser.body_limit(parser_body_size);
}

