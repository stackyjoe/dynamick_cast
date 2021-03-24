cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 20)

project(dynamick_cast)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


set(SOURCE_FILES main.cpp url_parser.cpp getter.cpp beastly_connection.cpp string_functions.cpp mainwindow.cpp)
set(SOURCE_FILES ${SOURCE_FILES} download_shared_state.cpp episode.cpp podcast.cpp ./contrib/function2/function2.hpp)
set(HEADER_FILES url_parser.hpp getter.hpp beastly_connection.hpp string_functions.hpp mainwindow.hpp)
set(HEADER_FILES ${HEADER_FILES} download_shared_state.hpp egg_timer.hpp episode.hpp podcast.hpp audio_interface.hpp audio_wrapper.hpp)
set(UI_FILES mainwindow.ui resources.qrc)

set(USE_SFML ON)
set(USE_SFML_MP3_EXPERIMENTAL ON)

if(USE_SFML)
if(USE_SFML_MP3_EXPERIMENTAL)
add_definitions(-DUSE_SFML_MP3_EXPERIMENTAL)
set(SOURCE_FILES ${SOURCE_FILES} ./contrib/sfml_mp3/music_adapter.cpp ./contrib/sfml_mp3/mp3.cpp ./contrib/sfml_mp3/SoundFileReaderMp3.cpp)
set(HEADER_FILES ${HEADER_FILES} ./contrib/sfml_mp3/music_adapter.hpp ./contrib/sfml_mp3/mp3.hpp ./contrib/sfml_mp3/SoundFileReaderMp3.hpp)
endif()
set(SOURCE_FILES ${SOURCE_FILES} ./audio_backends/sfml_wrapper.cpp)
set(HEADER_FILES ${HEADER_FILES} ./audio_backends/sfml_wrapper.hpp)
elseif (USE_AUDIERE)
set(SOURCE_FILES ${SOURCE_FILES}./audio_backends/audiere_wrapper.cpp)
set(HEADER_FILES ${HEADER_FILES} ./audio_backends/audiere_wrapper.hpp)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${UI_FILES})

set(SHARED_CXX_FLAGS "-DBOOST_ASIO_DISABLE_CONCEPTS")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SHARED_CXX_FLAGS} -g -Og -Wall -Werror -pedantic-errors ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${SHARED_CXX_FLAGS} -O2")

target_include_directories(${PROJECT_NAME} PUBLIC "/usr/local/include")
target_link_directories(${PROJECT_NAME} PUBLIC "/usr/local/lib")

target_include_directories(${PROJECT_NAME} PUBLIC "./contrib/imgui" "./contrib/imgui/backends")

#use Qt
find_package(Qt5Widgets REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Widgets)

#use Boost

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

find_package(Boost 1.72.0 COMPONENTS)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()
#end Boost

#use pthreads
find_package (Threads)
target_link_libraries (${PROJECT_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
#end pthreads

#use ssl, tsl
find_package(OpenSSL REQUIRED)
target_link_libraries (${PROJECT_NAME} PUBLIC OpenSSL::SSL OpenSSL::Crypto)
#end ssl, tsl

#use sdl2
# Intend to switch from QMake to CMake and then eventually either replace Qt with ImGui or make it a compile time alternative.
#target_link_libraries(${PROJECT_NAME} PUBLIC SDL2 GL)

#use pugixml
find_package(pugiXML REQUIRED)
target_link_libraries (${PROJECT_NAME} PUBLIC pugixml)

#use fmt
find_package(fmt REQUIRED)
target_link_libraries (${PROJECT_NAME} PUBLIC fmt)

#use SFML
find_package(SFML COMPONENTS audio graphics window system REQUIRED)
target_link_libraries (${PROJECT_NAME} PUBLIC sfml-audio sfml-graphics sfml-system sfml-window)

target_link_libraries(${PROJECT_NAME} PUBLIC openal ogg FLAC vorbisenc vorbisfile vorbis mpg123)
