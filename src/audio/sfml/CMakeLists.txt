
add_library(sfml_audio_implementation OBJECT)
target_sources(sfml_audio_implementation PRIVATE sfml_wrapper.cpp sfml_wrapper.hpp)


find_package(SFML COMPONENTS audio graphics window system REQUIRED)
target_link_libraries (sfml_audio_implementation PRIVATE sfml-audio sfml-graphics sfml-system sfml-window)
#target_include_directories(sfml_audio_implementation PUBLIC ./)

# Necessary audio libraries
target_link_libraries(sfml_audio_implementation PRIVATE openal ogg FLAC vorbisenc vorbisfile vorbis mpg123)
#
