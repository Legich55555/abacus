# pragma once

#include "ExprCalc.h"
#include "Universal.h"

#include <string>
#include <vector>

#include <tao/pegtl.hpp>

namespace Abacus
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;

    struct ParseError
    {
        std::string Message;
        std::vector<position> Positions;
    };

    struct ParseResult
    {
        ResultBrief Brief;
        std::vector<ParseError> Errors;
    };

    struct TerminatedException
    {

    };

    template<typename T>
    const T& GetValue(const Universal& u)
    {
        static_assert(
                    std::is_same<T, int>::value ||
                    std::is_same<T, double>::value ||
                    std::is_same<T, Universal::IntArray>::value ||
                    std::is_same<T, Universal::RealArray>::value,
                    "Invalid data type.");

        if (std::is_same<T, int>::value && Universal::Types::INTEGER == u.Type)
        {
            return u.Integer;
        }
        else if (std::is_same<T, double>::value && Universal::Types::REAL == u.Type)
        {
            return u.Real;
        }
        else if (std::is_same<T, Universal::IntArray>::value && Universal::Types::INT_SEQUENCE == u.Type)
        {
            return u.IntSequence;
        }
        else if (std::is_same<T, Universal::RealArray>::value && Universal::Types::REAL_SEQUENCE == u.Type)
        {
            return u.RealSequence;
        }

        throw std::runtime_error("Invalid call for GetValue().");
    }

    template<typename T>
    T GetNumber(const Universal& u)
    {
        static_assert(
                    std::is_same<T, int>::value ||
                    std::is_same<T, double>::value,
                    "Invalid data type.");

        if (std::is_same<T, int>::value && Universal::Types::INTEGER == u.Type)
        {
            return u.Integer;
        }
        else if (std::is_same<T, double>::value && Universal::Types::REAL == u.Type)
        {
            return u.Real;
        }

        throw std::runtime_error("Invalid call for GetValue().");
    }

}
