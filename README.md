# dynamick_cast

Dynamick cast is a simple podcast subscription and playback application for the desktop. The current version of this has only been tested to build on FreeBSD, but it should build under Windows and most GNU/Linuxes without too much trouble.

Uses the following libraries:
+ CMake build system
+ For the GUI there are two options roughly at feature parity. Qt5 and Dear ImGui.
+ SFML for Audio
    - SFML does not formally MP3 support yet. For now I'm using someone's experimental SoundFileReader plugin for SFML.
    - It has an optional Audiere audio backend.
        - Bug: on Ubuntu the application needs to be run with padsp for audiere to find the OSS audio device.
+ Boost libraries for networking
    - Currently this directly uses Boost ASIO and Beast, also needing threads and coroutines.
+ PugiXML for XML parsing
+ Boost JSON for JSON parsing.
+ Function2
    - I use this because I need a replacement for std::function that can handle move-only function-objects.

Next milestones:
+ Improving modularity
    - Audio
        - Add wrappers for other audio libraries.
	- Figure out better seeking strategy for VBR files.
    - GUI
        - Improve interface
    - Library
        - The program needs to save a JSON file of which podcasts the user is subscribed to. Currently this is hard coded and will probably not work for anyone else. This needs to be modified to be cross platform, and the code needs to be moved out of the GUI implementation into the library.

How to build:
+ See HowToBuild.md

