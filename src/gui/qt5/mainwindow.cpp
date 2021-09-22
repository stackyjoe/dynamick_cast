#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMetaObject>
#include <QStringListModel>
#include <QUrl>
#include <QTimer>

#include <boost/iostreams/stream.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <pugixml.hpp>

#include <filesystem>

#include <fmt/core.h>

#include "library/episode.hpp"
#include "gui/qt5/mainwindow.hpp"
#include "library/rss_parser.hpp"
#include "shared/string_functions.hpp"
#include "networking/url_parser.hpp"

using namespace std::string_literals;

/*
MainWindow::MainWindow(thread_safe_interface<audio_abstraction> &&audio_handle) :
    QMainWindow(nullptr),
    ui(std::make_unique<Ui::MainWindow>()),
    audio_handle(std::move(audio_handle)),
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
    QStringList ep_headers { QStringList { QString {""}, QString{"Newness"}, QString {"Episode title"} } };
    episode_item_model->setHorizontalHeaderLabels(ep_headers);
    ui->episodeView->setModel(episode_item_model);

    set_up_connections();
}
*/
MainWindow::~MainWindow() = default;

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

    // Not 100% sure why this is needed but it complains about the type not being registered and crashes without it.
    qRegisterMetaType<QVector<int> >("QVector<int>");
    connect(this, SIGNAL(requestEpisodeViewUpdate()), ui->episodeView, SLOT(update()));
    connect(this, SIGNAL(request_update_at(QModelIndex)), ui->episodeView, SLOT(update(QModelIndex)));
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
    audio_handle.perform([vol](audio_abstraction &interface){interface.set_volume(vol);});
}

void MainWindow::closeEvent([[maybe_unused]] QCloseEvent *ev) {
    quit();
}

void MainWindow::download(const QModelIndex &index) {
    auto itr = channels.find(open_channel);
    if(itr == channels.end()) {
        return;
    }

    auto * model = static_cast<QStandardItemModel*>(this->ui->episodeView->model());
    auto index_holding_data = model->index(index.row(), 2);

    QString episode_title = index_holding_data.data(Qt::DisplayRole).toString();
    podcast & cur_pod = itr->second;
    //std::string const * url = cur_pod.find_url(episode_title.toStdString());
    episode *ep = cur_pod.get_episode(episode_title.toStdString());
    if(ep == nullptr) {
        throw std::runtime_error("");
    }

    std::string url = ep->url();

    std::string file_name = ep->get_sanitized_file_name();

    if(not QDir::root().mkpath(QString::fromStdString(project_directory + native_separator + open_channel)))
        throw std::filesystem::filesystem_error("Could not create directory "s,
                                                              project_directory + native_separator + open_channel,
                                                              std::error_code());

    std::string local_path = project_directory + open_channel + native_separator + file_name;

    try {
        download(cur_pod, episode_title, url, local_path, index.row(), index);
    }
    catch(std::exception const &e) {
        std::cout << "An exception occurred: " << e.what() << std::endl;
    }
}

void MainWindow::download(podcast &cur_pod,
                          QString episode_title,
                          std::string url,
                          std::string file_dest,
                          [[maybe_unused]] int row,
                          QModelIndex index) {


//    fmt::print("Downloading url: {}\n", url);
    episode * ep = cur_pod.get_episode(episode_title.toStdString());

    if(ep == nullptr)
        throw std::invalid_argument("Can't find given episode title in the current podcast.");

    auto download_rights = ep->get_download_rights();
    //download_rights->set_index(index);

    auto maybe_lock = download_rights->try_lock();

    if(not maybe_lock.has_value()) {
        throw std::runtime_error("Could not acquire download_rights lock.");
    }

    auto * model = static_cast<QStandardItemModel*>(this->ui->episodeView->model());

    auto gui_callback = [this, download_rights, podcast_title = cur_pod.title(), episode_title, ep, model,index]() mutable -> void {

        if(index.isValid() and this->open_channel == podcast_title and ep->get_title() == episode_title.toStdString() ) {
            model->setData(index, QVariant{}, Qt::DecorationRole);
            populate_episode(index.row(), model, "", ep);
            this->request_update_at(index);
        }
    };

    download_rights->adopt_lock(std::move(*maybe_lock));
    maybe_lock = {};

    download_rights->set_gui_callback(std::move(gui_callback));

    // Since the completion handler holds ownership of the shared_state, we can just use a raw pointer here.
    auto prog = [shared_state=download_rights]([[maybe_unused]] size_t completed, [[maybe_unused]] size_t total) -> void {

        shared_state->set_bytes_completed(completed);
        shared_state->request_gui_update();
    };

    auto cmpl = [file_dest, this, pod_name=cur_pod.title(), download_rights=download_rights, &episode_title] (
            boost::beast::error_code const &ec,
            size_t bytes_read,
            beastly_connection &resources) mutable -> void {

        fmt::print("Attempting to clear the lock.\n");
        download_rights->clear_lock();

        if(ec or bytes_read == 0) {
            fmt::print("Error code or no bytes read. {}\n", ec.message());
            download_rights->request_gui_update();
            return;
        }

        std::ofstream output_file(file_dest, std::ios::binary);

        std::string const &body = resources.parser().body();

        output_file.write(body.c_str(), body.size());
        output_file.close();

        auto r = this->channels.find(pod_name);
        if(r == this->channels.end() or r->first != this->open_channel) {
            return;
        }
        auto &pod = r->second;
        auto * ep = pod.get_episode(pod_name);
        if(ep == nullptr) {
            return;
        }

        auto *model = static_cast<QStandardItemModel*>(ui->podcastView->model());
        auto list = model->findItems(episode_title, Qt::MatchExactly, 1);

        if(list.empty()) {
            return;
        }

        auto index = list.first();

        populate_episode(index->row(), model, project_directory+r->first+native_separator, ep);
        download_rights->request_gui_update();

    };

    auto f = get.get(url, std::move(prog), std::move(cmpl));
}

void MainWindow::download_or_play(const QModelIndex &index) {

    auto itr = channels.find(open_channel);
    if(itr == channels.end())
        return;

    QString episode_title = index.data(Qt::DisplayRole).toString();
    podcast & cur_pod = itr->second;
    //std::string const * url = cur_pod.find_url(episode_title.toStdString());
    auto * episode = cur_pod.get_episode(episode_title.toStdString());
    if(episode == nullptr)
        return;

    std::string url = episode->url();

    if(url.empty())
        return;

    std::string file_name = episode->get_sanitized_file_name();

    if(not QDir::root().mkpath(QString::fromStdString(project_directory + native_separator + open_channel)))
        throw std::filesystem::filesystem_error("Could not create directory "s,
                                                              project_directory + native_separator + open_channel,
                                                              std::error_code());

    std::string local_path = project_directory + open_channel + native_separator + file_name;

    QFileInfo file(QString::fromStdString(local_path));

    if(file.exists() and file.isFile()) {
        int vol = volume;
        if( audio_handle.perform(
            [local_path, vol](audio_abstraction &handle) -> bool {
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
            fmt::print("Error opening file: {}\n", local_path);
        }
    }
    else {
        try {
            download(cur_pod, episode_title, url, local_path, index.row(), index);
        }
        catch(std::exception const &e) {
            std::cout << e.what() << std::endl;
            return;
        }

        auto ep = cur_pod.get_episode(episode_title.toStdString());
        if(ep == nullptr) {
            populate_episode(index.row(),
                         static_cast<QStandardItemModel*>(ui->episodeView->model()),
                         project_directory + open_channel + native_separator,
                         ep);
        }
    }
}

void MainWindow::fetch_rss(std::string url) {
    fmt::print("fetch_rss({})\n", url);

    auto f = get.get(url,
                       []([[maybe_unused]] size_t read, [[maybe_unused]] size_t total){ return;},
                       [this, ui = ui.get(), url](boost::beast::error_code const & ec, [[maybe_unused]] size_t bytes_read, beastly_connection &res){
            if(ec) {
                fmt::print("An exception occurred: {}\n", ec.message());
                return;
            }

            auto rss = rss_parser(res.take_body());
            auto podcast = rss.parse(url);
            auto title = podcast.title();

            auto [itr, was_inserted] = channels.insert_or_assign(title, std::move(podcast));

            int cur_count = ui->podcastView->model()->rowCount();

            if( !was_inserted ) {
                for(int i = 0; i < cur_count; ++i) {
                    auto cur_index = ui->podcastView->model()->index(i, 0);
                    auto cur_podcast_title = ui->podcastView->model()->data(cur_index, Qt::DisplayRole).toString().toStdString();
                    if(title == cur_podcast_title) {
                        ui->podcastView->model()->removeRow(i);
                        cur_count = i;
                        break;
                    }
                }
            }

            ui->podcastView->model()->insertRow(cur_count);
            QModelIndex index = ui->podcastView->model()->index(cur_count,0);
            ui->podcastView->model()->setData(index, QString::fromStdString(title), Qt::DisplayRole);


            if( !was_inserted ) {
                // TODO: update ui->episodeView
                if(open_channel == title)
                    populate(ui->episodeView, project_directory, std::addressof(itr->second));
                std::cout << "Assigned " << title << std::endl;
                return;
            }
        }
    );
}

void MainWindow::load_subscriptions() {
    fmt::print("Loading subscriptions.\n");
    std::string file_path = project_directory + "subscriptions.json"s;
    std::ifstream save_file(file_path, std::ios::in);

    if(not save_file.is_open()) {
        fmt::print("Error opening file: {}\n", file_path);
        return;
    }

    try {
        boost::property_tree::ptree tree;
        boost::property_tree::read_json(save_file, tree);

        std::map<std::string, podcast> new_channels;

        auto end = tree.end();
        for(auto root = tree.begin(); root != end; ++root) {
            podcast channel(root->first);
            channel.fill_from_xml(root->second, channel.title());
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
    std::string const * url = cur_pod.find_url(episode_title.toStdString());

    if(url == nullptr)
        return;

    auto pos = url->find_last_of('/')+1;
    auto rpos = url->find_first_of('?');
    std::string file_name = url->substr(pos, rpos-pos);
    std::filesystem::path file(project_directory + open_channel + native_separator + file_name);
    std::error_code ec;
    std::filesystem::remove(file,ec);
}

void MainWindow::remove_local_files(std::string channel_name) {
    std::filesystem::path dir(project_directory + channel_name + native_separator);
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

void MainWindow::save_subscriptions() {
    fmt::print("Saving subscriptions.\n");
    std::string file_path = project_directory + "subscriptions.json"s;
    std::ofstream save_file(file_path, std::ios::out);

    if(not save_file.is_open()) {
        fmt::print("Error opening file: {}\n", file_path);
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
            (void)url;// Can't use [[maybe_unused]] attribute with structured bindings.
            save_file << "\"channel\" : {\n";
            channel.serialize_into(save_file);
            save_file << "},\n";
            ++ch_itr;
        }
        auto &[url, channel] = *ch_itr;
        (void)url; // Can't use [[maybe_unused]] attribute with structured bindings.
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
    audio_handle.perform([pos](audio_abstraction &player){player.seek_by_percent(pos);});
    seek_bar_lock.unlock();
}

void MainWindow::set_active_channel(const QModelIndex &index) {
    std::string channel_name = index.data(Qt::DisplayRole).toString().toStdString();

    auto itr = channels.find(channel_name);
    if(itr == channels.end())
        return;

    open_channel = channel_name;

    podcast &pod = itr->second;

    populate(ui->episodeView, project_directory, std::addressof(pod));
}

bool MainWindow::set_seek_bar_position(float percent) {
    std::unique_lock lock(seek_bar_lock, std::try_to_lock);

    if(lock.owns_lock())
        ui->seek_slider->setSliderPosition(static_cast<int>(percent * ui->seek_slider->maximum()));

    return lock.owns_lock();
}

void MainWindow::sync_audio_with_library_state() {
    switch(state) {
    case UserDesiredState::play :
        switch(audio_handle.perform_unsynchronized([](auto &h){return h.get_status();})) {
        case PlayerStatus::playing:
            break;
        case PlayerStatus::paused: {
            int vol = volume;
            audio_handle.perform([vol](audio_abstraction &player){player.play(); player.set_volume(vol);});
        } break;
        default: {
            state = UserDesiredState::stop;
            return;
        }
        }
        break;
    case UserDesiredState::pause :
        switch(audio_handle.perform_unsynchronized([](auto &h){return h.get_status();})) {
        case PlayerStatus::playing:
            audio_handle.perform([](audio_abstraction &player){player.pause();});
            break;

        default:
            break;
        }
        break;

    case UserDesiredState::stop :
        switch (audio_handle.perform_unsynchronized([](auto &h){return h.get_status();})) {
        case PlayerStatus::playing:
            audio_handle.perform([](audio_abstraction &player){player.pause();});
                        break;

        default:
            break;
    } break;
    default:
        fmt::print("music_libary.state has been corrupted.\n");
        break;
    }
}

void MainWindow::sync_ui_with_audio_state() {

    // This function can get called before things are fully initialized.
    if(ui == nullptr or ui->seek_slider == nullptr or ui->play_button == nullptr)
        return;

    // Synchronizes UI seek bar with audio backend.
    auto [dur, time_pos] = audio_handle.perform(
            [](audio_abstraction &player) {
                return std::make_pair(player.estimate_duration(), player.get_percent_played());
            }
        );

    if(not set_seek_bar_position(time_pos)) {
        ui->cur_time_label->setText(QString::fromStdString("~"+to_time(static_cast<int>(dur * ui->seek_slider->sliderPosition() / ui->seek_slider->maximum()))));
        ui->duration_label->setText(QString::fromStdString(to_time(dur)));
    }
    else {
        ui->cur_time_label->setText(QString::fromStdString("~"+to_time(static_cast<int>(dur*time_pos))));
        ui->duration_label->setText(QString::fromStdString(to_time(dur)));
    }


    // Synchronizes play/pause push button
    switch( audio_handle.perform_unsynchronized([](auto &h){return h.get_status();})) {
    case PlayerStatus::playing:
        ui->play_button->setIcon(QIcon(":/pause.svg"));
        ui->play_button->update();
        break;
    default:
        ui->play_button->setIcon(QIcon(":/play.svg"));
        ui->play_button->update();
        break;
    }
}

void MainWindow::sync_ui_with_download_state() {
    auto kv_pair = channels.find(open_channel);
    if(kv_pair == channels.end()) {
        return;
    }

    auto &p = kv_pair->second;
    populate_download_progress(std::addressof(p));
    emit requestEpisodeViewUpdate();
}

void MainWindow::sync_ui_with_library_state() {
    ui->podcastView->model()->removeRows(0, ui->podcastView->model()->rowCount());
    ui->podcastView->model()->insertRows(0, static_cast<int>(channels.size()));

    int cur_row=0;
    for(auto &[title, channel] : channels) {
        (void)channel;// Can't use [[maybe_unused]] attribute with structured bindings for older compilers.
        QModelIndex index = ui->podcastView->model()->index(cur_row,0);
        ui->podcastView->model()->setData(index, QString::fromStdString(title), Qt::DisplayRole);
        ++cur_row;
    }

    ui->episodeView->model()->removeRows(0, ui->episodeView->model()->rowCount());
    if(auto itr = channels.find(open_channel); itr != channels.end()) {
        auto &pod = itr->second;
        populate(ui->episodeView, project_directory, std::addressof(pod));
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
        auto podcast_name = index.data(Qt::DisplayRole).toString().toStdString();


        auto itr = channels.find(podcast_name);

        auto & podcast = itr->second;
        auto rss_url = podcast.rss_url();
        

        fmt::print("RSS URL={}\n", podcast.rss_url());

        menu.addAction(QString("Update feed"),
                       [this, url=rss_url](){
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
                            std::thread t([this, itr](){
                                std::string channel_name = itr->first;
                                this->remove_local_files(channel_name);
                            });
                            t.detach();
                        }
        );

        menu.exec(ui->podcastView->mapToGlobal(p));
    }
}

void MainWindow::episodeViewContextMenu(QPoint p) {
    if(QModelIndex index = ui->episodeView->indexAt(p); index.isValid()) {
        QMenu menu { ui->episodeView->indexWidget(index) };

        menu.addAction(QString("Download"), [ui=this, &index](){ui->get.post([ui, &index](){ui->download(index);});});
        menu.addAction(QString("Delete file"), [this, &index](){this->remove_local_file(index);});
        menu.addAction(QString("Play"), [this, &index](){this->download_or_play(index);});
        menu.exec(ui->episodeView->mapToGlobal(p));
    }
}

void MainWindow::populate(QTableView* tableview, std::string project_directory, podcast const * podcast) {
    auto * model = static_cast<QStandardItemModel *>(tableview->model());
    if(int count { model->rowCount() }; count > 0)
        model->removeRows(0,model->rowCount());

    model->insertRows(0, static_cast<int>(podcast->peek_items().size()));

    for(size_t i = 0; i < podcast->peek_items().size(); ++i) {
        populate_episode(i, model, project_directory + podcast->title() + "/"s, std::addressof(podcast->peek_items()[i]));
    }

}

void MainWindow::populate_episode(int row, QStandardItemModel *model, std::string directory, episode const * ep) {
    auto download_rights = ep->get_download_rights();
    auto &shared_state = *download_rights;
    QModelIndex index = model->index(row,0);

    std::string title = ep->get_title();

    if(index.isValid()) {
        auto maybe_lock = shared_state.try_lock();
        if(maybe_lock.has_value()) {
            auto file_name = ep->get_sanitized_file_name();
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


            double percent = 100.0*shared_state.get_bytes_completed() / std::max(1.0,static_cast<double>(shared_state.get_bytes_total()));
            model->setData(index, QVariant{}, Qt::DecorationRole);
            model->setData(index,
                           shared_state.get_bytes_total() > 0 ?
                               QString::number(percent, 'f', 1) + QString::fromLocal8Bit("%",1) :
                               QString::number(shared_state.get_bytes_completed()/1000000) + QString::fromLocal8Bit("MB",2),
                           Qt::DisplayRole);
        }
    }

    index = model->index(row,1);

    if(index.isValid() and not model->setData(index, QString::number(row), Qt::DisplayRole) )
        fmt::print("Failed to add item number {}.\n", row);

    index = model->index(row,2);

    if(index.isValid() and not model->setData(index, QString::fromStdString(title), Qt::DisplayRole))
            fmt::print("Failed to add {} to episodeView\n", title);
}

void MainWindow::populate_download_progress(int row, QStandardItemModel *model, episode const *ep) {
    QModelIndex index = model->index(row,0);

    auto download_rights = ep->get_download_rights();
    auto &shared_state = *download_rights;

    if(index.isValid()) {
        auto maybe_lock = shared_state.try_lock();
        if(not maybe_lock.has_value()) {

            double percent = 100.0*shared_state.get_bytes_completed() / std::max(1.0,static_cast<double>(shared_state.get_bytes_total()));

            model->setData(index,
                           shared_state.get_bytes_total() > 0 ?
                               QString::number(percent, 'f', 1) + QString::fromLocal8Bit("%",1) :
                               QString::number(shared_state.get_bytes_completed()/1000000) + QString::fromLocal8Bit("MB",2),
                           Qt::DisplayRole);
        }
    }
}

void MainWindow::populate_download_progress(podcast const *pod) {

        // Brittle! Should verify that the size of the tableview is correct.
    auto * model = static_cast<QStandardItemModel *>(ui->podcastView->model());

    for(size_t i = 0; i < pod->peek_items().size(); ++i) {
        populate_download_progress(static_cast<int>(i), model, std::addressof(pod->peek_items()[i]));
    }
}