#ifndef GUI_ABSTRACTION_HPP
#define GUI_ABSTRACTION_HPP

#include <memory>

class gui_abstraction {
public:
    gui_abstraction() = default;
    virtual ~gui_abstraction() = 0;

    virtual void sync_audio_with_library_state() = 0;
    virtual void sync_ui_with_audio_state() = 0;
    virtual void sync_ui_with_download_state() = 0;
    virtual void sync_ui_with_library_state() = 0;

    mutable std::mutex access_lock;
};

#endif // GUI_ABSTRACTION_HPP
