cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)

project(dynamick_cast)

if(NOT DEFINED USE_SFML)
    set(USE_SFML ON)
    add_compile_definitions(USE_SFML)
endif()

if(NOT DEFINED USE_SFML_MP3_EXPERIMENTAL)
    set(USE_SFML_MP3_EXPERIMENTAL ON)
    add_compile_definitions(USE_SFML_MP3_EXPERIMENTAL)
endif()

if(NOT DEFINED USE_QT5)
    set(USE_QT5 ON)
    add_compile_definitions(USE_QT5)
endif()

if(NOT DEFINED USE_DEAR_IMGUI)
    set(USE_DEAR_IMGUI OFF)
    add_compile_definitions(USE_DEAR_IMGUI)
endif()

if(NOT DEFINED TIDY)
    set(TIDY No)
endif()

if(NOT DEFINED STRICT)
    set(STRICT No)
endif()


include_directories(./include ./src ./lib)

add_subdirectory(lib)
add_subdirectory(src)
