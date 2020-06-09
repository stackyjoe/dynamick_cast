#ifndef PODCAST_HPP
#define PODCAST_HPP

#include <QTableView>

#include <string>
#include <vector>

#include "episode.hpp"

class podcast
{
public:
    podcast() = delete;
    ~podcast() = default;

    podcast(const podcast &) = delete;
    podcast &operator=(const podcast &) = delete;

    podcast(podcast &&) = default;
    podcast &operator=(podcast &&) = default;

    explicit podcast(std::string url);

    size_t episode_count() const noexcept;
    std::string const * find_url(const QString &title);
    void fill_from_json(boost::property_tree::ptree &parsed_json);
    void fill_from_xml(boost::property_tree::ptree &parsed_xml);
    void fill_from_xml(pugi::xml_document &parsed_xml);
    episode * get_episode(const QString &title);
    void populate(QTableView* view, std::string project_directory);
    void populate_download_progress(QTableView* view);
    void serialize_into(std::ofstream &file);
    const std::string & title();
    std::string rss_url();

protected:
    QString _author;
    QString _last_build_date;
    QString _managing_editor;
    QString _summary;
    std::string _title;
    QString _guid;
    std::string rss_feed_url;
    
    std::vector<episode> items;
    std::vector<std::string> item_errors;
};

#endif // PODCAST_HPP
