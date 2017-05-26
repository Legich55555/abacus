
add_executable(exprCalcTests TestsMain.cpp)
set_property(TARGET exprCalcTests PROPERTY CXX_STANDARD 14)
target_link_libraries(exprCalcTests exprCalc ${CMAKE_THREAD_LIBS_INIT})
