QT       += testlib dbus
QT       -= gui

TARGET = scannertest
CONFIG+= console
CONFIG+= c++11
CONFIG-= app_bundle

TEMPLATE = app

SOURCES += scannertest.cpp
SOURCES += ../scanner_server/manager.cpp
SOURCES += ../scanner_server/scanner.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += ../scanner_server
