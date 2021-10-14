#include "dynamick_cast/thread_safe_interface.hpp"
#include "audio/audio_abstraction.hpp"
#include "gui/gui_abstraction.hpp"

int main(int argc, char *argv[])
{


    auto gui_handle = make_gui(argc, argv, make_audio());


    gui_handle.perform([](auto &gui){gui.run();});

    return EXIT_SUCCESS;
}
