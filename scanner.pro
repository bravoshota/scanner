TEMPLATE = subdirs
CONFIG += ordered
win32:CONFIG += console

SUBDIRS = scanner_client
SUBDIRS+= scanner_server
SUBDIRS+= scanner_tests

HEADERS += common.h
