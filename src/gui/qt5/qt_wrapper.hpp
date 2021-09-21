#ifndef QT_WRAPPER_HPP
#define QT_WRAPPER_HPP

#include <QApplication>

#include "dynamick_cast/thread_safe_interface.hpp"
#include "gui/gui_abstraction.hpp"
#include "gui/qt5/mainwindow.hpp"


class qt_wrapper : public gui_abstraction {
    public:
    qt_wrapper(int argc, char **argv, thread_safe_interface<audio_abstraction> &&audio_handle);

    void sync_audio_with_library_state() override;
    void sync_ui_with_audio_state() override;
    void sync_ui_with_download_state() override;
    void sync_ui_with_library_state() override;

    void run() override;

    private:
    QApplication app;
    MainWindow window;
};


#endif // QT_WRAPPER_HPP