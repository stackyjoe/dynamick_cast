#ifndef PUGIXML_ADAPTER_HPP
#define PUGIXML_ADAPTER_HPP

#include <pugixml.hpp>

#include "string_functions.hpp"
#include "xml_node_adapter.hpp"

template<>
inline bool xml_node_adapter<pugi::xml_node, pugi::xml_node::iterator>::has_name(char const *name) {
    return strncmp(raw_node.name(), name, length(name)) == 0;
}

template<>
inline char const * xml_node_adapter<pugi::xml_node, pugi::xml_node::iterator>::name() {
    return raw_node.name();
}

template<>
inline char const * xml_node_adapter<pugi::xml_node, pugi::xml_node::iterator>::pcdata() {
    return raw_node.child_value();
}

template<>
inline xml_node_adapter<pugi::xml_node, pugi::xml_node::iterator>::iterator xml_node_adapter<pugi::xml_node, pugi::xml_node::iterator>::begin() {
    return raw_node.children().begin();
}

template<>
inline xml_node_adapter<pugi::xml_node, pugi::xml_node::iterator>::iterator xml_node_adapter<pugi::xml_node, pugi::xml_node::iterator>::end() {
    return raw_node.end();
}

#endif
