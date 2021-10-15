add_library(dear_imgui_gui_implementation STATIC)

target_include_directories(dear_imgui_gui_implementation PUBLIC
    ${imgui_directory}
    /usr/local/include/SDL2/
    ${imgui_directory}/examples/libs/gl3w/)

set(imgui_wrapper_sources dear_imgui_wrapper.cpp dear_imgui_wrapper.hpp)
list(TRANSFORM imgui_wrapper_sources PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")


set(imgui_sources imgui.cpp imgui.h imgui_demo.cpp imgui_internal.h
    imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp)
list(TRANSFORM imgui_sources PREPEND "${imgui_directory}/")


set(imgui_backend_sources imgui_impl_opengl3.h imgui_impl_opengl3.cpp 
    imgui_impl_sdl.cpp imgui_impl_sdl.h)
list(TRANSFORM imgui_backend_sources PREPEND "${imgui_directory}/backends/")



target_sources(dear_imgui_gui_implementation PUBLIC ${imgui_backend_sources} ${imgui_sources} ${imgui_wrapper_sources})

target_link_libraries(dear_imgui_gui_implementation PUBLIC GL SDL2)

target_compile_definitions(dear_imgui_gui_implementation PUBLIC IMGUI_IMPL_OPENGL_LOADER_GL3W=1)

find_package(X11 REQUIRED)

message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

target_include_directories(dear_imgui_gui_implementation PUBLIC ${X11_INCLUDE_DIR})

target_link_directories(dear_imgui_gui_implementation PUBLIC ${X11_LIBRARIES})

target_link_libraries(dear_imgui_gui_implementation PUBLIC ${X11_LIBRARIES})
