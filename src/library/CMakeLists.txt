
add_library(library OBJECT)

target_sources(library PRIVATE episode.cpp podcast.cpp rss_parser.hpp "../gui/qt5/download_shared_state.cpp")
target_link_libraries(library PUBLIC networking)

# use pugixml
find_package(pugiXML REQUIRED)
target_link_libraries (library PRIVATE pugixml)
#
