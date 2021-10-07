#include <fmt/core.h>

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

void library::serialize_into(std::ofstream &save_file) {
    save_file << "{\n";
    save_file << "\"dynamick_cast_version\" : " << version << ",\n";
    auto ch_itr = begin();
    auto early_end = std::next(end(),-1);

    while(ch_itr != early_end) {
        auto &[url, channel] = *ch_itr;
        (void)url;// Can't use [[maybe_unused]] attribute with structured bindings.
        save_file << "\"channel\" : {\n";
        channel.serialize_into(save_file);
        save_file << "},\n";
        ++ch_itr;
    }
    auto &[url, channel] = *ch_itr;
    (void)url; // Can't use [[maybe_unused]] attribute with structured bindings.
    save_file << "\"channel\" : {\n";
    channel.serialize_into(save_file);
    save_file << "}\n";

    save_file << "}";
}

void library::fill_from_xml(std::ifstream &save_file) {
    boost::property_tree::ptree tree;
    boost::property_tree::read_json(save_file, tree);

    std::map<std::string, podcast> new_channels;

    auto begin = tree.begin();
    auto end = tree.end();
    auto root = tree.begin();

    using std::string_literals::operator""s;

    if(root->first.data() == "dynamick_cast_version"s) {
        if(std::stoi(root->second.data()) != version)
            throw std::runtime_error("Attempting to load subscriptions.json with different version than what dynamick_cast expects.");
        ++root;
    }

    for(; root != end; ++root) {
        podcast channel(root->first);
        channel.fill_from_xml(root->second, channel.title());
        new_channels.insert_or_assign(channel.title(), std::move(channel));
    }

    channels = std::move(new_channels);

    for(auto &[name, channel] : channels)
        fmt::print("Loaded channel: {} with {}\n", name, channel.episode_count());
}
