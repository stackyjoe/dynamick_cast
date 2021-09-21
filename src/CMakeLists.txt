if(${STRICT} STREQUAL "Yes")
set(SHARED_CXX_FLAGS "-Wall -Werror -pedantic-errors")
else()
set(SHARED_CXX_FLAGS "")
endif()

if(${TIDY} STREQUAL "Yes")
set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -header-filter=.*;
    -checks=-*,bugprone-*,cert-*,clang-analyzer-*,concurrency-*,cppcoreguidelines-*,hicpp-*,misc-*,modernize-*,performance-*,portability-*,readability-*,-modernize-use-trailing-return-type,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers)

endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SHARED_CXX_FLAGS} -g -Og")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${SHARED_CXX_FLAGS} -O2")

#Add executable target
add_executable(main main.cpp)

set_property(TARGET main PROPERTY POSITION_INDEPENDENT_CODE ON)

add_subdirectory(audio)
add_subdirectory(gui)
add_subdirectory(library)
add_subdirectory(networking)
add_subdirectory(shared)

# use fmt
find_package(fmt REQUIRED)
target_link_libraries (main PRIVATE fmt)
#

target_include_directories (main PRIVATE audio gui library shared networking)
target_link_libraries ( main PRIVATE audio gui library networking shared)

target_include_directories(main PRIVATE "/usr/local/include")
target_link_directories( main PRIVATE "/usr/local/lib")



