#ifndef URL_PARSER_HPP
#define URL_PARSER_HPP

#include <string>
#include <tuple>

std::tuple<std::string,std::string,std::string,std::string> parse_url(std::string url);

#endif // URL_PARSER_HPP
