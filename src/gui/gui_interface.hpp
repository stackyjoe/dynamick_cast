#ifndef GUI_INTERFACE_HPP
#define GUI_INTERFACE_HPP

class gui_interface {
public:
    virtual ~gui_interface() = 0;

    virtual void sync_audio_with_library_state() = 0;
    virtual void sync_ui_with_audio_state() = 0;
    virtual void sync_ui_with_download_state() = 0;
    virtual void sync_ui_with_library_state() = 0;

protected:
    gui_interface() = default;

private:

};

#endif // GUI_INTERFACE_HPP
