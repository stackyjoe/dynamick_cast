#ifndef SERIALIZATION_HPP
#define SERIALIZATION_HPP

#include <string>

#include <boost/json.hpp>
#include <boost/hana.hpp>

inline boost::json::string_view operator"" _bsv(char const * c_string, size_t len) {
    return boost::json::string_view(c_string, len);
}

inline void serialize(std::string const * contents, boost::json::value &target) {
    target.emplace_string() = *contents;
}

inline void deserialize(std::string * target, boost::json::value const &contents) {
    *target = contents.as_string();
}

inline void serialize(int64_t const * contents, boost::json::value &target) {
    target.emplace_int64() = *contents;
}

inline void deserialize(int64_t * target, boost::json::value const &contents) {
    *target = contents.as_int64();
}

template<class T>
inline void serialize(std::vector<T> const * contents, boost::json::value &target) {
    int64_t len = contents->size();
    auto & as_array = target.emplace_array();
    as_array.resize(len);

    for(int64_t i = 0; i < len; ++i) {
        (*contents)[i].serialize_into(as_array[i]);
    }
}

template<class T>
inline void deserialize(std::vector<T> * target, boost::json::value const &contents) {
    auto const & as_array = contents.as_array();
    size_t len = as_array.size();

    target->clear();
    target->reserve(len);

    for(auto i = 0; i < len; ++i) {
        target->push_back(T::deserialize_from(as_array[i]));
    }
}

template<class V>
inline void serialize(std::map<std::string,V> const * target, boost::json::value & contents) {
    auto const & as_obj = contents.emplace_object();
    for(auto const_itr : *target) {
        auto & leaf = as_obj[const_itr.first];
        const_itr.second.serialize_into(leaf);
    }
}

template<class V>
inline void deserialize(std::map<std::string,V> * target, boost::json::value const & contents) {
    auto const & as_obj = contents.as_object();
    for(auto kv_pair : as_obj) {
        target->insert(std::string(kv_pair.key().data(), kv_pair.key().size()),
            V::deserialize_from(kv_pair.value()));
    }
}


#endif // SERIALIZATION_HPP
