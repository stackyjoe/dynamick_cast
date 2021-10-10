#ifndef EPISODE_HPP
#define EPISODE_HPP

#include <fstream>
#include <iostream>
#include <mutex>


//#include <boost/property_tree/ptree.hpp>
#include <pugixml.hpp>
#include "serialization.hpp"

#include "download_shared_state.hpp"
#include "shared/string_functions.hpp"

class episode {
public:
    episode() = delete;
    episode(size_t item_number, pugi::xml_node::iterator iterator);
    episode(boost::json::object const &object);

    const std::string &url() const;
    std::shared_ptr<download_shared_state> get_download_rights() const;
    std::string get_title() const;
    bool has_title(const std::string &text) const;
    
    void serialize_into(boost::json::value &val) const ;
    static episode deserialize_from(boost::json::value const &object);

    std::string get_sanitized_file_name() const noexcept;


private:
    void fill(pugi::xml_node::iterator iterator);

    std::string audio_url;
    int64_t item_number_in_xml;
    std::string description;
    std::string guid;
    std::string page_url;
    std::string publication_date;
    std::string subtitle;
    std::string title;
    mutable std::shared_ptr<download_shared_state> shared_state;

    auto member_field_name_pairs_for_serialization() noexcept {
        namespace hana = boost::hana;

        return hana::make_tuple(hana::make_pair(&audio_url, "audio_url"_bsv),
            hana::make_pair(&item_number_in_xml, "item_number"_bsv),
            hana::make_pair(&title, "title"_bsv),
            hana::make_pair(&subtitle, "subtitle"_bsv),
            hana::make_pair(&page_url, "link"_bsv),
            hana::make_pair(&guid, "guid"_bsv),
            hana::make_pair(&publication_date, "pubDate"_bsv));
    }

    auto member_field_name_pairs_for_serialization() const noexcept {
        namespace hana = boost::hana;

        return hana::make_tuple(hana::make_pair(&audio_url, "audio_url"_bsv),
            hana::make_pair(&item_number_in_xml, "item_number"_bsv),
            hana::make_pair(&title, "title"_bsv),
            hana::make_pair(&subtitle, "subtitle"_bsv),
            hana::make_pair(&page_url, "link"_bsv),
            hana::make_pair(&guid, "guid"_bsv),
            hana::make_pair(&publication_date, "pubDate"_bsv));
    }
};

#endif // EPISODE_HPP
