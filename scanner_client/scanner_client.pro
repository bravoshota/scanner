#-------------------------------------------------
#
# Project created by QtCreator 2016-12-27T23:07:18
#
#-------------------------------------------------

QT += core gui
QT += dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scanner_client
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui
