#pragma once

#include "Universal.h"

#include <tao/pegtl.hpp>

namespace Abacus
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;
    
    namespace Expr
    {
        // Forward declaration for Expr::Parse()
        template< typename Input >
        bool Parse(Input& input, IsTerminating isTerminating, unsigned threads, const State& variables, Universal& result);
    }

    namespace Sequence
    {
        template<typename Input>
        bool Parse(Input& input, IsTerminating isTerminating, unsigned threads, const State& variables, Universal& result)
        {
            ExpectChar<'{'>(input);

            Universal firstValue;
            if (!Expr::Parse(input, isTerminating, threads, variables, firstValue))
            {
                throw parse_error("Failed to parse first sequence parameter.", input);
            }

            if (firstValue.Type != Universal::Types::INTEGER)
            {
                throw parse_error(Print("First sequence parameter has wrong type. Value: %s",
                                        firstValue.ToString().c_str()),
                                  input);
            }

            ExpectComma(input);
            
            Universal secondValue;
            {
                if (!Expr::Parse(input, isTerminating, threads, variables, secondValue))
                {
                    throw parse_error("Failed to parse the second parameter in sequence.", input);
                }

                if (secondValue.Type != Universal::Types::INTEGER)
                {
                    throw parse_error("Second sequence parameter is not valid.", input);
                }
            }

            ExpectChar<'}'>(input);

            const int step = secondValue.Integer > firstValue.Integer ? 1 : -1;
            const int size = secondValue.Integer > firstValue.Integer ?
                        secondValue.Integer - firstValue.Integer + 1 : firstValue.Integer - secondValue.Integer + 1;

            std::vector<int> sequence;
            sequence.reserve(static_cast<size_t>(size));

            for (int i = firstValue.Integer; i != secondValue.Integer; i += step)
            {
                sequence.push_back(i);
            }

            sequence.push_back(secondValue.Integer);

            result = Universal(std::move(sequence));

            return true;
        }
    }
}

