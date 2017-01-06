QT += core
QT += dbus
QT -= gui

TARGET = scanner
CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    manager.cpp \
    scanner.cpp

HEADERS += \
    manager.h \
    scanner.h \
    interface.h
