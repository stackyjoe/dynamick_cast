add_library(gui OBJECT gui_interface.hpp)

add_subdirectory(qt5)
target_link_libraries (gui PRIVATE qt5_gui_implementation)
