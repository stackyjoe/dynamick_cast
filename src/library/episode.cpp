#include <exception>
#include <stdexcept>

#include "episode.hpp"
#include "shared/string_functions.hpp"


namespace hana = boost::hana;

episode::episode(size_t item_number, pugi::xml_node::iterator iterator)
    : item_number_in_xml(item_number), shared_state { std::make_shared<download_shared_state>() }
{
    fill(iterator);
}

episode::episode(boost::json::object const &obj)
    : shared_state(std::make_shared<download_shared_state>()){

    boost::hana::for_each(member_field_name_pairs_for_serialization(), [obj](auto pair){
        auto * member = hana::first(pair);
        auto field_name = hana::second(pair);

        auto itr = obj.find(field_name);

        if(itr == obj.end())
            throw std::runtime_error("Field not found");

        deserialize(member, itr->value());
    });
}

void episode::serialize_into(boost::json::value &val) const {
    auto & obj = val.emplace_object();
    boost::hana::for_each(member_field_name_pairs_for_serialization(),[&obj](auto pair) mutable -> void {
        auto * member = hana::first(pair);
        auto field_name = hana::second(pair);

        serialize(member, obj[field_name]);
    });
}

episode episode::deserialize_from(boost::json::value const &val) {
    auto const & obj = val.as_object();
    return episode(obj);
}



void episode::fill(pugi::xml_node::iterator iterator) {
    auto itr = iterator->children().begin();
    auto end = iterator->children().end();

    while(itr != end) {
        switch(hash(itr->name())) {
        case hash("title"):
            if (strcmp(itr->name(), "title")==0)
                title = itr->text().get();
            break;
        case hash("subtitle"):
            if (strcmp(itr->name(), "subtitle")==0)
                subtitle = itr->text().get();
            break;
        case hash("link"):
            if (strcmp(itr->name(), "link")==0)
                page_url = itr->text().get();
            break;
        case hash("description"):
            if (strcmp(itr->name(), "description")==0)
                description = itr->text().get();
            break;
        case hash("guid"):
            if (strcmp(itr->name(), "guid")==0)
                guid = itr->text().get();
            break;
        case hash("enclosure"): {
            if (strcmp(itr->name(), "enclosure") != 0)
                break;

            auto predicate = [](pugi::xml_attribute attr) -> bool{
                if(strcmp(attr.name(),"url")==0)
                    return true;
                return false;
            };
            auto results = itr->find_attribute(predicate);
            audio_url = results.value();

            if(audio_url.empty())
                throw std::invalid_argument("Can't find the xml attributes.");

        } break;
        case hash("pubDate"):
            if (strcmp(itr->name(), "pubDate")==0)
                publication_date = itr->text().get();
            break;
        default:
            break;
        }

        ++itr;
    }
}

const std::string &episode::url() const {
    return audio_url;
}

std::shared_ptr<download_shared_state> episode::get_download_rights() const {
    return shared_state;
}

std::string episode::get_title() const {
    return title;
}

bool episode::has_title(const std::string &text) const {
    return title == text;
}

std::string episode::get_sanitized_file_name() const noexcept {
    return std::to_string(hash(guid.c_str()))+".mp3";
}
