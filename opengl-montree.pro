#-------------------------------------------------
#
# Project created by QtCreator 2013-01-25T17:09:39
#
#-------------------------------------------------

TEMPLATE = app
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++0x
SOURCES += main.cpp

HEADERS += \
    backend.h

SOURCES += \
    backend.cpp

HEADERS += \
    rtree.h

SOURCES += \
    rtree.cpp

HEADERS += \
    mymainwindow.h

SOURCES += \
    mymainwindow.cpp

FORMS += \
    mymainwindow.ui

HEADERS += \
    qtmodules.h

HEADERS += \
    views.h

SOURCES += \
    views.cpp


#COMMENT OUT THESE LINES IF YOU DON'T WANT A STATIC BUILD.

#win32{
#TEMPLATE = app
#CONFIG+=static
#DEFINES += ON_WINDOWS_STATIC
# QTPLUGIN     +=  qico qsvg
#}
