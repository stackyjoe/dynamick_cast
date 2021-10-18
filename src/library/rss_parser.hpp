#ifndef RSS_PARSER_HPP
#define RSS_PARSER_HPP

#include <string>
#include <string_view>
#include "podcast.hpp"

class rss_parser
{
public:

    class xml_in_rss {
        std::string whole_rss_file;
        size_t lpos;
        size_t rpos;

    public:
        std::string_view xml_portion() const noexcept { return std::string_view(whole_rss_file).substr(lpos, rpos-lpos);}

        xml_in_rss() = delete;

        xml_in_rss(std::string const & rss_file_contents) : whole_rss_file(rss_file_contents) {

            constexpr const char * channel_begin = "<channel>";
            constexpr const char * channel_end = "</channel>";

            lpos = whole_rss_file.find(channel_begin);
            rpos = whole_rss_file.find(channel_end, whole_rss_file.size() < 1000 ? 0 : whole_rss_file.size()-1000);
            rpos += length(channel_end);

            if(lpos == std::string_view::npos || rpos == std::string_view::npos) {
                lpos = rpos = 0;
            }
        }
    };

    rss_parser() = delete;
    rss_parser(std::string rss_feed) : contents(xml_in_rss(rss_feed)) { }

private:
    const xml_in_rss contents;

public:
    podcast parse(std::string backup_url) const {
        auto xml = contents.xml_portion();

        auto p = podcast(std::string{});
        pugi::xml_document doc;

        doc.load_buffer(xml.data(), xml.size());

        p.fill_from_xml(doc, backup_url);

        return p;
    }

};

#endif // RSS_PARSER_HPP
