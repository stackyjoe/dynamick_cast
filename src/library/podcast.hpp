#ifndef PODCAST_HPP
#define PODCAST_HPP


#include <string>
#include <vector>

#include "episode.hpp"

class podcast
{
public:
    podcast() = delete;
    ~podcast() = default;

    podcast(const podcast &) = delete;
    podcast &operator=(const podcast &) = delete;

    podcast(podcast &&) = default;
    podcast &operator=(podcast &&) = default;

    explicit podcast(std::string url);

    size_t episode_count() const noexcept;
    std::string const * find_url(const std::string &title);

    //void fill_from_json(boost::json::object const &parsed_json);
    void fill_from_xml(pugi::xml_document &parsed_xml, std::string backup_url);
    episode * get_episode(const std::string &title);

    void serialize_into(boost::json::value &val) const;
    static podcast deserialize_from(boost::json::value const &obj);

    const std::string & title() const;
    std::string rss_url();

    std::vector<episode> const & peek_items() const noexcept;

private:
    std::string _author;
    std::string _last_build_date;
    std::string _managing_editor;
    std::string _summary;
    std::string _title;
    std::string _guid;
    std::string rss_feed_url;
    
    std::vector<episode> items;

    auto member_field_name_pairs_for_serialization() noexcept {
        namespace hana = boost::hana;
        return hana::make_tuple(hana::make_pair(&_author, "author"_bsv),
            hana::make_pair(&_last_build_date, "lastBuildDate"_bsv),
            hana::make_pair(&_managing_editor, "managingEditor"_bsv),
            hana::make_pair(&_summary, "itunes:summary"_bsv),
            hana::make_pair(&_title, "title"_bsv),
            hana::make_pair(&_guid, "guid"_bsv),
            hana::make_pair(&rss_feed_url, "rss_feed_url"_bsv),
            hana::make_pair(&items, "items"_bsv));
    }
    auto member_field_name_pairs_for_serialization() const noexcept {
        namespace hana = boost::hana;
        return hana::make_tuple(hana::make_pair(&_author, "author"_bsv),
            hana::make_pair(&_last_build_date, "lastBuildDate"_bsv),
            hana::make_pair(&_managing_editor, "managingEditor"_bsv),
            hana::make_pair(&_summary, "itunes:summary"_bsv),
            hana::make_pair(&_title, "title"_bsv),
            hana::make_pair(&_guid, "guid"_bsv),
            hana::make_pair(&rss_feed_url, "rss_feed_url"_bsv),
            hana::make_pair(&items, "items"_bsv));
    }

};

#endif // PODCAST_HPP
