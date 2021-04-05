#ifndef URL_PARSER_HPP
#define URL_PARSER_HPP

#include <optional>
#include <string>
#include <tuple>
#include <variant>


struct parsed_url {
	std::string protocol;
	std::string host;
	std::string port;
	std::string path;
    std::string total_path;
	std::string query;
	std::string fragment;

    bool operator==(parsed_url const &other) const noexcept {
        return (protocol == other.protocol) &&
                (host == other.host) && (total_path == other.total_path);
    }
};


parsed_url parse(std::string url) noexcept;


#endif // URL_PARSER_HPP
