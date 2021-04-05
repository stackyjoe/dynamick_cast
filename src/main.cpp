
#include "audio/audio_interface.hpp"
#include "gui/qt5/mainwindow.hpp"
#include "audio/sfml/sfml_wrapper.hpp"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    audio_interface audio_handle = audio_interface::make<sfml_wrapper>();

    MainWindow w(audio_handle);
    w.show();

    a.exec();
    return EXIT_SUCCESS;
}
