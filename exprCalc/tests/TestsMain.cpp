#include <cstdlib>
#include <iostream>

#include "exprCalc/ExprCalc.h"

/*
 * @brief Check expresssions
 * @return Number errors (both parsing errors and execution errors)
 */
unsigned CheckExpression(
    const std::string& expression, 
    const ExprCalc::Variables& variables,
    const ExprCalc::Universal& expectedValue)
{
    if (ExprCalc::Calculate(expression, variables) == expectedValue)
    {
        std::cout << "PASSED test for \"" << expression << "\"" << std::endl;
        return 0U;
    }
    else
    {
        std::cout << "FAILED test for \"" << expression << "\"" << std::endl;
        return 1U;
    }
}

unsigned CheckInvalidExpression(
    const std::string& expression, 
    const ExprCalc::Variables& variables)
{
    if (!ExprCalc::Calculate(expression, variables).IsValid())
    {
        std::cout << "PASSED test for " << expression << std::endl;
        return 0U;
    }
    else
    {
        std::cout << "FAILED test for " << expression << std::endl;
        return 1U;
    }    
}

int main()
{
    std::string programText = 
    "var n = 500"
    "var sequence = map({0, n}, i -> (-1)^i / (2 * i + 1))"
    "var pi = 4 * reduce(sequence, 0, x y -> x + y)"
    "print \"pi = \""
    "out pi";
    
    unsigned errorsNumber = 0;

    errorsNumber +=  CheckInvalidExpression(
        "(a + UNDEFINED_VARIABLE)",
        {
            {"a", ExprCalc::Universal(1)},
            {"b", ExprCalc::Universal(5)},            
        });

    errorsNumber += CheckExpression(
        "(a + b)",
        {
            {"a", ExprCalc::Universal(1)},
            {"b", ExprCalc::Universal(5)},            
        },
        ExprCalc::Universal(6));
    
    errorsNumber += CheckExpression(
        "(a + b) - ccccccccc12",
        {
            {"a", ExprCalc::Universal(1)},
            {"b", ExprCalc::Universal(5)},            
            {"ccccccccc12", ExprCalc::Universal(6)},            
        },
        ExprCalc::Universal(0));
    
    errorsNumber += CheckExpression(" 1.0", {}, ExprCalc::Universal(1.f));
    errorsNumber += CheckExpression(" 1.0 ", {}, ExprCalc::Universal(1.f));
    errorsNumber += CheckExpression("1.0 ", {}, ExprCalc::Universal(1.f));
    errorsNumber += CheckExpression("1.0", {}, ExprCalc::Universal(1.f));
    errorsNumber += CheckExpression("9.0", {},  ExprCalc::Universal(9.f));
    errorsNumber += CheckExpression("-1.0", {},  ExprCalc::Universal(-1.f));
    errorsNumber += CheckExpression("+1.0", {}, ExprCalc::Universal(1.f));
    errorsNumber += CheckExpression("-.1", {},  ExprCalc::Universal(-0.1f));
    errorsNumber += CheckExpression("+.1", {},  ExprCalc::Universal(0.1f));
    errorsNumber += CheckExpression("-1", {},  ExprCalc::Universal(-1));
    errorsNumber += CheckExpression("-1.0", {},  ExprCalc::Universal(-1.f));
    
    errorsNumber += CheckExpression(" {  1 , 2 } ", {}, ExprCalc::Universal(1, 2));
    errorsNumber += CheckExpression(" { 1,2 }", {}, ExprCalc::Universal(1, 2));

    errorsNumber += CheckExpression("((1))", {},  ExprCalc::Universal(1));
    errorsNumber += CheckExpression("((1 + -2 + -1*+2.0))", {},  ExprCalc::Universal(-3.0f));
    errorsNumber += CheckExpression("1 + 1", {},  ExprCalc::Universal(2));
    errorsNumber += CheckExpression("1 +2.0", {},  ExprCalc::Universal(3.0f));
    errorsNumber += CheckExpression("+1*+2", {}, ExprCalc::Universal(2));
    errorsNumber += CheckExpression("-1 + 1", {},  ExprCalc::Universal(0));
    errorsNumber += CheckExpression("(((1 + 2) + 3) + 4)", {},  ExprCalc::Universal(10));
    errorsNumber += CheckExpression("(1 + -2 + -1*+2.0 + 10 / (3 + 2))", {},  ExprCalc::Universal(-1.0f));
    errorsNumber += CheckExpression("((+1 + -2 + -1*+2 + 10 / (3 + 2)) + (4/2 + 1))+12/3", {},  ExprCalc::Universal(6));
    errorsNumber += CheckExpression("(1 + -2 + -1*+2.0)", {},  ExprCalc::Universal(-3.0f));
    errorsNumber += CheckExpression("((1 + -2 + -1*+2.0))", {},  ExprCalc::Universal(-3.0f));
    errorsNumber += CheckExpression("((+1 + -2 + -1*+2.0 + 10 / (3 + 2)) + (4/2 + 1))+12/3", {},  ExprCalc::Universal(6.0f));
    errorsNumber += CheckExpression("1 + 2^(3+1)", {},  ExprCalc::Universal(17));
    errorsNumber += CheckExpression("1 + 2^(3.0+1)", {},  ExprCalc::Universal(17.f));
    errorsNumber += CheckExpression("1 + 2.0^(3+1)", {},  ExprCalc::Universal(17.f));
    errorsNumber += CheckExpression("1 + 2.0^(3+1.0)", {},  ExprCalc::Universal(17.f));
    
    if (errorsNumber != 0)
    {
        std::cout << "Failed " << errorsNumber << " tests." << std::endl;
    }
    else
    {
        std::cout << "All tests passed." << std::endl;
    }
    
    return (0 == errorsNumber) ? EXIT_SUCCESS : EXIT_FAILURE;
}
