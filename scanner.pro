TEMPLATE = subdirs
CONFIG += ordered
win32:CONFIG += console
SUBDIRS = scanner_client/scanner_client.pro scanner_server/scanner_server.pro

HEADERS += common.h
