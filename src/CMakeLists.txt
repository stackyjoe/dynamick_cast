if(${STRICT} STREQUAL "Yes")
if(UNIX)
set(SHARED_CXX_FLAGS "-Wall -Werror -pedantic-errors")
endif()
else()
set(SHARED_CXX_FLAGS "")
endif()

if(${TIDY} STREQUAL "Yes")
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_CXX_CLANG_TIDY
        clang-tidy;
        -header-filter=.*;
        -checks=-*,bugprone-*,cert-*,clang-analyzer-*,concurrency-*,cppcoreguidelines-*,hicpp-*,misc-*,modernize-*,performance-*,portability-*,readability-*,-modernize-use-trailing-return-type,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers)
    endif()
endif()

if(UNIX)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SHARED_CXX_FLAGS} -g -Og")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${SHARED_CXX_FLAGS} -O2")
endif()

#Add executable target
add_library(main STATIC main.cpp)

set_property(TARGET main PROPERTY POSITION_INDEPENDENT_CODE ON)

add_subdirectory(audio)
add_subdirectory(gui)
add_subdirectory(library)
add_subdirectory(networking)
add_subdirectory(shared)

# use fmt
find_package(fmt REQUIRED)
target_link_libraries (main PUBLIC fmt)
#

target_link_libraries ( main PUBLIC audio gui library networking shared)


