
target_sources(${PROJECT_NAME} PRIVATE audiere_wrapper.hpp)

# Necessary audio libraries
target_link_libraries(${PROJECT_NAME} PUBLIC openal ogg FLAC vorbisenc vorbisfile vorbis mpg123)
#

