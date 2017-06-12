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
            if (!parse< one<'{'> >(input))
            {
                return false;
            }

            Universal firstValue;
            {
                if (!Expr::Parse(input, isTerminating, threads, variables, firstValue))
                {
                    throw parse_error("First sequence parameter is not valid.", input);
                }

                if (firstValue.Type != Universal::Types::INTEGER)
                {
                    throw parse_error("First sequence parameter is not valid.", input);
                }
            }
            
            if (parse< seq< pad<one<','>, space> > >(input) == false)
            {
                throw parse_error("Failed to parse sequence. There is no comma.", input);
            }
            
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

            if (parse< seq< pad<one<'}'>, space> > >(input) == false)
            {
                throw parse_error("Failed to parse sequence. There is not closing brace.", input);
            }

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

