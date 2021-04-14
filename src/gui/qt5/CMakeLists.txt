set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


add_library(qt5_gui_implementation OBJECT)

target_sources(qt5_gui_implementation PRIVATE mainwindow.cpp mainwindow.hpp download_shared_state.cpp download_shared_state.hpp)
target_sources(qt5_gui_implementation PRIVATE resources.qrc mainwindow.ui )

#use Qt
find_package(Qt5Widgets REQUIRED)
target_link_libraries(qt5_gui_implementation PRIVATE Qt5::Widgets)
#
