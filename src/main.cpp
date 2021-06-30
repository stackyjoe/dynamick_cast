#include "dynamick_cast/thread_safe_interface.hpp"
#include "audio/audio_abstraction.hpp"
//#include "gui/qt5/qt_wrapper.hpp"
#include "gui/dear_imgui/dear_imgui_wrapper.hpp"
#include "audio/sfml/sfml_wrapper.hpp"

//#include <QApplication>


int main(int argc, char *argv[])
{

    //audio_interface audio_handle = audio_interface::make<sfml_wrapper>();
    auto audio_handle = thread_safe_interface<audio_abstraction>::make<sfml_wrapper>();

    //auto gui_handle = thread_safe_interface<gui_abstraction>::make<qt_wrapper>(argc, argv, std::move(audio_handle));

    auto gui_handle = thread_safe_interface<gui_abstraction>::make<dear_imgui_wrapper>(argc, argv, std::move(audio_handle));


    gui_handle.perform([](auto &gui){gui.run();});
    //MainWindow w(std::move(audio_handle));
    //w.show();

    //a.exec();
    return EXIT_SUCCESS;
}
