# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "src/gui/qt5/CMakeFiles/qt5_gui_implementation_autogen.dir/AutogenUsed.txt"
  "src/gui/qt5/CMakeFiles/qt5_gui_implementation_autogen.dir/ParseCache.txt"
  "src/gui/qt5/qt5_gui_implementation_autogen"
  )
endif()
