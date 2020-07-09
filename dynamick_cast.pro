#-------------------------------------------------
#
# Project created by QtCreator 2019-04-03T12:50:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dynamick_cast
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++2a

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    podcast.cpp \
    episode.cpp \
    http_connection_resources.cpp \
    url_parser.cpp \
    string_functions.cpp \
    getter.cpp \
    contrib/sfml_mp3/music_adapter.cpp \
    download_shared_state.cpp

HEADERS += \
        mainwindow.hpp \
    podcast.hpp \
    episode.hpp \
    audio_interface.hpp \
    audio_wrapper.hpp \
    user_desired_state.hpp \
    http_connection_resources.hpp \
    url_parser.hpp \
    string_functions.hpp \
    getter.hpp \
    contrib/function2/function2.hpp \
    contrib/sfml_mp3/music_adapter.hpp \
    download_shared_state.hpp

*-g++ {
    LIBS += -lstdc++fs
}

*-clang++ {
    LIBS += -lc++fs
}

DEFINES += USE_SFML
DEFINES += USE_SFML_MP3_EXPERIMENTAL
#DEFINES += USE_AUDIERE

LIBS += -lboost_system
LIBS += -lcrypto
LIBS += -lssl
LIBS += -lpugixml

contains(DEFINES, USE_SFML) {
message("Using SFML audio backend.")
LIBS += -lsfml-audio            # SFML audio shared library
LIBS += -lsfml-system           # SFML system shared library, needed for sf::Time
LIBS += -lopenal                # dependency
LIBS += -lFLAC                  # dependency
LIBS += -lvorbisenc             # dependency
LIBS += -lvorbisfile            # dependency
LIBS += -lvorbis                # dependency
LIBS += -logg                   # dependency

SOURCES += audio_backends/sfml_wrapper.cpp
HEADERS += audio_backends/sfml_wrapper.hpp

}

contains(DEFINES, USE_SFML_MP3_EXPERIMENTAL) {
message("Using experimental SFML mp3 support")
LIBS += -lmpg123

SOURCES += contrib/sfml_mp3/mp3.cpp
HEADERS += contrib/sfml_mp3/mp3.hpp

}

contains(DEFINES, USE_AUDIERE) {
message("Using Audiere audio backend.")
LIBS += -laudiere
LIBS += -lFLAC
LIBS += -logg
LIBS += -lvorbis
LIBS += -lvorbisenc
LIBS += -lvorbisfile

SOURCES += audio_backends/audiere_wrapper.cpp
HEADERS += audio_backends/audiere_wrapper.hpp
}

FORMS += \
        mainwindow.ui

QMAKE_CXXFLAGS += -Wall -Wextra -pedantic-errors -Og -pg -fmodules-ts
INCLUDEPATH += /usr/local/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
