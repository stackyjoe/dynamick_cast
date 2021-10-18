# How to build

This uses CMake. Building it should be straightforward. Here are instructions for UNIX-like environments.

1. Create a build directory. From the root directory of the project you can execute.

    mkdir build && cd build

2. Configure the project. You need to tell it what audio and GUI libraries to use. Currently the working configuration is SFML for audio and Qt5 for GUI.

    cmake .. -DUSE_SFML=ON -DUSE_QT5=ON

There is also an audiere audio option, which should *just work*, and a Dear ImGui GUI option, which currently only has very limited functionality. If you are interested in other options feel free to do a feature request. There are some more options here as well, as a larger example:

    cmake .. -DUSE_SFML=ON -DUSE_DEAR_IMGUI=ON -DLTO=ON -DCMAKE_BUILD_TYPE=Release -DSTRICT=ON

On Windows I have only built this with Dear ImGui and using vcpkg to corral all the libraries and get them working with CMake. So if you wish to do that you'll probably need to run something like:

   cmake .. -DUSE_SFML=ON -DUSE_DEAR_IMGUI=ON -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake"

to ensure that CMake will correctly find the packages installed with CMake.

3. Build the project.

    cmake --build .

4. Run the executable

   ./src/main

