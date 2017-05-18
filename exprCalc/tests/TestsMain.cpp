#include <cstdlib>
#include <iostream>

#include "exprCalc/ExprCalc.h"

bool CheckExpression(const std::string& expression, const ExprCalc::Universal& expectedValue)
{
	if (ExprCalc::Calculate(expression) == expectedValue)
	{
		std::cout << "PASSED test for " << expression << std::endl;
		return true;
	}
	else
	{
		std::cout << "FAILED test for " << expression << std::endl;
		return false;
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
	
	CheckExpression("1.0", ExprCalc::Universal(1.f));
	CheckExpression("9.0", ExprCalc::Universal(9.f));
	CheckExpression("-1.0", ExprCalc::Universal(-1.f));
	CheckExpression("+1.0", ExprCalc::Universal(1.f));
	CheckExpression("-.1", ExprCalc::Universal(-0.1f));
	CheckExpression("+.1", ExprCalc::Universal(0.1f));
	CheckExpression("((1))", ExprCalc::Universal(1));
	CheckExpression("((1 + -2 + -1*+2.0))", ExprCalc::Universal(-3.0f));
	CheckExpression("1 + 1", ExprCalc::Universal(2));
	CheckExpression("1 +2.0", ExprCalc::Universal(3.0f));
	CheckExpression("-1", ExprCalc::Universal(-1));
	CheckExpression("-1.0", ExprCalc::Universal(-1.f));
	CheckExpression("-1 + 1", ExprCalc::Universal(0));
	CheckExpression("(((1 + 2) + 3) + 4)", ExprCalc::Universal(10));
	CheckExpression("(1 + -2 + -1*+2.0 + 10 / (3 + 2))", ExprCalc::Universal(-1.0f));
	CheckExpression("((+1 + -2 + -1*+2 + 10 / (3 + 2)) + (4/2 + 1))+12/3", ExprCalc::Universal(6));
	CheckExpression("(1 + -2 + -1*+2.0)", ExprCalc::Universal(-3.0f));
	CheckExpression("((1 + -2 + -1*+2.0))", ExprCalc::Universal(-3.0f));
	CheckExpression("((+1 + -2 + -1*+2.0 + 10 / (3 + 2)) + (4/2 + 1))+12/3", ExprCalc::Universal(6.0f));

	return 0;
}
