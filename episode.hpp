#ifndef EPISODE_HPP
#define EPISODE_HPP

#include <QModelIndex>
#include <QStandardItemModel>
#include <QString>

#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <fstream>
#include <mutex>

class episode
{
public:
    episode() = delete;
    explicit episode(boost::property_tree::ptree::iterator tree_node);

    const std::string &url() const;
    QString id() const;
    std::optional<std::unique_lock<std::mutex>> get_download_rights();
    bool has_title(const QString &text) const;
    void populate(int i, QStandardItemModel *model) const;
    void serialize_into(std::ofstream &file);

protected:
    std::string audio_url;
    QString description;
    QString guid;
    std::string page_url;
    QString publication_date;
    QString subtitle;
    QString title;
    mutable std::unique_ptr<std::mutex> download_lock;
};

#endif // EPISODE_HPP
