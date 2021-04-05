#include <exception>
#include <stdexcept>

#include <QPixmap>

#include "episode.hpp"
#include "shared/string_functions.hpp"

episode::episode(size_t item_number, boost::property_tree::ptree::iterator tree_node)
    : item_number_in_xml(item_number), shared_state { std::make_shared<download_shared_state>() }
{
    fill(tree_node);
}

episode::episode(size_t item_number, pugi::xml_node::iterator iterator)
    : item_number_in_xml(item_number), shared_state { std::make_shared<download_shared_state>() }
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
                title = itr->second.data();
            break;
        case hash("subtitle"):
            if (itr->first == "subtitle")
                subtitle = itr->second.data();
            break;
        case hash("link"):
            if (itr->first == "link")
                page_url = itr->second.data();
            break;
        case hash("description"):
            if (itr->first == "description")
                description = itr->second.data();
            break;
        case hash("guid"):
            if (itr->first == "guid")
                guid = itr->second.data();
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
                publication_date = itr->second.data();
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
                title = itr->text().get();
            break;
        case hash("subtitle"):
            if (strcmp(itr->name(), "subtitle")==0)
                subtitle = itr->text().get();
            break;
        case hash("link"):
            if (strcmp(itr->name(), "link")==0)
                page_url = itr->text().get();
            break;
        case hash("description"):
            if (strcmp(itr->name(), "description")==0)
                description = itr->text().get();
            break;
        case hash("guid"):
            if (strcmp(itr->name(), "guid")==0)
                guid = itr->text().get();
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
                publication_date = itr->text().get();
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

std::shared_ptr<download_shared_state> episode::get_download_rights() {
    return shared_state;
}

std::string episode::get_title() const {
    return title;
}

bool episode::has_title(const std::string &text) const {
    return title == text;
}

void episode::populate(int row, QStandardItemModel *model, std::string directory) const {
    QModelIndex index = model->index(row,0);

    if(index.isValid()) {
        auto maybe_lock = shared_state->try_lock();
        if(maybe_lock.has_value()) {
            auto file_name = get_sanitized_file_name();
            std::fstream file{directory+file_name};
            if(file.good()) {
                auto item = model->itemFromIndex(index);
                if(item != nullptr){
                    item->setData(QVariant(), Qt::UserRole);
                    item->setData(QVariant(), Qt::DecorationRole);
                    item->setData(QVariant(), Qt::DisplayRole);
                }
                model->setData(index, QIcon(":/package.svg").pixmap(QSize(16,16)), Qt::DecorationRole);
            }
            else {

                model->setData(index, QIcon(":/hexagon.svg").pixmap(QSize(16,16)), Qt::DecorationRole);
            }
        }
        else {
            // Someone must be downloading


            double percent = 100.0*shared_state->get_bytes_completed() / std::max(1.0,static_cast<double>(shared_state->get_bytes_total()));
            model->setData(index, QVariant{}, Qt::DecorationRole);
            model->setData(index,
                           shared_state->get_bytes_total() > 0 ?
                               QString::number(percent, 'f', 1) + QString::fromLocal8Bit("%",1) :
                               QString::number(shared_state->get_bytes_completed()/1000000) + QString::fromLocal8Bit("MB",2),
                           Qt::DisplayRole);
        }
    }

    index = model->index(row,1);

    if(index.isValid() and not model->setData(index, QString::number(item_number_in_xml), Qt::DisplayRole) )
        std::cout << "Failed to add item number." << std::endl;

    index = model->index(row,2);

    if(index.isValid() and not model->setData(index, QString::fromStdString(title), Qt::DisplayRole))
            std::cout << "Failed to add " << title << " to episodeView." << std::endl;
}

void episode::populate_download_progress(int row, QStandardItemModel *model) const {
    QModelIndex index = model->index(row,0);

    if(index.isValid()) {
        auto maybe_lock = shared_state->try_lock();
        if(not maybe_lock.has_value()) {

            double percent = 100.0*shared_state->get_bytes_completed() / std::max(1.0,static_cast<double>(shared_state->get_bytes_total()));

            model->setData(index,
                           shared_state->get_bytes_total() > 0 ?
                               QString::number(percent, 'f', 1) + QString::fromLocal8Bit("%",1) :
                               QString::number(shared_state->get_bytes_completed()/1000000) + QString::fromLocal8Bit("MB",2),
                           Qt::DisplayRole);
        }
    }
}

void episode::serialize_into(std::ofstream &file) {
    file << "\"item\" : {\n";
    file << "\t\"enclosure\": {\n\t\t\"<xmlattr>\": { \"url\" : \"" << audio_url << "\"}\n\t},\n";
    // TODO: add description, but it needs to be sanitized, since many podcasts embed XML in their description
    // fields.
    file << "\t\"description\": \"" << " " /*description.toStdString()*/ << "\",\n";
    file << "\t\"guid\": \"" << guid << "\",\n";
    file << "\t\"link\": \"" << page_url << "\",\n";
    file << "\t\"pubDate\": \"" << publication_date << "\",\n";
    file << "\t\"subtitle\": \"" << subtitle << "\",\n";
    file << "\t\"title\": \"" << sanitize(title) << "\"\n";
    file << "}";
    // Outputting ",\n" is responsibility of podcast, sadly.
}

std::string episode::get_sanitized_file_name() const noexcept {
    return std::to_string(hash(guid.c_str()))+".mp3";
}
