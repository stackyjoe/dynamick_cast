#include "qt_wrapper.hpp"

qt_wrapper::qt_wrapper(int &argc, char **argv, thread_safe_interface<audio_abstraction> &&audio_handle)
    : app(argc,argv), window(std::move(audio_handle)) {

    }

void qt_wrapper::sync_audio_with_library_state() {
    window.sync_audio_with_library_state();
}

void qt_wrapper::sync_ui_with_audio_state() {
    window.sync_ui_with_audio_state();
}

void qt_wrapper::sync_ui_with_download_state() {
    window.sync_ui_with_download_state();
}

void qt_wrapper::sync_ui_with_library_state() {
    window.sync_ui_with_library_state();
}

void qt_wrapper::run() {
    window.show();
    app.exec();
}