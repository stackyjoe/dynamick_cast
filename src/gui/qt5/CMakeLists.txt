set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(qt5_gui_implementation STATIC resources.qrc mainwindow.ui mainwindow.hpp)

target_sources(qt5_gui_implementation PUBLIC)

set(qt_gui_sources mainwindow.cpp 
    qt_wrapper.cpp qt_wrapper.hpp 
    download_shared_state.hpp)

list(TRANSFORM qt_gui_sources PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

SET(UIC_DIR "${CMAKE_CURRENT_BINARY_DIR}/qt5_gui_implementation_autogen/")

target_sources(qt5_gui_implementation PUBLIC ${qt_gui_sources})

set_property(TARGET qt5_gui_implementation PROPERTY POSITION_INDEPENDENT_CODE ON)

#use Qt
find_package(Qt5Widgets REQUIRED)
target_link_libraries(qt5_gui_implementation PUBLIC Qt5::Widgets)
target_include_directories(qt5_gui_implementation INTERFACE ${Qt5Widgets_INCLUDE_DIRS} "${UIC_DIR}/include/")
#
