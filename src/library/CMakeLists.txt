add_library(library STATIC)

set(library_sources episode.cpp  episode.hpp
    podcast.cpp podcast.hpp
    library.cpp library.hpp
    download_shared_state.cpp download_shared_state.hpp
    rss_parser.hpp)

list(TRANSFORM library_sources PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")


target_sources(library PUBLIC ${library_sources})

# use pugixml
find_package(pugiXML REQUIRED)
target_link_libraries (library PUBLIC pugixml::pugixml)
#

# use fmt
find_package(fmt REQUIRED)
target_link_libraries (library PUBLIC fmt::fmt)
#

#use boost json
find_package(Boost REQUIRED COMPONENTS json )

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(library PUBLIC Boost::json)
endif()
#