#include <fmt/core.h>

#include <boost/json.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <pugixml.hpp>

#include "library.hpp"

using iterator = library::iterator;
using const_iterator = library::const_iterator;

iterator library::begin() noexcept {
    return channels.begin();
}
    
const_iterator library::begin() const noexcept {
    return channels.begin();
}
    
iterator library::end() noexcept {
    return channels.end();
}
    
const_iterator library::end() const noexcept {
    return channels.end();
}

size_t library::size() const noexcept {
    return channels.size();
}

bool library::empty() const noexcept {
    return channels.empty();
}

podcast * library::find(std::string const &name) noexcept {
    auto itr = channels.find(name);
    if(itr == channels.end())
        return nullptr;
    return &(itr->second);
}

podcast const * library::find(std::string const &name) const noexcept {
    auto itr = channels.find(name);
    if(itr == channels.end())
        return nullptr;
    return &(itr->second);
}

void library::erase(std::string const &name) {
    channels.erase(name);
}

std::pair<iterator, bool> library::insert_or_assign(std::string const &key, podcast && value) {
    return channels.insert_or_assign(key,std::move(value));
}

boost::json::value library::to_json() const noexcept {
    boost::json::value result;

    auto & root = result.emplace_object();

    root["dynamick_cast_version"].emplace_int64() = version;

    auto & podcasts = root["channels"].emplace_object();

    for(auto const & [name, pod ] : channels) {
        auto & this_pod = podcasts[name];
        pod.serialize_into(this_pod);
    }

    return result;
}

void library::serialize_into(std::ofstream &save_file) {

    boost::json::value v = to_json();

    std::string serialized = boost::json::serialize(v);

    save_file << serialized;
}

void library::fill_from_json(std::ifstream &save_file) {

    using std::string_literals::operator""s;

    
    save_file.seekg(0, std::ios::end);
    auto file_size = save_file.tellg();
    save_file.seekg(0, std::ios::beg);

    std::vector<char> file_contents(file_size);

    save_file.read(& (file_contents[0]), file_size);


    boost::json::parse_options opt;

    opt.allow_comments = true;
    opt.allow_trailing_commas = true;
    opt.allow_invalid_utf8 = true;


#ifdef NDEBUG
    boost::json::error_code ec;


    auto parsed_subscriptions_file = boost::json::parse(boost::json::string_view(file_contents.data(), file_size), ec, {}, opt);

    if(ec) 
        throw std::runtime_error("Unable to parse JSON file subscriptions.json");
#else
    auto parsed_subscriptions_file = boost::json::parse(boost::json::string_view(file_contents.data(), file_size), {}, opt);
#endif

    std::map<std::string, podcast> new_channels;

    auto & parsed_object = parsed_subscriptions_file.as_object();
    
    if(version != parsed_object["dynamick_cast_version"].as_int64())
        throw std::runtime_error("JSON file written with different version of dynamick_cast.");

    auto & pods = parsed_object["channels"].as_object();

    for(auto cur_kv_itr : pods) {
        new_channels.insert_or_assign(std::string(cur_kv_itr.key()),
            podcast::deserialize_from(cur_kv_itr.value()));
    }

    std::swap(channels, new_channels);
}
