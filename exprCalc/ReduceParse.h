# pragma once

#include <tao/pegtl.hpp>
#include "ExprCalc.h"
#include "Universal.h"

namespace ExprCalc
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;
    
    namespace Reduce
    {
        struct ReduceBegin : seq<string< 'r', 'e', 'd', 'u', 'c', 'e' >, star<space>, one<'('> > { };
        
        struct LambdaBegin : seq< star< space>, identifier, star<space>, identifier, pad< string<'-','>' >, space> > { };
        
        template<typename Rule>
        struct IdentifierAction : nothing<Rule> { };
            
        template<>
        struct IdentifierAction<identifier>
        {
            template< typename Input >
            static void apply(const Input& in, std::vector<std::string>& lambdaParameters)
            {
                lambdaParameters.push_back(in.string());
            }
        };
        
        template< typename Input >
        bool Parse(Input& input, const Variables& variables, Universal& result)
        {
            if (parse<ReduceBegin>(input) == false)
            {
                return false;
            }

            size_t parsed;
            Universal firstValue = Calculate(
                input.current(),
                input.size(),
                "Reduce first parameter",
                parsed,
                variables);
            if (firstValue.Type != Universal::Types::INT_SEQUENCE &&
                firstValue.Type != Universal::Types::REAL_SEQUENCE)
            {
                throw parse_error("First reduce() parameter is not valid.", input);
            }
            input.bump(parsed);
            
            if (parse< seq< pad<one<','>, space> > >(input) == false)
            {
                throw parse_error("Failed to parse reduce()", input);
            }

            Universal secondValue = Calculate(
                input.current(),
                input.size(),
                "Reduce second parameter",
                parsed,
                variables);
            if (secondValue.Type != Universal::Types::INTEGER &&
                secondValue.Type != Universal::Types::REAL)
            {
                throw parse_error("Second reduce() parameter is not valid.", input);
            }
            input.bump(parsed);
            
            if (parse< seq< pad<one<','>, space> > >(input) == false)
            {
                throw parse_error("Failed to parse reduce()", input);
            }

            std::vector<std::string> lambdaParameters;
            if (parse<LambdaBegin, IdentifierAction>(input, lambdaParameters) == false)
            {
                throw parse_error("Invalid lambda syntax.", input);
            }
            
            Universal intermediateValue = secondValue;
            
            size_t lambdaExprSize;
            if (firstValue.Type == Universal::Types::INT_SEQUENCE)
            {
                for (int i : firstValue.IntSequence)
                {
                    Variables lambdaParams = { 
                        {lambdaParameters[0], intermediateValue},
                        {lambdaParameters[1], Universal(i)}
                    };
                    
                    Universal callResult = Calculate(input.current(), input.size(), "Reduce lambda", lambdaExprSize, lambdaParams);
					if (!callResult.IsNumber())
					{
                        throw parse_error("Runtime error in lambda", input);
                    }
                    
                    intermediateValue = callResult;
                }
            }
            else
            {
                for (float f : firstValue.RealSequence)
                {
                    Variables lambdaParams = { 
                        {lambdaParameters[0], intermediateValue},
                        {lambdaParameters[1], Universal(f)}
                    };
                    
                    Universal callResult = Calculate(input.current(), input.size(), "Reduce lambda", lambdaExprSize, lambdaParams);
                    if (!callResult.IsNumber())
                    {                    
                        throw parse_error("Runtime error in lambda", input);
                    }
                    
                    intermediateValue = callResult;
                }
            }
            
            input.bump(lambdaExprSize);
            
            if (parse< pad< one<')'>, space> >(input) == false)
            {
                throw parse_error("Invalid map() syntax - no closing round brace.", input);
            }
            
            result = intermediateValue;

            return true;            
        }
    }
}
