add_library(sfml_mp3_implementation OBJECT)

if(USE_SFML)
if(USE_SFML_MP3_EXPERIMENTAL)
    add_compile_definitions(USE_SFML_MP3_EXPERIMENTAL yes)

    target_sources(sfml_mp3_implementation PRIVATE SoundFileReaderMp3.cpp mp3.cpp music_adapter.cpp music_adapter.hpp)

    find_package(SFML COMPONENTS audio graphics window system REQUIRED)
    target_link_libraries (sfml_mp3_implementation PRIVATE sfml-audio sfml-graphics sfml-system sfml-window)
    target_include_directories(sfml_mp3_implementation PUBLIC ./)

    # Necessary audio libraries
    target_link_libraries(sfml_mp3_implementation PRIVATE openal ogg FLAC vorbisenc vorbisfile vorbis mpg123)
    #

endif()
endif()
