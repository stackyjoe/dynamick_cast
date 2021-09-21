add_library(dear_imgui_gui_implementation OBJECT)
set(this_dir ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(dear_imgui_gui_implementation PUBLIC
    ${imgui_directory}
    /usr/local/include/SDL2/
    ${imgui_directory}/examples/libs/gl3w/)

target_sources(dear_imgui_gui_implementation PUBLIC
    ${this_dir}/dear_imgui_wrapper.hpp
    ${this_dir}/dear_imgui_wrapper.cpp
    ${imgui_directory}/imgui.cpp
    ${imgui_directory}/imgui_demo.cpp
    ${imgui_directory}/imgui_draw.cpp
    ${imgui_directory}/imgui_tables.cpp
    ${imgui_directory}/imgui_widgets.cpp
    ${imgui_directory}/backends/imgui_impl_opengl3.h
    ${imgui_directory}/backends/imgui_impl_opengl3.cpp
    ${imgui_directory}/backends/imgui_impl_sdl.cpp
    ${imgui_directory}/backends/imgui_impl_sdl.h)

target_link_libraries(dear_imgui_gui_implementation PUBLIC GL SDL2)

target_compile_definitions(dear_imgui_gui_implementation PUBLIC IMGUI_IMPL_OPENGL_LOADER_GL3W=1)

find_package(X11 REQUIRED)

message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

target_include_directories(dear_imgui_gui_implementation PUBLIC ${X11_INCLUDE_DIR})

target_link_directories(dear_imgui_gui_implementation PUBLIC ${X11_LIBRARIES})

target_link_libraries(dear_imgui_gui_implementation PUBLIC ${X11_LIBRARIES})
