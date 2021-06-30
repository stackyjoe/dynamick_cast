
add_library(audio OBJECT audio_abstraction.hpp)

if(USE_SFML)

    if(USE_SFML_MP3_EXPERIMENTAL)
    endif()

    add_subdirectory(sfml)
    target_link_libraries(audio INTERFACE sfml_audio_implementation)

    if(USE_SFML_MP3_EXPERIMENTAL)
        target_link_libraries(audio INTERFACE sfml_mp3_implementation)
    endif()

elseif (USE_AUDIERE)
    add_subdirectory(audiere)
    target_link_libraries(audio PUBLIC audiere_audio_implementation)
endif()


