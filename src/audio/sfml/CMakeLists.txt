
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
if(WIN32)
    find_package(flac CONFIG REQUIRED)
    target_link_libraries(sfml_audio_implementation PUBLIC FLAC::FLAC FLAC::FLAC++)

    find_package(Ogg CONFIG REQUIRED)
    target_link_libraries(sfml_audio_implementation PUBLIC Ogg::ogg)

    find_package(Vorbis CONFIG REQUIRED)
    target_link_libraries(sfml_audio_implementation PUBLIC Vorbis::vorbis Vorbis::vorbisenc Vorbis::vorbisfile)
endif()
if(UNIX)
    target_link_libraries(sfml_audio_implementation PUBLIC FLAC ogg vorbis vorbisenc vorbisfile)
endif()
#
