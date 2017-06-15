#pragma once

#include "Common.h"
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

    namespace Reduce
    {
        template< typename Input >
        bool Parse(Input& input, IsTerminating isTerminating, unsigned threads, const State& variables, Universal& result)
        {
            struct ReduceBegin : seq<string< 'r', 'e', 'd', 'u', 'c', 'e' >, star<space>, one<'('> > { };

            if (parse<ReduceBegin>(input) == false)
            {
                return false;
            }

            Universal firstValue;
            if (!Expr::Parse(input, isTerminating, threads, variables, firstValue))
            {
                throw parse_error("First reduce() parameter is not valid.", input);
            }
            if (firstValue.Type != Universal::Types::INT_SEQUENCE &&
                firstValue.Type != Universal::Types::REAL_SEQUENCE)
            {
                throw parse_error("First reduce() parameter is not valid.", input);
            }
            
            ExpectComma(input);

            Universal secondValue;
            if (!Expr::Parse(input, isTerminating, threads, variables, secondValue))
            {
                throw parse_error("Failed to parse the second reduce() parameter.", input);
            }
            if (!secondValue.IsNumber())
            {
                throw parse_error(Print("Second reduce() parameter is must be a number but actual value is %s.",
                                        secondValue.ToString().c_str()),
                                  input);
            }
            
            ExpectComma(input);
            std::string firstParam = ExpectIdentifier(input);
            std::string secondParam = ExpectIdentifier(input);
            ExpectArrow(input);
            
            // Backup position for concurrent/multiple run.
            const char* inputCurr = input.current();
            size_t inputSize = input.size();
            
            Universal intermediateValue = secondValue;
            
            if (firstValue.Type == Universal::Types::INT_SEQUENCE)
            {
                for (size_t idx = 0; idx < firstValue.IntSequence.size(); ++idx)
                {
                    if (isTerminating())
                    {
                        return false;
                    }

                    memory_input<> lambdaInput(inputCurr, inputSize, "reduce() lambda");
                    
                    auto& lambdaInputRef = idx != 0 ? lambdaInput : input;
                    
                    State lambdaParams = 
                    { 
                        {firstParam, intermediateValue},
                        {secondParam, Universal(firstValue.IntSequence[idx])}
                    };
                    
                    Universal lambdaResult;
                    if (!Expr::Parse(lambdaInputRef, isTerminating, threads, lambdaParams, lambdaResult))
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
                        {firstParam, intermediateValue},
                        {secondParam, Universal(firstValue.RealSequence[idx])}
                    };
                    
                    Universal lambdaResult;
                    if (!Expr::Parse(lambdaInputRef, isTerminating, threads, lambdaParams, lambdaResult))
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
            
            ExpectClosingBracket(input);
            
            result = intermediateValue;

            return true;            
        }
    }
}
