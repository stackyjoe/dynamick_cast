#pragma once
#ifndef PTREE_ADAPTER_HPP
#define PTREE_ADAPTER_HPP

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "xml_node_adapter.hpp"

template<>
inline xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator> xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator>::iterator::operator*() {
    return xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator>{(*raw_iterator).second};
}

template<>
inline bool xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator>::has_name(char const *name) {
    return raw_node.begin()->first.c_str() == name;
}

template<>
inline char const * xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator>::name() {
    return raw_node.data().c_str();
}

template<>
inline char const * xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator>::pcdata() {
    return raw_node.begin()->second.data().c_str();
}

template<>
inline xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator>::iterator xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator>::begin() {
    return xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator>::iterator{raw_node.begin()};
}

template<>
inline xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator>::iterator xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator>::end() {
    return xml_node_adapter<boost::property_tree::ptree, boost::property_tree::ptree::iterator>::iterator{raw_node.end()};
}

#endif // PTREE_ADAPTER_HPP
