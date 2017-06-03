TARGET = exprCalc
TEMPLATE = lib
CONFIG+= staticlib

SOURCES += ExprCalc.cpp \
    Universal.cpp

HEADERS += ExprCalc.h \
    Universal.h \
    StmtParse.h \
    ExprParse.h \
    BinaryStack.h \
    MapParse.h \
    ReduceParse.h \
    SequenceParse.h

INCLUDEPATH += $$PWD/..
