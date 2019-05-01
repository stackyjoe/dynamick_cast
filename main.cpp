#include <QApplication>

#include <gperftools/profiler.h>

#include "audio_interface.hpp"
#include "mainwindow.hpp"

int main(int argc, char *argv[])
{
    ProfilerStart("/usr/home/joe/cpu.info");
    QApplication a(argc, argv);

    extern audio_interface audio_handle;

    MainWindow w(audio_handle, nullptr);
    w.show();

    a.exec();
    ProfilerFlush();
    ProfilerStop();
    return EXIT_SUCCESS;
}
