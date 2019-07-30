#include <exception>
#include <stdexcept>

#include <QPixmap>

#include "episode.hpp"
#include "string_functions.hpp"

episode::episode()
    : download_lock (std::make_shared<std::tuple<std::mutex, std::optional<std::unique_lock<std::mutex> > >>()) {

}

episode::episode(boost::property_tree::ptree::iterator tree_node)
    : download_lock { std::make_shared<std::tuple<std::mutex, std::optional<std::unique_lock<std::mutex> > >>() }
{
    fill(tree_node);
}

episode::episode(pugi::xml_node::iterator iterator)
    : download_lock { std::make_shared<std::tuple<std::mutex, std::optional<std::unique_lock<std::mutex> > >>() }
{
    fill(iterator);
}

void episode::fill(boost::property_tree::ptree::iterator tree_node) {
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

void episode::fill(pugi::xml_node::iterator iterator) {
    auto itr = iterator->children().begin();
    auto end = iterator->children().end();

    while(itr != end) {
        switch(hash(itr->name())) {
        case hash("title"):
            if (strcmp(itr->name(), "title")==0)
                title = QString::fromStdString(itr->text().get());
            break;
        case hash("subtitle"):
            if (strcmp(itr->name(), "subtitle")==0)
                subtitle = QString::fromStdString(itr->text().get());
            break;
        case hash("link"):
            if (strcmp(itr->name(), "link")==0)
                page_url = itr->text().get();
            break;
        case hash("description"):
            if (strcmp(itr->name(), "description")==0)
                description = QString::fromStdString(itr->text().get());
            break;
        case hash("guid"):
            if (strcmp(itr->name(), "guid")==0)
                guid = QString::fromStdString(itr->text().get());
            break;
        case hash("enclosure"): {
            if (strcmp(itr->name(), "enclosure") != 0)
                break;

            auto predicate = [](pugi::xml_attribute attr) -> bool{
                if(strcmp(attr.name(),"url")==0)
                    return true;
                return false;
            };
            auto results = itr->find_attribute(predicate);
            audio_url = results.value();

            if(audio_url.empty())
                throw std::invalid_argument("Can't find the xml attributes.");

        } break;
        case hash("pubDate"):
            if (strcmp(itr->name(), "pubDate")==0)
                publication_date = QString::fromStdString(itr->text().get());
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

std::shared_ptr<std::tuple<std::mutex,std::optional<std::unique_lock<std::mutex>>>> episode::get_download_rights() {
    return download_lock;
}

bool episode::has_title(const QString &text) const {
    return title == text;
}

void episode::populate(int i, QStandardItemModel *model, std::string directory) const {
    if( QModelIndex index = model->index(i,0); index.isValid()) {
        if(std::unique_lock lock(std::get<0>(*download_lock), std::try_to_lock); lock.owns_lock()) {
            auto pos = audio_url.find_last_of('/')+1;
            auto rpos = audio_url.find_first_of('?');
            std::string file_name { audio_url.substr(pos, rpos-pos) };
            if(std::fstream file{directory+file_name}; file.good()) {

                //model->clearItemData(index);
                auto item = model->itemFromIndex(index);
                if(item != nullptr){
                    item->setData(QVariant(), Qt::UserRole);
                    item->setData(QVariant(), Qt::DecorationRole);
                    item->setData(QVariant(), Qt::DisplayRole);
                }
                model->setData(index, QIcon(":/icons/icons/check-square.svg").pixmap(QSize(16,16)), Qt::DecorationRole);
            }
        }
        else {
            // Someone must be downloading
            //TODO(joe): implement UI updates based on download progress

            //QString::fromUtf8("\xf0\x9f\x95\x97")
            std::cout << "Still downloading " << title.toStdString() << "\n";
            model->setData(index, QIcon(":/icons/icons/loader.svg").pixmap(QSize(16,16)), Qt::DecorationRole);
        }
    }

    if( QModelIndex index = model->index(i,1); index.isValid() and not model->setData(index, title, Qt::DisplayRole))
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
    file << "\t\"title\": \"" << sanitize(title.toStdString()) << "\"\n";
    file << "}";
    // Outputting ",\n" is responsibility of podcast, sadly.
}
