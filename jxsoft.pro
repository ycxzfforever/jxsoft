#-------------------------------------------------
#
# Project created by QtCreator 2016-12-22T16:29:01
#
#-------------------------------------------------

QT       += core
QT       += sql
QT       += serialport

QT       -= gui

TARGET = jxsoft
CONFIG   += console
CONFIG   += c++11
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    src/log.cpp \
    src/main.cpp \
    src/readwriteconf.cpp \
    src/serial.cpp \
    src/communication.cpp \    
    src/mainobj.cpp \
    src/pressfolderthread.cpp

HEADERS += \
    include/kernel.h \
    include/log.h \
    include/readwriteconf.h \
    include/serial.h \
    include/communication.h \
    include/mainobj.h \
    include/pressfolderthread.h
