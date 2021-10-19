#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QContextMenuEvent>
#include <QDir>
#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>

#include <future>
#include <map>
#include <thread>

#include "dynamick_cast/thread_safe_interface.hpp"

#include "audio/audio_abstraction.hpp"
#include "library/library.hpp"
#include "networking/getter.hpp"
#include "shared/user_desired_state.hpp"

#include "ui_mainwindow.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

signals:
    void request_update_at(QModelIndex);
    void requestEpisodeViewUpdate();

private:
    void add_rss_from_dialog();
    void change_volume(int vol);
    void closeEvent(QCloseEvent *ev) override;
    void podcastViewContextMenu(QPoint p);
    void episodeViewContextMenu(QPoint p);
    void download(const QModelIndex &episode_index);
    void download(podcast &cur_pod,
                  QString episode_title,
                  std::string url,
                  std::string file_dest,
                  int row,
                  QModelIndex index);
    void download_or_play(const QModelIndex &episode_index);
    void fetch_rss(std::string url);
    void load_subscriptions();
    void on_play_button_clicked();
    void populate_episode_view();
    void populate(QTableView* tableview, std::string project_directory, podcast const * podcast);
    void populate_episode(int row, QStandardItemModel *model, std::string directory, episode const * ep);
    void populate_download_progress(int row, QStandardItemModel *model, episode const *ep);
    void populate_download_progress(podcast const *pod);


    [[noreturn]] void quit();
    void remove_local_file(const QModelIndex &index);
    void remove_local_files(std::string channel_name);
    void save_subscriptions();
    void seek();
    void set_active_channel(const QModelIndex &podcast_index);
    bool set_seek_bar_position(float percent);
    void set_up_connections();



    std::unique_ptr<Ui::MainWindow> ui;
    library channels;
    std::mutex seek_bar_lock;
    std::mutex daemon_lock;
    thread_safe_interface<audio_abstraction> audio_handle;
    UserDesiredState state;
    std::string open_channel;
    getter get;
    std::thread daemon;
    std::atomic<int> volume;
    std::string project_directory;
    std::string native_separator;

    static constexpr size_t maximum_allowed_bytes_between_updates = 100000;

    public: 
    MainWindow() = default;

    explicit MainWindow(thread_safe_interface<audio_abstraction> &&audio_handle)
    :
    QMainWindow(nullptr),
    ui(std::make_unique<Ui::MainWindow>()),
    audio_handle(std::move(audio_handle)),
    state(UserDesiredState::stop),
    home_path(QDir::homePath().toStdString()),
    native_separator(std::string("")+QDir::separator().toLatin1()),
    project_directory(home_path + std::string("/.local/share/applications/dynamick-cast/")),
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
    auto [afp, ns] = library_handle.perform([](auto &l){
        return std::make_pair(l.app_file_path(), l.native_sep());
    });
    project_directory = afp;
    native_separator = ns;

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
    load_subscriptions();
}
    ~MainWindow();
    void sync_audio_with_library_state();
    void sync_ui_with_audio_state();
    void sync_ui_with_download_state();
    void sync_ui_with_library_state();

};


#endif // MAINWINDOW_HPP
