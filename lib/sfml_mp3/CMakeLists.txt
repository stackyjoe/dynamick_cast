add_library(sfml_mp3_implementation OBJECT)
set(this_dir ${CMAKE_CURRENT_SOURCE_DIR})

if(USE_SFML)
if(USE_SFML_MP3_EXPERIMENTAL)
    add_compile_definitions(USE_SFML_MP3_EXPERIMENTAL yes)

    target_sources(sfml_mp3_implementation PUBLIC
        ${this_dir}/SoundFileReaderMp3.cpp
        ${this_dir}/mp3.cpp
        ${this_dir}/music_adapter.cpp 
        ${this_dir}/music_adapter.hpp)

    find_package(SFML COMPONENTS audio graphics window system REQUIRED)
    target_link_libraries (sfml_mp3_implementation PUBLIC sfml-audio sfml-graphics sfml-system sfml-window)
    target_include_directories(sfml_mp3_implementation PUBLIC ./)

    # Necessary audio libraries
    target_link_libraries(sfml_mp3_implementation PUBLIC mpg123)
    #

endif()
endif()
