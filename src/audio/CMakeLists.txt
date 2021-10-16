add_library(audio STATIC audio_abstraction.hpp)

set_target_properties(audio PROPERTIES LINKER_LANGUAGE CXX)

if(USE_SFML)

    if(USE_SFML_MP3_EXPERIMENTAL)
    endif()

    add_subdirectory(sfml)
    target_link_libraries(audio PUBLIC sfml_audio_implementation)

    if(USE_SFML_MP3_EXPERIMENTAL)
        target_link_libraries(audio PUBLIC sfml_mp3_implementation)
    endif()

elseif (USE_AUDIERE)
    add_subdirectory(audiere)
    target_link_libraries(audio PUBLIC audiere_audio_implementation)
endif()


