#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QContextMenuEvent>
#include <QMainWindow>

#include <future>
#include <map>
#include <thread>

#include "ui_mainwindow.h"

#include "audio_interface.hpp"
#include "podcast.hpp"
#include "getter.hpp"
#include "user_desired_state.hpp"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(audio_interface &audio_handle);

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
    [[noreturn]] void quit();
    void remove_local_file(const QModelIndex &index);
    void remove_local_files(std::string channel_name);
    void save_subscriptions();
    void seek();
    void set_active_channel(const QModelIndex &podcast_index);
    bool set_seek_bar_position(float percent);
    void set_up_connections();
    void sync_audio_with_library_state();
    void sync_ui_with_audio_state();
    void sync_ui_with_download_state();
    void sync_ui_with_library_state();


    std::unique_ptr<Ui::MainWindow> ui;
    std::map<std::string, podcast> channels;
    std::mutex seek_bar_lock;
    std::mutex daemon_lock;
    audio_interface &_audio_handle;
    UserDesiredState state;
    std::string open_channel;
    std::string home_path;
    std::string native_separator;
    std::string project_directory;
    getter get;
    std::thread daemon;
    std::atomic<int> volume;

    static constexpr size_t maximum_allowed_bytes_between_updates = 100000;

};

#endif // MAINWINDOW_HPP
