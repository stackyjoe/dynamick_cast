#ifndef PODCAST_HPP
#define PODCAST_HPP


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
    std::string const * find_url(const std::string &title);
    void fill_from_json(boost::property_tree::ptree &parsed_json);
    void fill_from_xml(boost::property_tree::ptree &parsed_xml, std::string backup_url);
    void fill_from_xml(pugi::xml_document &parsed_xml, std::string backup_url);
    episode * get_episode(const std::string &title);
    //void populate(QTableView* view, std::string project_directory);
    //void populate_download_progress(QTableView* view);
    void serialize_into(std::ofstream &file);
    const std::string & title() const;
    std::string rss_url();

    std::vector<episode> const & peek_items() const noexcept;

private:
    std::string _author;
    std::string _last_build_date;
    std::string _managing_editor;
    std::string _summary;
    std::string _title;
    std::string _guid;
    std::string rss_feed_url;
    
    std::vector<episode> items;
};

#endif // PODCAST_HPP
