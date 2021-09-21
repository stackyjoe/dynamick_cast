//#include <QStandardItemModel>

#include <fmt/core.h>

#include "shared/string_functions.hpp"
#include "podcast.hpp"

using namespace std::string_literals;

podcast::podcast(std::string url) : rss_feed_url(url) { }

size_t podcast::episode_count() const noexcept {
    return items.size();
}

void podcast::fill_from_xml(boost::property_tree::ptree &parsed_xml) {
    auto itr = parsed_xml.begin();
    auto end = itr->second.end();
    itr = itr->second.begin();

    while(itr != end) {
        switch(hash(itr->first.c_str())) {
        case hash("rss_feed_url"):
            if(itr->first == "rss_feed_url")
                rss_feed_url = itr->second.data();
            break;
        case hash("title"):
            if(itr->first == "title")
                _title = itr->second.data();
            break;
        case hash("author"):
            if(itr->first == "author")
                _author = itr->second.data();
            break;
        case hash("lastBuildDate"):
            if(itr->first == "lastBuildDate")
                _last_build_date = itr->second.data();
            break;
        case hash("managingEditor"):
            if(itr->first == "managingEditor")
                _managing_editor = itr->second.data();
            break;
        case hash("itunes:summary"):
            if(itr->first == "itunes:summary")
                _summary = itr->second.data();
            break;
        case hash("item"):
            if(itr->first == "item")
                goto parse_items;
            break;
        default:
            break;
        }
    ++itr;
    }

    parse_items:

    if(itr == end)
        throw std::invalid_argument("constructor of podcast class called with inappropriate xml argument.");

    std::string podcast_title(itr->second.data());

    size_t item_number = 0;
    while(itr != end) {
        if(itr->first == "item") {
            try {
                items.emplace_back(episode(item_number, itr));
            }
            catch(std::exception &e) {
                std::cout << "An exception has occurred: " << e.what();
            }
        }

        ++item_number;
        ++itr;
    }
}

void podcast::fill_from_xml(pugi::xml_document &parsed_xml) {
    auto itr = parsed_xml.children().begin()->begin();
    auto end = parsed_xml.children().begin()->end();

    while(itr != end) {
        switch(hash(itr->name())) {
        case hash("rss_feed_url"):
            if(strcmp(itr->name(), "rss_feed_url") == 0)
                //itr->name();
                rss_feed_url = itr->text().get();
            break;
        case hash("itunes:new-feed-url"):
            if(strcmp(itr->name(), "itunes:new-feed-url")==0) {
                auto predicate_rel = [](pugi::xml_attribute attr) -> bool{
                    if(strcmp(attr.name(),"rel")==0)
                        return true;
                    return false;
                };

                auto predicate_href = [](pugi::xml_attribute attr) -> bool{
                    if(strcmp(attr.name(),"href")==0)
                        return true;
                    return false;
                };


                auto results_rel = itr->find_attribute(predicate_rel);
                auto results_href = itr->find_attribute(predicate_href);

                if(strcmp(results_rel.value(), "self") == 0) {
                    rss_feed_url = results_href.value();
                }

            }
            break;
        case hash("atom:link"):
            if(strcmp(itr->name(), "atom:link") == 0) {
                auto predicate_rel = [](pugi::xml_attribute attr) -> bool{

                    if(strcmp(attr.name(),"rel")==0)
                        return true;
                    return false;
                };

                auto predicate_href = [](pugi::xml_attribute attr) -> bool{
                    if(strcmp(attr.name(),"href")==0)
                        return true;
                    return false;
                };

                auto results_rel = itr->find_attribute(predicate_rel);
                auto results_href = itr->find_attribute(predicate_href);

                if(strcmp(results_rel.value(), "self") == 0) {
                    rss_feed_url = results_href.value();
                }


            }
            break;
        case hash("title"):
            if(strcmp(itr->name(), "title")==0)
                _title = itr->text().get();
            break;
        case hash("author"):
            if(strcmp(itr->name(), "author")==0)
                _author = itr->text().get();
            break;
        case hash("lastBuildDate"):
            if(strcmp(itr->name(), "lastBuildDate")==0)
                _last_build_date = itr->text().get();
            break;
        case hash("managingEditor"):
            if(strcmp(itr->name(), "managingEditor")==0)
                _managing_editor = itr->text().get();
            break;
        case hash("itunes:summary"):
            if(strcmp(itr->name(), "itunes:summary")==0)
                _summary = itr->text().get();
            break;
        case hash("item"):
            if(strcmp(itr->name(), "item")==0)
                goto parse_items;
            break;
        default:
            break;
        }
    ++itr;
    }

    // Jump label
    parse_items:
    ;

    if(itr == end)
        throw std::invalid_argument("constructor of podcast class called with inappropriate xml argument.");

    size_t item_number = 0;
    while(itr != end) {
        if(strcmp(itr->name(), "item")==0) {
            try {
                items.emplace_back(episode(item_number, itr));
            }
            catch(std::exception const &e) {
                std::cout << "An exception occurred: " << e.what() << std::endl;
            }
        }

        ++item_number;
        ++itr;
    }

}

std::string const * podcast::find_url(const std::string &title) {
    auto results = std::find_if(items.cbegin(), items.cend(), [&title](const episode &ep){ return ep.has_title(title);});
    if(results == items.end())
        return nullptr;

    return &(results->url());
}

episode * podcast::get_episode(const std::string &title) {
    auto results = std::find_if(items.begin(), items.end(), [&title](const episode &ep){ return ep.has_title(title);});
    if(results == items.end())
        return nullptr;
    return &*results;
}
/*
void podcast::populate(QTableView *tableview, std::string directory) {
    auto * model = static_cast<QStandardItemModel *>(tableview->model());
    if(int count { model->rowCount() }; count > 0)
        model->removeRows(0,model->rowCount());

    model->insertRows(0, static_cast<int>(items.size()));

    for(size_t i = 0; i < items.size(); ++i) {
        items[i].populate(static_cast<int>(i), model, directory + _title + "/"s);
    }
}

void podcast::populate_download_progress(QTableView *tableview) {
    // Brittle! Should verify that the size of the tableview is correct.
    auto * model = static_cast<QStandardItemModel *>(tableview->model());

    for(size_t i = 0; i < items.size(); ++i) {
        items[i].populate_download_progress(static_cast<int>(i), model);
    }
}
*/

void podcast::serialize_into(std::ofstream &file) {
    file << "\"" << rss_feed_url << "\" : {\n";
    file << "\"author\": \"" << _author << "\",\n";
    file << "\"lastBuildDate\": \"" << _last_build_date << "\",\n";
    file << "\"managingEditor\": \"" << _managing_editor << "\",\n";
    // TODO(joe): sanitize summary field.
    file << "\"summary\": \"" << " "/*_summary.toStdString()*/ << "\",\n";
    file << "\"title\": \"" << sanitize(_title) << "\",\n";
    file << "\"guid\": \"" << _guid << "\",\n";
    file << "\"rss_feed_url\": \"" << rss_feed_url << "\",\n";

    if(items.empty()) {
        file << "}\n";
        return;
    }

    auto ep_itr = items.begin();
    auto early_end = items.end()-1;

    while(ep_itr != early_end) {
        ep_itr->serialize_into(file);
        file << ",\n";
        ++ep_itr;
    }

    ep_itr->serialize_into(file);
    file << "\n";

    file << "}";
    // printing ",\n" is the responsibility of mainwindow, unfortunately.
}

const std::string & podcast::title() const {
    return _title;
}

std::string podcast::rss_url() {
    return rss_feed_url;
}

std::vector<episode> const & podcast::peek_items() const noexcept {
    return items;
}
