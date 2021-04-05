
add_library(library OBJECT)

target_sources(library PRIVATE episode.cpp podcast.cpp rss_parser.hpp)
target_link_libraries(library PUBLIC qt5_gui_implementation networking)

#use Qt
find_package(Qt5Widgets REQUIRED)
target_link_libraries(library PUBLIC Qt5::Widgets)
#

# use pugixml
find_package(pugiXML REQUIRED)
target_link_libraries (library PRIVATE pugixml)
#
