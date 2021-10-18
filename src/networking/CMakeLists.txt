add_library(networking STATIC beastly_connection.cpp getter.cpp url_parser.cpp)

#use Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

find_package(Boost COMPONENTS context coroutine REQUIRED)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(networking PUBLIC Boost::context Boost::coroutine)
endif()
#

#use pthreads
find_package (Threads)
target_link_libraries (networking PUBLIC ${CMAKE_THREAD_LIBS_INIT})
#

# use ssl, tsl
find_package(OpenSSL REQUIRED)
target_link_libraries (networking PUBLIC OpenSSL::SSL OpenSSL::Crypto)
#



