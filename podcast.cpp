#include <QStandardItemModel>

#include "string_functions.hpp"
#include "podcast.hpp"

using std::string_literals::operator""s;

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
                _author = QString::fromStdString(itr->second.data());
            break;
        case hash("lastBuildDate"):
            if(itr->first == "lastBuildDate")
                _last_build_date = QString::fromStdString(itr->second.data());
            break;
        case hash("managingEditor"):
            if(itr->first == "managingEditor")
                _managing_editor = QString::fromStdString(itr->second.data());
            break;
        case hash("itunes:summary"):
            if(itr->first == "itunes:summary")
                _summary = QString::fromStdString(itr->second.data());
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

    size_t loops = 0;
    while(itr != end) {
        if(itr->first == "item") {
            try {
                items.emplace_back(episode(itr));
            }
            catch(std::exception &e) {
                item_errors.emplace_back(e.what());
            }
        }

        ++loops;
        ++itr;
    }
}

std::string const * podcast::find_url(const QString &title) {
    auto results = std::find_if(items.cbegin(), items.cend(), [&title](const episode &ep){ return ep.has_title(title);});
    if(results == items.end())
        return nullptr;

    return &(results->url());
}

episode * podcast::get_episode(const QString &title) {
    auto results = std::find_if(items.begin(), items.end(), [&title](const episode &ep){ return ep.has_title(title);});
    if(results == items.end())
        return nullptr;
    return &*results;
}

void podcast::populate(QTableView *tableview) {
    auto * model = static_cast<QStandardItemModel *>(tableview->model());
    if(int count { model->rowCount() }; count > 0)
        model->removeRows(0,model->rowCount());

    model->insertRows(0, static_cast<int>(items.size()));

    for(size_t i = 0; i < items.size(); ++i) {
        items[i].populate(static_cast<int>(i), model);
    }
}

void podcast::serialize_into(std::ofstream &file) {
    file << "\"" << rss_feed_url << "\" : {\n";
    file << "\"author\": \"" << _author.toStdString() << "\",\n";
    file << "\"lastBuildDate\": \"" << _last_build_date.toStdString() << "\",\n";
    file << "\"managingEditor\": \"" << _managing_editor.toStdString() << "\",\n";
    // TODO: sanitize summary field.
    file << "\"summary\": \"" << " "/*_summary.toStdString()*/ << "\",\n";
    file << "\"title\": \"" << sanitize(_title) << "\",\n";
    file << "\"guid\": \"" << _guid.toStdString() << "\",\n";
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

const std::string & podcast::title() {
    return _title;
}

std::string podcast::rss_url() {
    return rss_feed_url;
}
