#include "dynamick_cast/thread_safe_interface.hpp"
#include "audio/audio_abstraction.hpp"
#include "gui/qt5/mainwindow.hpp"
#include "audio/sfml/sfml_wrapper.hpp"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //audio_interface audio_handle = audio_interface::make<sfml_wrapper>();
    auto audio_handle = thread_safe_interface<audio_abstraction>::make<sfml_wrapper>();

    MainWindow w(std::move(audio_handle));
    w.show();

    a.exec();
    return EXIT_SUCCESS;
}
