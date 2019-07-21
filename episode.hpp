#ifndef EPISODE_HPP
#define EPISODE_HPP

#include <QModelIndex>
#include <QStandardItemModel>
#include <QString>

#include <iostream>
#include <fstream>
#include <mutex>

#include <boost/property_tree/ptree.hpp>
#include <pugixml.hpp>

#include "string_functions.hpp"

class episode
{
public:
    episode();
    explicit episode(pugi::xml_node::iterator itr);
    void fill(pugi::xml_node::iterator itr);

    // The following two are left in but as is are dead code. ptree seems finicky and randomly
    // fails to parse xml that the pugi library succeeds with, however if you really want one less
    // dependency you can modify the calls in MainWindow::fetch_rss() to use them again.
    explicit episode(boost::property_tree::ptree::iterator tree_node);
    void fill(boost::property_tree::ptree::iterator tree_node);

    const std::string &url() const;
    QString id() const;
    std::shared_ptr<std::tuple<std::mutex,std::optional<std::unique_lock<std::mutex>>>> get_download_rights();
    bool has_title(const QString &text) const;
    void populate(int i, QStandardItemModel *model, std::string directory) const;
    void serialize_into(std::ofstream &file);

protected:
    std::string audio_url;
    QString description;
    QString guid;
    std::string page_url;
    QString publication_date;
    QString subtitle;
    QString title;
    mutable std::shared_ptr<std::tuple<std::mutex,std::optional<std::unique_lock<std::mutex>>>> download_lock;
};

#endif // EPISODE_HPP
