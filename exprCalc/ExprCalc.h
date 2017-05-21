# pragma once

#include <map>
#include <string>
#include "Universal.h"

namespace ExprCalc
{
    typedef std::map<std::string, Universal> Variables;
    
    Universal Calculate(
        const char* expressionData,
        std::size_t size,
        const std::string& exprName,
        const Variables& variables);
    
    Universal Calculate(
        const std::string& expression,
        const std::string& exprName,
        const Variables& variables = {});

    Universal Calculate(
        const std::string& expression,
        const Variables& variables = {});
}
