# pragma once

#include <map>
#include <string>
#include "Universal.h"

namespace ExprCalc
{
    typedef std::map<std::string, Universal> Variables;
    
	Universal Calculate(const std::string& expression, const Variables& variables = {});
}
