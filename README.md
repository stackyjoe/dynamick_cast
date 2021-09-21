# dynamick_cast

Dynamick cast is a simple podcast subscription and playback application for the desktop. The current version of this has only been tested to build on FreeBSD, but it should build under Windows and most GNU/Linuxes without too much trouble.

Uses the following libraries:
+ CMake build system
+ Qt for the GUI.
+ SFML for Audio
    - SFML does not formally MP3 support yet. For now I'm using someone's experimental SoundFileReader plugin for SFML.
    - It has an optional Audiere audio backend.
    - Bug: on Ubuntu the application needs to be run with padsp for audiere to find the OSS audio device.
+ Boost libraries for networking
    - Currently this only uses Boost ASIO and Beast.
    - I've only built the current project with Boost version 1.75.
    - There is legacy code for XML parsing using ptree, but it wasn't very robust so I will likely get rid of it.
+ PugiXML for XML parsing
+ Function2
    - I use this because I need a replacement for std::function that can handle move-only function-objects.

Next milestones:
+ Improving modularity
    - GUI
        - Right now the application functionally needs Qt. I intend to make this a compile time point of customization, starting with Dear ImGui.
    - Library model
        - Add more structure to it.
    - Audio
        - Add wrappers for other audio libraries.
	- Figure out better seeking strategy for VBR files.

How to build:
+ Make a directory out and change to it in your terminal.

+ Execute `cmake ..` then `cmake --build .`

