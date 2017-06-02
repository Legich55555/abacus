
add_library(exprCalc STATIC
    ExprCalc.h
    ExprCalc.cpp
    Universal.h
    Universal.cpp
    StmtParse.h 
    MapParse.h
    ReduceParse.h
    SequenceParse.h)
set_property(TARGET exprCalc PROPERTY CXX_STANDARD 14)

add_subdirectory(tests/)
