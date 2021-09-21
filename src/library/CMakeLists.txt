
add_library(library OBJECT)

target_sources(library PRIVATE episode.cpp podcast.cpp rss_parser.hpp)
target_link_libraries(library PUBLIC networking)

# use pugixml
find_package(pugiXML REQUIRED)
target_link_libraries (library PRIVATE pugixml)
#
