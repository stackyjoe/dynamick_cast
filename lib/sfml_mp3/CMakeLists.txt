add_library(sfml_mp3_implementation STATIC)

if(USE_SFML AND USE_SFML_MP3_EXPERIMENTAL)
    add_compile_definitions(USE_SFML_MP3_EXPERIMENTAL Yes)

    set(sfml_mp3_sources SoundFileReaderMp3.cpp  SoundFileReaderMp3.hpp
        mp3.cpp mp3.hpp
        music_adapter.cpp music_adapter.hpp)

    list(TRANSFORM sfml_mp3_sources PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

    target_sources(sfml_mp3_implementation PUBLIC ${sfml_mp3_sources})

    find_package(SFML COMPONENTS audio graphics window system REQUIRED)
    target_link_libraries (sfml_mp3_implementation PUBLIC sfml-audio sfml-graphics sfml-system sfml-window)
    target_include_directories(sfml_mp3_implementation PUBLIC ./)

    # Necessary audio libraries
    target_link_libraries(sfml_mp3_implementation PUBLIC mpg123)
    #

endif()

