add_library(qt5_gui_implementation OBJECT)

target_sources(qt5_gui_implementation PRIVATE mainwindow.cpp download_shared_state.cpp mainwindow.hpp)
target_sources(qt5_gui_implementation PRIVATE resources.qrc mainwindow.ui )

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} CMAKE_CURRENT_SOURCE_DIR)

#use Qt
find_package(Qt5Widgets REQUIRED)
target_link_libraries(qt5_gui_implementation PRIVATE Qt5::Widgets)
#
