# pragma once

#include "Universal.h"
#include <tao/pegtl.hpp>
#include <vector>
#include <cassert>

namespace ExprCalc
{
using namespace tao::TAOCPP_PEGTL_NAMESPACE;

struct SubExpr
{
    size_t Offset;
    size_t Size;
};

enum BraceType { CURLY, ROUND };

template<typename Input>
std::vector<SubExpr> ParseBraces(
    Input& input,
    const size_t offset, // offset is where the first external opening bracket is.
    const BraceType braceType)
{
    assert(!input.empty());
    assert(braceType == CURLY || braceType == ROUND);
    assert(input.peek_char(offset) == (braceType == CURLY ? '{' : '('));

    int roundBalance = 0;
    int curlyBalance = 0;

    std::vector<SubExpr> result;

    std::size_t idx = offset;
    std::size_t subExprStartIdx = idx + 1U;
    do
    {
        const char curr = input.peek_char(idx);

        if (curr == '(')
        {
            ++roundBalance;
        }
        else if (curr == ')')
        {
            --roundBalance;
        }
        else if (curr == '{')
        {
            ++curlyBalance;
        }
        else if (curr == '}')
        {
            --curlyBalance;
        }
        else if (curr == ',')
        {
            if ((braceType == CURLY && roundBalance == 0 && curlyBalance == 1U) ||
                (braceType == ROUND && roundBalance == 1 && curlyBalance == 0U))
            {
                result.push_back(SubExpr {subExprStartIdx, idx - subExprStartIdx});
                subExprStartIdx = idx + 1U;
            }
        }

        ++idx;
    } while (idx < input.size() && (roundBalance != 0 || curlyBalance != 0));


    if (roundBalance != 0 || curlyBalance != 0)
    {
        throw parse_error("There is no pair for opening bracket", input);
    }
    
    result.push_back(SubExpr {subExprStartIdx, idx - subExprStartIdx - 1U});

    return result;
}
}
