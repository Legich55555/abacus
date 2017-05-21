# pragma once

#include "ExprCalc.h"
#include "BracesParse.h"
#include "Universal.h"
#include <tao/pegtl.hpp>
#include <map>

namespace ExprCalc
{
using namespace tao::TAOCPP_PEGTL_NAMESPACE;

namespace Sequence
{

template<typename Input>
bool Parse(Input& input, const Variables& variables, Universal& result)
{
    char c = input.peek_char(0);
    if ('{' != c)
    {
        return false;
    }

    std::vector<SubExpr> subExpressions = ParseBraces(input, 0, BraceType::CURLY);

    if (subExpressions.size() != 2U)
    {
        throw parse_error("Invalid usage of { first, second }", input);
    }

    size_t bumped;
    Universal firstValue = Calculate(
                               input.current() + subExpressions[0].Offset,
                               subExpressions[0].Size,
                               "First sequence parameter",
                               bumped,
                               variables);

    if (firstValue.Type != Universal::Types::INTEGER)
    {
        throw parse_error("First sequence parameter is not valid.", input);
    }

    Universal secondValue = Calculate(
                                input.current() + subExpressions[1].Offset,
                                subExpressions[1].Size,
                                "Second sequence parameter",
                                bumped,
                                variables);

    if (secondValue.Type != Universal::Types::INTEGER)
    {
        throw parse_error("Second sequence parameter is not valid.", input);
    }

    result = Universal(firstValue.Integer, secondValue.Integer);

    input.bump(subExpressions.back().Offset + subExpressions.back().Size + 1U);

    return true;
}
}
}

