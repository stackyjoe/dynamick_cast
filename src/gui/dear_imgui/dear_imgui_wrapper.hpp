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

class SDL_Window;
class ImGuiIO;

class dear_imgui_wrapper : public gui_abstraction {
    public:
    dear_imgui_wrapper(int &argc, char **argv, thread_safe_interface<audio_abstraction> &&audio_handle);
    ~dear_imgui_wrapper();

    void sync_audio_with_library_state() override;
    void sync_ui_with_audio_state() override;
    void sync_ui_with_download_state() override;
    void sync_ui_with_library_state() override;

    void run() override;

    void load_subscriptions() noexcept;

    void fetch_rss(std::string url);


private:
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

    void download_or_play(episode const & ep);
};

#endif // DEAR_IMGUI_WRAPPER