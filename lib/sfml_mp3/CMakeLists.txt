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

    
    find_package(OpenAL CONFIG REQUIRED)
    target_link_libraries(sfml_mp3_implementation PUBLIC OpenAL::OpenAL)

    target_include_directories(sfml_mp3_implementation PUBLIC ./)

    # Necessary audio libraries
    if(WINDOWS)
        find_package(mpg123 CONFIG REQUIRED)
        target_link_libraries(sfml_mp3_implementation PUBLIC MPG123::libmpg123 MPG123::libout123 MPG123::libsyn123)
    endif()
    # CMake aware mpg123 seems to be Windows only in October 2021. Likely to change in the next few months.
    if(UNIX)
        target_link_libraries(sfml_mp3_implementation PUBLIC mpg123 out123 syn123)
    endif()
    #

endif()

