# pragma once

#include "BracesParse.h"
#include "ExprCalc.h"
#include "Universal.h"
#include <tao/pegtl.hpp>

namespace ExprCalc
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;

    namespace Map
    {
        struct MapBegin : seq< string<'m', 'a', 'p'>, star<space>, one<'('> > { };
        
        struct LambdaBegin : seq< pad< one<','>, space>, identifier, pad< string<'-','>' >, space> > { };
        
        template<typename Rule>
        struct IdentifierAction : nothing<Rule> { };
            
        template<>
        struct IdentifierAction<identifier>
        {
            template< typename Input >
            static void apply(const Input& in, std::string& lambdaParameter)
            {
                lambdaParameter = in.string();
            }
        };

        template< typename Input >
        bool Parse(Input& input, const Variables& variables, Universal& result)
        {
            if (parse<MapBegin>(input) == false)
            {
                return false;
            }

            size_t parsed;
            Universal firstValue = Calculate(
                input.current(),
                input.size(),
                "Map parameter",
                parsed,
                variables);
            if (firstValue.Type != Universal::Types::INT_SEQUENCE)
            {
                throw parse_error("First map parameter is not valid.", input);
            }
            
            input.bump(parsed);
            
            std::string lambdaParameter;
            if (parse<LambdaBegin, IdentifierAction>(input, lambdaParameter) == false)
            {
                throw parse_error("Invalid lambda syntax.", input);
            }
            
            std::vector<float> mapResult;
            mapResult.reserve(firstValue.IntSequence.size());
            
            size_t lambdaExprSize;
            for (int i : firstValue.IntSequence)
            {
                Variables lambdaParams = { {lambdaParameter, Universal(i)} };
                Universal callResult = Calculate(input.current(), input.size(), "Map lambda", lambdaExprSize, lambdaParams);
                if (!callResult.IsNumber())
                {                    
                    throw parse_error("Runtime error in lambda", input);
                }                
                
                mapResult.push_back(callResult.Real);
            }
            
            input.bump(lambdaExprSize);
            
            if (parse< pad< one<')'>, space> >(input) == false)
            {
                throw parse_error("Invalid map() syntax - no closing round brace.", input);
            }
            
            result = Universal(mapResult);
            
            return true;
        }
    }    
}
