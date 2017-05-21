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
// struct SubExpr
// {
//     size_t Offset;
//     size_t Size;
// };
// 
// enum BracketType { CURLY, ROUND };
// 
// template<typename Input>
// std::vector<SubExpr> ParseBrackets(
//     Input& input,
//     const size_t offset, // offset is where the first external opening bracket is.
//     const BracketType bracketType)
// {
//     assert(!input.empty());
//     assert(bracketType == CURLY || bracketType == ROUND);
//     assert(input.peek_char(offset) == (bracketType == CURLY ? '{' : '('));
// 
//     int roundBalance = 0;
//     int curlyBalance = 0;
// 
//     std::vector<SubExpr> result;
// 
//     std::size_t idx = offset;
//     std::size_t subExprStartIdx = idx + 1U;
//     do
//     {
//         const char curr = input.peek_char(idx);
// 
//         if (curr == '(')
//         {
//             ++roundBalance;
//         }
//         else if (curr == ')')
//         {
//             --roundBalance;
//         }
//         else if (curr == '{')
//         {
//             ++curlyBalance;
//         }
//         else if (curr == '}')
//         {
//             --curlyBalance;
//         }
//         else if (curr == ',')
//         {
//             if ((bracketType == CURLY && roundBalance == 0 && curlyBalance == 1U) ||
//                 (bracketType == ROUND && roundBalance == 1 && curlyBalance == 0U))
//             {
//                 result.push_back(SubExpr {subExprStartIdx, idx - 1U});
//                 subExprStartIdx = idx + 1U;
//             }
//         }
// 
//         ++idx;
//     } while (idx < input.size() && (roundBalance != 0 || curlyBalance != 0));
// 
// 
//     if (roundBalance != 0 || curlyBalance != 0)
//     {
//         throw parse_error("There is no pair for opening bracket", input);
//     }
//     
//     result.push_back(SubExpr {subExprStartIdx, idx - 1U});
// 
//     return result;
// }

template<typename Input>
bool Parse(Input& input, const Variables& variables, Universal& result)
{
    char c = input.peek_char(0);
    if ('{' != c)
    {
        return false;
    }

    std::vector<SubExpr> subExpression = ParseBrackets(input, 0, BraceType::CURLY);

    if (subExpression.size() != 2U)
    {
        throw parse_error("Invalid usage of { first, second }", input);
    }

    Universal firstValue = Calculate(
                               input.current() + subExpression[0].Offset,
                               subExpression[0].Size,
                               "First sequence parameter",
                               variables);

    if (firstValue.Type != Universal::Types::INTEGER)
    {
        throw parse_error("First sequence parameter is not valid.", input);
    }

    Universal secondValue = Calculate(
                                input.current() + subExpression[1].Offset,
                                subExpression[1].Size,
                                "Second sequence parameter",
                                variables);

    if (secondValue.Type != Universal::Types::INTEGER)
    {
        throw parse_error("Second sequence parameter is not valid.", input);
    }

    result = Universal(firstValue.Integer, secondValue.Integer);

    input.bump(subExpression[1].Offset + subExpression[1].Size + 1U);

    return true;
}


// template<typename Input>
// bool Parse(Input& input, const Variables& variables, Universal& result)
// {
//     char c = input.peek_char(0);
//     if ('{' != c)
//     {
//         return false;
//     }
// 
//     int roundBracketsBalance = 0;
//     int curlyBracketsBalance = 1; // The first we've already found
// 
//     bool foundComma = false;
//     std::size_t commaOffset = 0;
// 
//     std::size_t offset = 1U;
//     while (offset < input.size())
//     {
//         const char curr = input.peek_char(offset);
// 
//         if (curr == '(')
//         {
//             ++roundBracketsBalance;
//         }
//         else if (curr == ')')
//         {
//             --roundBracketsBalance;
//         }
//         else if (curr == '{')
//         {
//             ++curlyBracketsBalance;
//         }
//         else if (curr == '}')
//         {
//             --curlyBracketsBalance;
// 
//             if (curlyBracketsBalance == 0)
//             {
//                 break;
//             }
//         }
//         else if (curr == ',' && curlyBracketsBalance == 1 && roundBracketsBalance == 0)
//         {
//             if (!foundComma)
//             {
//                 foundComma = true;
//                 commaOffset = offset;
//             }
//             else
//             {
//                 throw parse_error("Unexpected comma", input);
//             }
//         }
// 
//         ++offset;
//     }
// 
//     if (curlyBracketsBalance != 0 || !foundComma)
//     {
//         return false;
//     }
// 
//     const char* firstExprBegin = input.current() + 1;
//     const std::size_t firstExprSize = commaOffset - 1U;
// 
//     memory_input<> firstExprInput(firstExprBegin, firstExprSize, "first expression");
// 
//     Universal firstValue = Calculate(firstExprBegin, firstExprSize, "First value", variables);
//     if (firstValue.Type != Universal::Types::INTEGER)
//     {
//         throw parse_error("Invalid expression", input);
//     }
// 
//     const char* secondExprBegin = input.current() + commaOffset + 1;
//     const std::size_t secondExprSize = offset - commaOffset - 1;
// 
//     memory_input<> secondExprInput(secondExprBegin, secondExprSize, "second expression");
// 
//     Universal secondValue = Calculate(secondExprBegin, secondExprSize, "Second value", variables);
//     if (secondValue.Type != Universal::Types::INTEGER)
//     {
//         throw parse_error("Invalid expression", input);
//     }
// 
//     result = Universal(firstValue.Integer, secondValue.Integer);
// 
//     input.bump(offset + 1U);
// 
//     return true;
// }
}
}

