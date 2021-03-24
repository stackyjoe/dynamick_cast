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
};

//std::tuple<std::string,std::string,std::string,std::string> parse_url(std::string url) noexcept;

parsed_url parse(std::string url) noexcept;


#endif // URL_PARSER_HPP
