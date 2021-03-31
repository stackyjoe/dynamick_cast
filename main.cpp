#include <QApplication>

#include "audio_interface.hpp"
#include "mainwindow.hpp"
#include "audio_backends/sfml_wrapper.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    audio_interface audio_handle = audio_interface::make<sfml_wrapper>();


    MainWindow w(audio_handle);
    w.show();

    a.exec();
    return EXIT_SUCCESS;
}
