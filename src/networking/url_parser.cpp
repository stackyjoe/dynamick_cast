#include <boost/bind.hpp>
#include <boost/phoenix/object/construct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/lex.hpp>

#include <functional>

#include <fmt/core.h>

#include "url_parser.hpp"

parsed_url parse(std::string url) noexcept {

    parsed_url desiderata;

    using boost::spirit::lex::_start;
    using boost::spirit::lex::_end;

    if( boost::spirit::x3::phrase_parse(url.begin(),
                                        url.end(),
                                        (-((+boost::spirit::x3::alpha) >> "://")
                                                                   [ ([&](auto &ctx) mutable{ auto str = boost::optional<std::string>(boost::spirit::x3::_attr(ctx)); desiderata.protocol = str.has_value() ? str.value() : std::string();}) ])
                                                    >> (+(boost::spirit::x3::char_ - (boost::spirit::x3::char_("[/?]"))))[([&desiderata](auto &ctx) mutable {
                                                                                                                           auto str = boost::optional<std::string>(boost::spirit::x3::_attr(ctx));

                                                                                                                           if(! str.has_value()) {
                                                                                                                               desiderata.host = "";
                                                                                                                               desiderata.port = "";
                                                                                                                               return;
                                                                                                                           }

                                                                                                                           const std::string authority_text = str.value();

                                                                                                                           auto colon_itr = std::find(authority_text.begin(), authority_text.end(), ':');

                                                                                                                           if(colon_itr == authority_text.end()) {
                                                                                                                               desiderata.host = authority_text;
                                                                                                                               desiderata.port = desiderata.protocol == "http" ? "80" : "443";
                                                                                                                               return;
                                                                                                                           }

                                                                                                                           auto const host = std::string(authority_text.begin(), colon_itr);
                                                                                                                           auto const port = std::string(colon_itr+1, authority_text.end());

                                                                                                                           desiderata.host = host;
                                                                                                                           desiderata.port = std::to_string(std::atoi(port.c_str()));

                                                                                                                       })]
                                                    >> -(
                                                        (+(boost::spirit::x3::char_ - boost::spirit::x3::char_("?")))[ ([&](auto &ctx) mutable {auto str = boost::optional<std::string>(boost::spirit::x3::_attr(ctx)); desiderata.path = str.has_value()? str.value() : std::string();})]
                                                    )
                                                    >> -(
                                                        (+(boost::spirit::x3::char_ - boost::spirit::x3::char_("#")))[([&](auto &ctx){auto str = boost::optional<std::string>(boost::spirit::x3::_attr(ctx)); desiderata.query = str.has_value() ? str.value() : std::string();})]
                                                    )
                                         >> -((+(boost::spirit::x3::char_))[([&]([[maybe_unused]] auto &ctx){auto str = boost::optional<std::string>(boost::spirit::x3::_attr(ctx)); desiderata.fragment = str.has_value() ? str.value() : std::string();})])
                                        ,
                                        boost::spirit::x3::space)) {
        // If parse was successful
        desiderata.total_path = desiderata.path + desiderata.query + desiderata.fragment;
        return desiderata;
    }

    fmt::print("Failed to parse: {}\n", url);

    return desiderata;
}
