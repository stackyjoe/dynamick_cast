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

CONFIG += c++1z

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    rss_getter.cpp \
    podcast.cpp \
    episode.cpp \
    audio_backends/audiere_wrapper.cpp \
    http_connection_resources.cpp \
    url_parser.cpp \
    string_functions.cpp

HEADERS += \
        mainwindow.hpp \
    podcast.hpp \
    episode.hpp \
    audio_backends/audiere_wrapper.hpp \
    audio_interface.hpp \
    audio_wrapper.hpp \
    user_desired_state.hpp \
    rss_getter.hpp \
    http_connection_resources.hpp \
    url_parser.hpp \
    string_functions.hpp

LIBS += -lc++experimental
LIBS += -laudiere
LIBS += -lFLAC
LIBS += -logg
LIBS += -lvorbis
LIBS += -lvorbisenc
LIBS += -lvorbisfile

FORMS += \
        mainwindow.ui

QMAKE_CXXFLAGS += -Wall -Wextra -pedantic-errors -O0 -pg
INCLUDEPATH += /usr/local/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
