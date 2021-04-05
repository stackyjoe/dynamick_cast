
add_library(audio OBJECT audio_wrapper.hpp audio_interface.hpp)
target_include_directories(audio PUBLIC ./)

if(USE_SFML)

    if(USE_SFML_MP3_EXPERIMENTAL)
    add_definitions(-DUSE_SFML_MP3_EXPERIMENTAL)
    endif()

    add_subdirectory(sfml)
    target_link_libraries(audio PRIVATE sfml_audio_implementation)


elseif (USE_AUDIERE)
    add_subdirectory(audiere)
    target_link_libraries(audio PRIVATE audiere_audio_implementation)
endif()


