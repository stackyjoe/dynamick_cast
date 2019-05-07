#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QStringListModel>
#include <QUrl>

#include <boost/iostreams/stream.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <experimental/filesystem>

#include "episode.hpp"
#include "mainwindow.hpp"
#include "rss_getter.hpp"
#include "url_parser.hpp"

using std::string_literals::operator""s;

MainWindow::MainWindow(audio_interface &audio_handle, QWidget *parent) :
    QMainWindow(parent),
    ui(std::make_unique<Ui::MainWindow>()),
    _audio_handle(audio_handle),
    state(UserDesiredState::stop),
    home_path(QDir::homePath().toStdString()),
    native_separator(""s+QDir::separator().toLatin1()),
    project_directory(home_path + "/.local/share/applications/dynamick-cast/"s),
    daemon(
        [this](){
            while(1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                this->sync_audio_with_library_state();

                this->sync_ui_with_audio_state();
            }
        }),
    volume(100)
{
    ui->setupUi(this);
    showMaximized();

    auto * podcast_item_model = new QStandardItemModel;
    QStringList headers { QString {"Podcast title"} };
    podcast_item_model->setHorizontalHeaderLabels(std::move(headers));
    ui->podcastView->setModel(podcast_item_model);

    auto * episode_item_model = new QStandardItemModel;
    QStringList ep_headers {QStringList { QString {"Episode title"} } };
    episode_item_model->setHorizontalHeaderLabels(ep_headers);
    ui->episodeView->setModel(episode_item_model);

    set_up_connections();
}

void MainWindow::set_up_connections() {
    connect(ui->podcastView, &QTableView::doubleClicked, this, &MainWindow::set_active_channel);
    connect(ui->podcastView, &QWidget::customContextMenuRequested, this, &MainWindow::podcastViewContextMenu);
    connect(ui->episodeView, &QTableView::doubleClicked, this, &MainWindow::download_or_play);
    connect(ui->episodeView, &QTableView::customContextMenuRequested, this, &MainWindow::episodeViewContextMenu);
    connect(ui->actionSave_subscriptions, &QAction::triggered, this, &MainWindow::save_subscriptions);
    connect(ui->actionLoad_subscriptions, &QAction::triggered, this, &MainWindow::load_subscriptions);
    connect(ui->actionAdd_a_podcast, &QAction::triggered, this, &MainWindow::add_rss_from_dialog);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::quit);
    connect(ui->volume_slider, &QAbstractSlider::valueChanged, this, &MainWindow::change_volume);
    connect(ui->seek_slider, &QAbstractSlider::sliderReleased, this, [&](){this->seek();});
    connect(ui->seek_slider, &QAbstractSlider::sliderPressed, this, [&](){seek_bar_lock.lock();});
    connect(ui->play_button, &QAbstractButton::clicked, this, &MainWindow::on_play_button_clicked);
}

void MainWindow::add_rss_from_dialog() {
    bool success = false;
    std::string url = QInputDialog::getText(this, tr("Fetch feed"),
                                             tr("RSS feed URL:"), QLineEdit::Normal,
                                             tr(""), &success).toStdString();
    if(not success)
        return;

    fetch_rss(url);
}

void MainWindow::change_volume(int vol) {
    volume = vol;
    _audio_handle.perform([vol](audio_wrapper &interface){interface.set_volume(vol);});
}

void MainWindow::closeEvent([[maybe_unused]] QCloseEvent *ev) {
    quit();
}

bool MainWindow::download(const QModelIndex &index) {
    auto itr = channels.find(open_channel);
    if(itr == channels.end())
        return false;

    QString episode_title = index.data(Qt::DisplayRole).toString();
    podcast & cur_pod = itr->second;
    std::string const * url = cur_pod.find_url(episode_title);

    if(url == nullptr)
        return false;

    auto pos = url->find_last_of('/')+1;
    auto rpos = url->find_first_of('?');
    std::string file_name = url->substr(pos, rpos-pos);

    if(not QDir::root().mkpath(QString::fromStdString(project_directory + native_separator + open_channel)))
        throw std::experimental::filesystem::filesystem_error("Could not create directory "s,
                                                              project_directory + native_separator + open_channel,
                                                              std::error_code());

    std::string local_path = project_directory + open_channel + native_separator + file_name;

    return download(cur_pod, episode_title, *url, local_path);
}

bool MainWindow::download(podcast &cur_pod,
                          QString episode_title,
                          std::string url,
                          std::string file_dest) {

    try {
        episode * ep = cur_pod.get_episode(episode_title);

        if(ep == nullptr)
            throw std::invalid_argument("Can't find given episode title in the current podcast.");

        std::optional<std::unique_lock<std::mutex>> download_rights = ep->get_download_rights();

        if(not download_rights.has_value())
            return false;

        std::thread thd {
                    [url, file_dest, this, download_rights = std::move(download_rights)]() mutable {
                        auto [protocol, hostname, target, query] = parse_url(url);
                        this->getter.download_file(hostname,
                                                   target,
                                                   80,
                                                   file_dest,
                                                   std::move(*download_rights));}
        };

        thd.detach();
    }
    catch(const std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    return true;
}

void MainWindow::download_or_play(const QModelIndex &index) {
    auto itr = channels.find(open_channel);
    if(itr == channels.end())
        return;

    QString episode_title = index.data(Qt::DisplayRole).toString();
    podcast & cur_pod = itr->second;
    std::string const * url = cur_pod.find_url(episode_title);

    if(url == nullptr)
        return;

    auto pos = url->find_last_of('/')+1;
    auto rpos = url->find_first_of('?');
    std::string file_name { url->substr(pos, rpos-pos) };
    std::cout << file_name << std::endl;

    if(not QDir::root().mkpath(QString::fromStdString(project_directory + native_separator + open_channel)))
        throw std::experimental::filesystem::filesystem_error("Could not create directory "s,
                                                              project_directory + native_separator + open_channel,
                                                              std::error_code());

    std::string local_path = project_directory + open_channel + native_separator + file_name;

    QFileInfo file(QString::fromStdString(local_path));

    if(file.exists() and file.isFile()) {
        int vol = volume;
        if( _audio_handle.perform(
            [local_path, vol](audio_wrapper &handle) -> bool {
                if(not handle.open_from_file(local_path))
                    return false;
                handle.play();
                handle.set_volume(vol);
                return true;
            }
        )) {
            state = UserDesiredState::play;
            ui->current_track_label->setText(episode_title);
        }
        else {
            std::cout << "Error opening file: " << local_path << std::endl;
        }
    }
    else {
        download(cur_pod, episode_title, *url, local_path);
    }
}

void MainWindow::fetch_rss(std::string url) {
    boost::property_tree::ptree tree;


    // rss_feed is a std::string owning a possibly very large string (~950000 bytes in one example I'm testing on)
    // xml_segment is a std::string_view to the xml segment of it. The alternative is making a deep copy with
    // std::string::substr(), or using std::string::erase() which still has to move most of the data.
    try {
        auto [clean_url, rss_feed, xml_segment] = getter.get_feed(url, 80);

        std::cout << clean_url << std::endl;
        url = clean_url;

        boost::iostreams::stream<boost::iostreams::array_source> stream(xml_segment.begin(),xml_segment.size());
        boost::property_tree::read_xml(stream, tree);
    }
    catch(const std::exception &e) {
        std::cout << e.what() << std::endl;
        return;
    }

    podcast new_channel(url);
    new_channel.fill_from_xml(tree);

    std::string title = new_channel.title();
    auto [itr, was_inserted] = channels.insert_or_assign(title, std::move(new_channel));

    if(not was_inserted) {
        // TODO: update ui->episodeView
        std::cout << "Assigned " << title << std::endl;
        return;
    }

    int cur_count = ui->podcastView->model()->rowCount();
    ui->podcastView->model()->insertRow(cur_count);
    QModelIndex index = ui->podcastView->model()->index(cur_count,0);
    ui->podcastView->model()->setData(index, QString::fromStdString(title), Qt::DisplayRole);

}

void MainWindow::load_subscriptions() {
    std::cout << "Loading subscriptions." << std::endl;
    std::string file_path = project_directory + "subscriptions.json"s;
    std::ifstream save_file(file_path, std::ios::in);

    if(not save_file.is_open()) {
        std::cout << "Error opening file: " << file_path << std::endl;
        return;
    }

    try {
        boost::property_tree::ptree tree;
        boost::property_tree::read_json(save_file, tree);

        std::map<std::string, podcast> new_channels;

        auto end = tree.end();
        for(auto root = tree.begin(); root != end; ++root) {
            podcast channel(root->first);
            channel.fill_from_xml(root->second);
            new_channels.insert_or_assign(channel.title(), std::move(channel));
        }

        channels = std::move(new_channels);

        for(auto &[name, channel] : channels)
            std::cout << "Loaded channel: " << name << " with " << channel.episode_count() << " episodes." << std::endl;


        sync_ui_with_library_state();
    }
    catch(const std::exception &e) {
        std::cout << "An exception occurred: " << e.what() << std::endl;
    }    
}


[[noreturn]] void MainWindow::quit() {
    std::quick_exit(EXIT_SUCCESS);
}

void MainWindow::remove_local_file(const QModelIndex &index) {
    auto itr = channels.find(open_channel);
    if(itr == channels.end())
        return;

    QString episode_title = index.data(Qt::DisplayRole).toString();
    podcast & cur_pod = itr->second;
    std::string const * url = cur_pod.find_url(episode_title);

    if(url == nullptr)
        return;

    auto pos = url->find_last_of('/')+1;
    auto rpos = url->find_first_of('?');
    std::string file_name = url->substr(pos, rpos-pos);
    std::experimental::filesystem::path file(project_directory + open_channel + native_separator + file_name);
    std::error_code ec;
    std::experimental::filesystem::remove(file,ec);
}

void MainWindow::remove_local_files(std::string channel_name) {
    std::experimental::filesystem::path dir(project_directory + channel_name + native_separator);
    std::error_code ec;
    std::experimental::filesystem::remove_all(dir, ec);
}

void MainWindow::save_subscriptions() {
    std::cout << "Saving subscriptions." << std::endl;
    std::string file_path = project_directory + "subscriptions.json"s;
    std::ofstream save_file(file_path, std::ios::out);

    if(not save_file.is_open()) {
        std::cout << "Error opening file: " << file_path << std::endl;
        return;
    }

    try{
        // Recall channels is a map, QString -> podcast
        if(channels.empty()) {
            save_file.close();
            return;
        }

        save_file << "{\n";
        auto ch_itr = channels.begin();
        auto early_end = std::next(channels.end(),-1);

        while(ch_itr != early_end) {
            auto &[url, channel] = *ch_itr;
            save_file << "\"channel\" : {\n";
            channel.serialize_into(save_file);
            save_file << "},\n";
            ++ch_itr;
        }
        auto &[url, channel] = *ch_itr;
        save_file << "\"channel\" : {\n";
        channel.serialize_into(save_file);
        save_file << "}\n";

        save_file << "}";
        save_file.close();
    }
    catch(const std::exception &e) {
        std::cout << "An exception occurred: " << e.what() << std::endl;
    }

    return;
}

void MainWindow::seek() {
    float pos = ui->seek_slider->sliderPosition();
    pos = pos/std::max(1,ui->seek_slider->maximum());
    _audio_handle.perform([pos](audio_wrapper &player){player.seek_by_percent(pos);});
    seek_bar_lock.unlock();
}

void MainWindow::set_active_channel(const QModelIndex &index) {
    std::string channel_name = index.data(Qt::DisplayRole).toString().toStdString();

    auto itr = channels.find(channel_name);
    if(itr == channels.end())
        return;

    open_channel = channel_name;

    podcast &pod = itr->second;

    pod.populate(ui->episodeView);
}

void MainWindow::set_seek_bar_position(float percent) {
    std::unique_lock lock(seek_bar_lock, std::try_to_lock);

    if(lock.owns_lock())
        ui->seek_slider->setSliderPosition(static_cast<int>(percent * ui->seek_slider->maximum()));
}

void MainWindow::sync_audio_with_library_state() {
    switch(state) {
    case UserDesiredState::play :
        switch(_audio_handle.get_status()) {
        case PlayerStatus::playing:
            break;
        case PlayerStatus::paused: {
            int vol = volume;
            _audio_handle.perform([vol](audio_wrapper &player){player.play(); player.set_volume(vol);});
        } break;
        default: {
            state = UserDesiredState::stop;
            return;
        }
        }
        break;
    case UserDesiredState::pause :
        switch(_audio_handle.get_status()) {
        case PlayerStatus::playing:
            _audio_handle.perform([](audio_wrapper &player){player.pause();});
            break;

        default:
            break;
        }
        break;

    case UserDesiredState::stop :
        switch (_audio_handle.get_status()) {
        case PlayerStatus::playing:
            _audio_handle.perform([](audio_wrapper &player){player.pause();});
                        break;

        default:
            break;
    } break;
    default:
        std::cout << "music_libary.state has been corrupted." << std::endl;
        break;
    }
}

void MainWindow::sync_ui_with_audio_state() {

    // This function can get called before things are fully initialized.
    if(ui == nullptr or ui->seek_slider == nullptr or ui->play_button == nullptr)
        return;

    // Synchronizes UI seek bar with audio backend.
    float time_pos = _audio_handle.perform(
            [](audio_wrapper &player){ return player.get_percent_played(); }
        );

    set_seek_bar_position(time_pos);

    // Synchronizes play/pause push button
    switch( _audio_handle.get_status() ) {
    case PlayerStatus::playing:
        ui->play_button->setIcon(QIcon(":/icons/icons/pause.svg"));
        ui->play_button->update();
        break;
    default:
        ui->play_button->setIcon(QIcon(":/icons/icons/play.svg"));
        ui->play_button->update();
        break;
    }
}

void MainWindow::sync_ui_with_library_state() {
    ui->episodeView->model()->removeRows(0, ui->episodeView->model()->rowCount());
    ui->podcastView->model()->removeRows(0, ui->podcastView->model()->rowCount());

    ui->podcastView->model()->insertRows(0, static_cast<int>(channels.size()));

    int cur_row=0;
    for(auto &[title, channel] : channels) {
        QModelIndex index = ui->podcastView->model()->index(cur_row,0);
        ui->podcastView->model()->setData(index, QString::fromStdString(title), Qt::DisplayRole);
        ++cur_row;
    }

    ui->episodeView->update();
    ui->podcastView->update();
}

void MainWindow::on_play_button_clicked() {
    switch( state ) {
    case UserDesiredState::play:
        state = UserDesiredState::pause;
        break;
    default:
        state = UserDesiredState::play;
        break;
    }
}

void MainWindow::podcastViewContextMenu(QPoint p) {

    if(QModelIndex index = ui->podcastView->indexAt(p); index.isValid()) {
        QMenu menu {ui->podcastView->indexWidget(index)};
        auto itr = channels.find(index.data(Qt::DisplayRole).toString().toStdString());

        menu.addAction(QString("Fetch RSS"),
                       [this, url=itr->second.rss_url()](){
                            this->fetch_rss(url);
                        }
        );

        menu.addAction(QString("Remove podcast"),
                       [this, itr, row=index.row()](){
                            std::string channel_name = itr->first;
                            this->channels.erase(itr);
                            this->ui->podcastView->model()->removeRow(row);
                            this->remove_local_files(channel_name);
                        }
        );

        menu.addAction(QString("Remove local files"),
                       [this, itr](){
                            std::string channel_name = itr->first;
                            this->remove_local_files(channel_name);
                        }
        );

        menu.exec(ui->podcastView->mapToGlobal(p));
    }
}

void MainWindow::episodeViewContextMenu(QPoint p) {
    if(QModelIndex index = ui->episodeView->indexAt(p); index.isValid()) {
        QMenu menu { ui->episodeView->indexWidget(index) };

        menu.addAction(QString("Download"), [this, &index](){this->download(index);});
        menu.addAction(QString("Delete file"), [this, &index](){this->remove_local_file(index);});
        menu.addAction(QString("Play"), [this, &index](){this->download_or_play(index);});
        menu.exec(ui->episodeView->mapToGlobal(p));
    }
}
