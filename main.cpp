#include <QApplication>

#include "audio_interface.hpp"
#include "mainwindow.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    extern audio_interface audio_handle;

    MainWindow w(audio_handle, nullptr);
    w.show();

    a.exec();
    return EXIT_SUCCESS;
}
