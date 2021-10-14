cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)

project(dynamick_cast)

if(USE_SFML)
    add_compile_definitions(USE_SFML)

    if((NOT DEFINED USE_SFML_MP3) OR (USE_SFML_MP3 STREQUAL ON) OR (USE_SFML_MP3 STREQUAL Yes))
        set(USE_SFML_MP3_EXPERIMENTAL ON)
        add_compile_definitions(USE_SFML_MP3_EXPERIMENTAL)
    endif()
endif()

if((USE_QT5 STREQUAL ON) OR (USE_QT5 STREQUAL Yes))
    set(USE_QT5 ON)
    add_compile_definitions(USE_QT5)
endif()

if((USE_DEAR_IMGUI STREQUAL ON) OR (USE_DEAR_IMGUI STREQUAL Yes))
    set(USE_DEAR_IMGUI ON)
    add_compile_definitions(USE_DEAR_IMGUI)
endif()

if(NOT DEFINED TIDY)
    set(TIDY No)
endif()

if(NOT DEFINED STRICT)
    set(STRICT No)
endif()


if(UNIX)
include_directories(SYSTEM "/usr/local/include")
link_directories(SYSTEM "/usr/local/lib")
endif()


include_directories(./include ./src ./lib)

add_subdirectory(lib)
add_subdirectory(src)
