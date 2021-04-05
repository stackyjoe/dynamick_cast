cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)

project(dynamick_cast)

set(USE_SFML ON)
set(USE_SFML_MP3_EXPERIMENTAL ON)

include_directories(./include ./src ./lib)

add_subdirectory(lib)
add_subdirectory(src)
