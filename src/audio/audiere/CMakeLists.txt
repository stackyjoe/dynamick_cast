add_library(audiere_audio_implementation STATIC)

target_sources(audiere PUBLIC audiere_wrapper.hpp audiere_wrapper.cpp)

# Necessary audio libraries
target_link_libraries(audiere PUBLIC openal ogg FLAC vorbisenc vorbisfile vorbis mpg123)
#

