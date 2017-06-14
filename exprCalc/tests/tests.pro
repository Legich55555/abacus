TARGET = tests
TEMPLATE = app

SOURCES += TestsMain.cpp

unix: !osx: LIBS += -L../../exprCalc -lexprCalc
win32:!winrt: LIBS += -L../../exprCalc/debug -lexprCalc

tests.depends = exprCalc

INCLUDEPATH += $$PWD/../..
