#include <fmt/core.h>

#include <boost/json.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <pugixml.hpp>

#include "library.hpp"

using iterator = library::iterator;
using const_iterator = library::const_iterator;


#ifdef __unix__
extern "C" {
    #include <unistd.h>
}
#endif

#ifdef _WIN64
extern "C" {
    #include <stdlib.h>
}
#endif

using std::string_literals::operator""s;

std::string get_local_file_path() {
#ifdef __unix__
    constexpr size_t buf_size = 64;
    
    auto username_buf = std::make_unique<char[]>(64);
    
    for(auto i = 0; i < buf_size; ++i) {
        username_buf[i] = '\0';
    }

    getlogin_r(username_buf.get(),buf_size);
    return "/home/"s + std::string(username_buf.get()) + "/.local/share"s;
#endif

#ifdef _WIN64
    
    char* p = nullptr;
    size_t buf_size = 64;

    _dupenv_s(&p, &buf_size, "APPDATA");

    // The string seems to have a trailing space
    std::string contents = { p, p+buf_size-1 };
    free(p);
    return contents;
#endif
    throw std::runtime_error("Compilation environment should have defined either __unix__ or _WIN64 macros. Could not correctly initialize library component.");
}

std::string get_native_separator() {
    #ifdef _WIN64
    return "\\"s;
    #endif
    return "/"s;
}

library::library() 
    : application_files_path(get_local_file_path()),
    native_separator(get_native_separator()),
    project_directory(application_files_path+native_separator+"dynamick_cast"s + native_separator)
{

}


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

std::string library::native_sep() const {
    return native_separator;
}

std::string library::app_file_path() const {
    return project_directory;
}
