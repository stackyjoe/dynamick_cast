
add_library(sfml_audio_implementation STATIC)
set(this_dir ${CMAKE_CURRENT_SOURCE_DIR})

find_package(SFML COMPONENTS network audio graphics window system REQUIRED)

target_sources(sfml_audio_implementation PUBLIC
    ${this_dir}/sfml_wrapper.cpp
    ${this_dir}/sfml_wrapper.hpp)


if(SFML_FOUND)
  target_include_directories(sfml_audio_implementation PUBLIC ${SFML_INCLUDE_DIR})
  target_link_libraries (sfml_audio_implementation PUBLIC
    sfml-audio
    sfml-graphics
    sfml-system
    sfml-window)
endif()

# Necessary audio libraries
target_link_libraries(sfml_audio_implementation PUBLIC openal ogg FLAC vorbisenc vorbisfile vorbis)
#
