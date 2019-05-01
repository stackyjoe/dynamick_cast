#include <exception>
#include <stdexcept>

#include "constexpr_string_functions.hpp"
#include "episode.hpp"

episode::episode(boost::property_tree::ptree::iterator tree_node)
    : download_lock { std::make_unique<std::mutex>() }
{
    if(tree_node->first != "item")
        throw std::invalid_argument("Constructor for episode class called on invalid node. Expected node with key \"item\" and instead got \"" + tree_node->first + "\"\n");

    auto itr = tree_node->second.begin();
    auto end = tree_node->second.end();

    while(itr != end) {
        switch(hash(itr->first.data())) {
        case hash("title"):
            if (itr->first == "title")
                title = QString::fromStdString(itr->second.data());
            break;
        case hash("subtitle"):
            if (itr->first == "subtitle")
                subtitle = QString::fromStdString(itr->second.data());
            break;
        case hash("link"):
            if (itr->first == "link")
                page_url = itr->second.data();
            break;
        case hash("description"):
            if (itr->first == "description")
                description = QString::fromStdString(itr->second.data());
            break;
        case hash("guid"):
            if (itr->first == "guid")
                guid = QString::fromStdString(itr->second.data());
            break;
        case hash("enclosure"): {
            if (itr->first != "enclosure")
                break;

            auto child_itr = itr->second.begin();
            auto child_end = itr->second.end();
            while(child_itr != child_end and child_itr->first != "<xmlattr>")
                ++child_itr;

            if(child_itr == child_end)
                throw std::invalid_argument("Can't find the xml attributes.");

            auto attribute_itr = child_itr->second.begin();
            auto attribute_end = child_itr->second.end();
            while(attribute_itr != attribute_end and attribute_itr->first != "url")
                ++attribute_itr;

            if(attribute_itr == attribute_end)
                throw std::invalid_argument("Can't find the url attribute.");

            audio_url = attribute_itr->second.data();
        } break;
        case hash("pubDate"):
            if (itr->first == "pubDate")
                publication_date = QString::fromStdString(itr->second.data());
            break;
        default:
            break;
        }

        ++itr;
    }
}

const std::string &episode::url() const {
    return audio_url;
}

std::optional<std::unique_lock<std::mutex>> episode::get_download_rights() {
    std::unique_lock download_rights(*download_lock, std::try_to_lock);

    if(download_rights.owns_lock())
        return std::move(download_rights);

    return {};
}

bool episode::has_title(const QString &text) const {
    return title == text;
}

void episode::populate(int i, QStandardItemModel *model) const {
    QModelIndex index = model->index(i,0);

    if(not index.isValid())
        return;
    if(not model->setData(index, title, Qt::DisplayRole))
        std::cout << "Failed to add " << title.toStdString() << " to episodeView." << std::endl;
}

void episode::serialize_into(std::ofstream &file) {
    file << "\"item\" : {\n";
    file << "\t\"enclosure\": {\n\t\t\"<xmlattr>\": { \"url\" : \"" << audio_url << "\"}\n\t},\n";
    // TODO: add description, but it needs to be sanitized, since many podcasts embed XML in their description
    // fields.
    file << "\t\"description\": \"" << " " /*description.toStdString()*/ << "\",\n";
    file << "\t\"guid\": \"" << guid.toStdString() << "\",\n";
    file << "\t\"link\": \"" << page_url << "\",\n";
    file << "\t\"pubDate\": \"" << publication_date.toStdString() << "\",\n";
    file << "\t\"subtitle\": \"" << subtitle.toStdString() << "\",\n";
    file << "\t\"title\": \"" << title.toStdString() << "\"\n";
    file << "}";
    // Outputting ",\n" is responsibility of podcast, sadly.
}
