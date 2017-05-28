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
        bool Parse(Input& input, const State& variables, Universal& result)
        {
            if (parse<ReduceBegin>(input) == false)
            {
                return false;
            }

            Universal firstValue;
            if (!Expr::Parse(input, variables, firstValue))
            {
                throw parse_error("First reduce() parameter is not valid.", input);
            }
            if (firstValue.Type != Universal::Types::INT_SEQUENCE &&
                firstValue.Type != Universal::Types::REAL_SEQUENCE)
            {
                throw parse_error("First reduce() parameter is not valid.", input);
            }
            
            if (parse< seq< pad<one<','>, space> > >(input) == false)
            {
                throw parse_error("Failed to parse reduce()", input);
            }

            Universal secondValue;
            if (!Expr::Parse(input, variables, secondValue))
            {
                throw parse_error("Second reduce() parameter is not valid.", input);
            }
            if (!secondValue.IsNumber())
            {
                throw parse_error("Second reduce() parameter is not valid.", input);
            }
            
            if (parse< seq< pad<one<','>, space> > >(input) == false)
            {
                throw parse_error("Failed to parse reduce()", input);
            }

            std::vector<std::string> lambdaParameters;
            if (parse<LambdaBegin, IdentifierAction>(input, lambdaParameters) == false)
            {
                throw parse_error("Invalid lambda syntax.", input);
            }
            
            // Backup position for concurrent/multiple run.
            const char* inputCurr = input.current();
            size_t inputSize = input.size();
            
            Universal intermediateValue = secondValue;
            
            if (firstValue.Type == Universal::Types::INT_SEQUENCE)
            {
                for (size_t idx = 0; idx < firstValue.IntSequence.size(); ++idx)
                {
                    memory_input<> lambdaInput(inputCurr, inputSize, "reduce() lambda");
                    
                    auto& lambdaInputRef = idx != 0 ? lambdaInput : input;
                    
                    State lambdaParams = 
                    { 
                        {lambdaParameters[0], intermediateValue},
                        {lambdaParameters[1], Universal(firstValue.IntSequence[idx])}
                    };
                    
                    Universal lambdaResult;
                    if (!Expr::Parse(lambdaInputRef, lambdaParams, lambdaResult))
                    {
                        throw parse_error("Failed to calculate reduce() lambda.", input);
                    }
                    if (!secondValue.IsNumber())
                    {
                        throw parse_error("reduce() lambda returned unexpected result.", input);
                    }
                    
                    intermediateValue = lambdaResult;
                }
            }
            else
            {
                for (size_t idx = 0; idx < firstValue.RealSequence.size(); ++idx)
                {
                    memory_input<> lambdaInput(inputCurr, inputSize, "reduce() lambda");
                    
                    auto& lambdaInputRef = idx != 0 ? lambdaInput : input;
                    
                    State lambdaParams = 
                    { 
                        {lambdaParameters[0], intermediateValue},
                        {lambdaParameters[1], Universal(firstValue.RealSequence[idx])}
                    };
                    
                    Universal lambdaResult;
                    if (!Expr::Parse(lambdaInputRef, lambdaParams, lambdaResult))
                    {
                        throw parse_error("Failed to calculate reduce() lambda.", input);
                    }
                    if (!secondValue.IsNumber())
                    {
                        throw parse_error("reduce() lambda returned unexpected result.", input);
                    }
                    
                    intermediateValue = lambdaResult;
                }

            }
            
            if (parse< pad< one<')'>, space> >(input) == false)
            {
                throw parse_error("Invalid map() syntax - no closing round brace.", input);
            }
            
            result = intermediateValue;

            return true;            
        }
    }
}
