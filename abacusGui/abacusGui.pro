#-------------------------------------------------
#
# Project created by QtCreator 2017-05-29T08:59:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AbacusUI
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        ExecQueue.cpp

HEADERS += \
        mainwindow.h \
        ExecQueue.h

FORMS += \
        mainwindow.ui

LIBS += -L../exprCalc -lexprCalc

INCLUDEPATH += $$PWD/..
