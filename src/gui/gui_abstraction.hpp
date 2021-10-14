#ifndef GUI_ABSTRACTION_HPP
#define GUI_ABSTRACTION_HPP

#include <mutex>

#include "dynamick_cast/thread_safe_interface.hpp"

#include "audio/audio_abstraction.hpp"

class gui_abstraction {
public:
    gui_abstraction() = default;
    virtual ~gui_abstraction() = default;

    virtual void sync_audio_with_library_state() = 0;
    virtual void sync_ui_with_audio_state() = 0;
    virtual void sync_ui_with_download_state() = 0;
    virtual void sync_ui_with_library_state() = 0;

    virtual void run() = 0;

    mutable std::mutex access_lock;
};

thread_safe_interface<gui_abstraction> make_gui(int argc, char ** argv, thread_safe_interface<audio_abstraction> audio);

#endif // GUI_ABSTRACTION_HPP
