//#include <QStandardItemModel>

#include <fmt/core.h>

#include "shared/string_functions.hpp"
#include "podcast.hpp"

using namespace std::string_literals;

podcast::podcast(std::string url) : rss_feed_url(url) { }

size_t podcast::episode_count() const noexcept {
    return items.size();
}

podcast podcast::deserialize_from(boost::json::value const &val) {
    auto const & obj = val.as_object();
    podcast value{std::string()};

    boost::hana::for_each(value.member_field_name_pairs_for_serialization(),[obj](auto pair){
        auto * member = boost::hana::first(pair);
        auto field_name = boost::hana::second(pair);

        auto itr = obj.find(field_name);

        if(itr == obj.end())
            throw std::runtime_error("Field not found");

        deserialize(member, itr->value());
    });

    return value;
}

void podcast::fill_from_xml(pugi::xml_document &parsed_xml, std::string backup_url) {
    auto itr = parsed_xml.children().begin()->begin();
    auto end = parsed_xml.children().begin()->end();

    while(itr != end) {
        switch(hash(itr->name())) {
        case hash("rss_feed_url"):
            if(strcmp(itr->name(), "rss_feed_url") == 0)
                //itr->name();
                rss_feed_url = itr->text().get();
            break;
        case hash("itunes:new-feed-url"):
            if(strcmp(itr->name(), "itunes:new-feed-url")==0) {
                auto predicate_rel = [](pugi::xml_attribute attr) -> bool{
                    if(strcmp(attr.name(),"rel")==0)
                        return true;
                    return false;
                };

                auto predicate_href = [](pugi::xml_attribute attr) -> bool{
                    if(strcmp(attr.name(),"href")==0)
                        return true;
                    return false;
                };


                auto results_rel = itr->find_attribute(predicate_rel);
                auto results_href = itr->find_attribute(predicate_href);

                if(strcmp(results_rel.value(), "self") == 0) {
                    rss_feed_url = results_href.value();
                }

            }
            break;
        case hash("atom:link"):
            if(strcmp(itr->name(), "atom:link") == 0) {
                auto predicate_rel = [](pugi::xml_attribute attr) -> bool{

                    if(strcmp(attr.name(),"rel")==0)
                        return true;
                    return false;
                };

                auto predicate_href = [](pugi::xml_attribute attr) -> bool{
                    if(strcmp(attr.name(),"href")==0)
                        return true;
                    return false;
                };

                auto results_rel = itr->find_attribute(predicate_rel);
                auto results_href = itr->find_attribute(predicate_href);

                if(strcmp(results_rel.value(), "self") == 0) {
                    rss_feed_url = results_href.value();
                }


            }
            break;
        case hash("title"):
            if(strcmp(itr->name(), "title")==0)
                _title = itr->text().get();
            break;
        case hash("author"):
            if(strcmp(itr->name(), "author")==0)
                _author = itr->text().get();
            break;
        case hash("lastBuildDate"):
            if(strcmp(itr->name(), "lastBuildDate")==0)
                _last_build_date = itr->text().get();
            break;
        case hash("managingEditor"):
            if(strcmp(itr->name(), "managingEditor")==0)
                _managing_editor = itr->text().get();
            break;
        case hash("itunes:summary"):
            if(strcmp(itr->name(), "itunes:summary")==0)
                _summary = itr->text().get();
            break;
        case hash("item"):
            if(strcmp(itr->name(), "item")==0)
                goto parse_items;
            break;
        default:
            break;
        }
    ++itr;
    }

    // Jump label
    parse_items:
    ;

    if(itr == end)
        throw std::invalid_argument("constructor of podcast class called with inappropriate xml argument.");

    size_t item_number = 0;
    while(itr != end) {
        if(strcmp(itr->name(), "item")==0) {
            try {
                items.emplace_back(episode(item_number, itr));
            }
            catch(std::exception const &e) {
                std::cout << "An exception occurred: " << e.what() << std::endl;
            }
        }

        ++item_number;
        ++itr;
    }

    if(rss_feed_url.empty())
        rss_feed_url = backup_url;

}

std::string const * podcast::find_url(const std::string &title) {
    auto results = std::find_if(items.cbegin(), items.cend(), [&title](const episode &ep){ return ep.has_title(title);});
    if(results == items.end())
        return nullptr;

    return &(results->url());
}

episode * podcast::get_episode(const std::string &title) {
    auto results = std::find_if(items.begin(), items.end(), [&title](const episode &ep){ return ep.has_title(title);});
    if(results == items.end())
        return nullptr;
    return &*results;
}

void podcast::serialize_into(boost::json::value & val) const {
    auto &obj = val.emplace_object();
    boost::hana::for_each(member_field_name_pairs_for_serialization(), [&obj](auto pair) mutable -> void {
        auto const * member = boost::hana::first(pair);
        auto field_name = boost::hana::second(pair);

        serialize(member, obj[field_name]);
    });

}

const std::string & podcast::title() const {
    return _title;
}

std::string podcast::rss_url() {
    return rss_feed_url;
}

std::vector<episode> const & podcast::peek_items() const noexcept {
    return items;
}
