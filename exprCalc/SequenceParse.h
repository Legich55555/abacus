#pragma once

#include "Universal.h"

#include <tao/pegtl.hpp>

namespace Abacus
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;
    
    namespace Expr
    {
        template< typename Input >
        bool Parse(Input& input, const State& variables, Universal& result);
    }

    namespace Sequence
    {
        template<typename Input>
        bool Parse(Input& input, const State& variables, Universal& result)
        {
            if (!parse< one<'{'> >(input))
            {
                return false;
            }

            Universal firstValue;
            {
                if (!Expr::Parse(input, variables, firstValue))
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
                if (!Expr::Parse(input, variables, secondValue))
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
            
            result = Universal(firstValue.Integer, secondValue.Integer);

            return true;
        }
    }
}

