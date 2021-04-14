set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SHARED_CXX_FLAGS} -g -Og -Wall -Werror -pedantic-errors ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${SHARED_CXX_FLAGS} -O2")

#Add executable target
add_executable(main main.cpp)

add_subdirectory(audio)
add_subdirectory(gui)
add_subdirectory(library)
add_subdirectory(networking)
add_subdirectory(shared)

# use fmt
find_package(fmt REQUIRED)
target_link_libraries (main PUBLIC fmt)
#

target_include_directories (main PUBLIC audio gui library shared networking)
target_link_libraries ( main PUBLIC audio gui library networking shared)

target_link_libraries ( main PUBLIC sfml_mp3_implementation)
target_link_libraries ( main PUBLIC sfml_audio_implementation)
target_link_libraries ( main PUBLIC qt5_gui_implementation)

target_include_directories(main PRIVATE "/usr/local/include")
target_link_directories( main PRIVATE "/usr/local/lib")

#target_include_directories(${PROJECT_NAME} PRIVATE ${AUTOGEN_BUILD_DIR})


set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -header-filter=.*;
    -checks=-*,bugprone-*,cert-*,clang-analyzer-*,concurrency-*,cppcoreguidelines-*,hicpp-*,misc-*,modernize-*,performance-*,portability-*,readability-*,-modernize-use-trailing-return-type,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers)


