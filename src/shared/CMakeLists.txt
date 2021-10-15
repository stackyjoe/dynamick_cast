add_library(shared STATIC)


set(shared_sources string_functions.cpp string_functions.hpp)
list(TRANSFORM shared_sources PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")


target_sources(shared PUBLIC ${shared_sources})
