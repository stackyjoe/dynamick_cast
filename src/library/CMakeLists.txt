
add_library(library OBJECT)

target_sources(library PRIVATE episode.cpp  episode.hpp
    podcast.cpp podcast.hpp
    library.cpp library.hpp
    download_shared_state.cpp download_shared_state.hpp
    rss_parser.hpp)
target_link_libraries(library PUBLIC networking)

# use pugixml
find_package(pugiXML REQUIRED)
target_link_libraries (library PRIVATE pugixml)
#
