#ifndef DEAR_IMGUI_WRAPPER
#define DEAR_IMGUI_WRAPPER
#include <map>
#include <mutex>
#include <string>

#include "dynamick_cast/thread_safe_interface.hpp"
#include "audio/audio_abstraction.hpp"
#include "gui/gui_abstraction.hpp"
#include "library/library.hpp"
#include "networking/getter.hpp"
#include "shared/user_desired_state.hpp"

struct SDL_Window;
struct ImGuiIO;

class dear_imgui_wrapper : public gui_abstraction {
public:
    dear_imgui_wrapper(int &argc, char **argv, thread_safe_interface<audio_abstraction> &&audio_handle);
    ~dear_imgui_wrapper();

    void sync_audio_with_library_state() override;
    void sync_ui_with_audio_state() override;
    void sync_ui_with_download_state() override;
    void sync_ui_with_library_state() override;

    void run() override;

private:

    void download_or_play(episode const & ep);
    void download(episode const &ep, std::string file_path, std::string url);

    void load_subscriptions() noexcept;
    void save_subscriptions();

    void fetch_rss(std::string url);
    void fetch_rss_dialog();

    void hotkey_handler();
    void toggle_state();

    static constexpr size_t buffer_size = 1024;

    bool should_continue;
    bool open_window;
    thread_safe_interface<audio_abstraction> audio_handle;
    SDL_Window * window;
    void * gl_context;
    ImGuiIO *io_context;

    library channels;
    std::mutex seek_bar_lock;
    std::mutex daemon_lock;
    UserDesiredState state;
    std::string open_channel;
    std::string home_path;
    std::string native_separator;
    std::string project_directory;
    getter get;
    std::unique_ptr<char[]> url_input_buffer;


    float vol;
    bool user_holds_volume_slider;
    float track_position;
    bool user_holds_track_slider;
    ImVec2 menu_size;

};

#endif // DEAR_IMGUI_WRAPPER