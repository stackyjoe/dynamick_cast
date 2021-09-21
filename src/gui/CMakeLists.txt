add_library(gui OBJECT gui_abstraction.hpp)

if(USE_QT5)
add_subdirectory(qt5)
target_link_libraries (gui INTERFACE qt5_gui_implementation)
endif()

if(USE_DEAR_IMGUI)
add_subdirectory(dear_imgui)
target_link_libraries (gui INTERFACE dear_imgui_gui_implementation)
endif()
