#include <cstdlib>

#include "exprCalc/ExprCalc.h"


int main()
{
    std::string programText = 
        "var n = 500"
        "var sequence = map({0, n}, i -> (-1)^i / (2 * i + 1))"
        "var pi = 4 * reduce(sequence, 0, x y -> x + y)"
        "print \"pi = \""
        "out pi";
        
    const std::string simpleExpr = "1 + 2";
    
    ExprCalc::ParseProgram(simpleExpr);
    
    return EXIT_SUCCESS;
}
