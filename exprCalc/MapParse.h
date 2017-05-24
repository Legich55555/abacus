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
        
//         template<typename Input, typename ST>
//         Universal CalculateSequence(Input& input, const Universal& inputSequence, const std::string& paramName)
//         {
//             std::vector<ST> outputSequence;
//             outputSequence.reserve(inputSequence);
//             
//             return Universal();
//         }

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
            if (firstValue.Type != Universal::Types::INT_SEQUENCE ||
                firstValue.IntSequence.empty())
            {
                throw parse_error("First map parameter is not valid.", input);
            }
            
            input.bump(parsed);
            
            std::string lambdaParameter;
            if (parse<LambdaBegin, IdentifierAction>(input, lambdaParameter) == false)
            {
                throw parse_error("Invalid lambda syntax.", input);
            }
            
            // Calculate the first item of sequence
            size_t lambdaExprSize;
            auto seqIt = firstValue.IntSequence.cbegin();
            Variables lambdaParams = { {lambdaParameter, Universal(*seqIt)} };
            Universal callResult = Calculate(input.current(), input.size(), "Map lambda", lambdaExprSize, lambdaParams);
            if (!callResult.IsNumber())
            {                    
                throw parse_error("Runtime error in map() lambda. Result is expected to be a number.", input);
            }  
            
            if (Universal::Types::INTEGER == callResult.Type)
            {
                std::vector<int> intResult;
                intResult.reserve(firstValue.IntSequence.size());
                intResult.push_back(callResult.Integer);

                while (++seqIt != firstValue.IntSequence.cend())
                {
                    lambdaParams = { {lambdaParameter, Universal(*seqIt)} };
                    callResult = Calculate(input.current(), input.size(), "Map lambda", lambdaExprSize, lambdaParams);
                    if (Universal::Types::INTEGER != callResult.Type)
                    {                    
                        throw parse_error("Runtime error in map() lambda. Result is expected to be an integer number.", input);
                    }
                    
                    intResult.push_back(callResult.Integer);
                }
            
                result = Universal(intResult);
            }
            else
            {
                std::vector<double> doubleResult;
                doubleResult.reserve(firstValue.IntSequence.size());
                doubleResult.push_back(callResult.Real);

                while (++seqIt != firstValue.IntSequence.cend())
                {
                    lambdaParams = { {lambdaParameter, Universal(*seqIt)} };
                    callResult = Calculate(input.current(), input.size(), "Map lambda", lambdaExprSize, lambdaParams);
                    if (Universal::Types::REAL != callResult.Type)
                    {                    
                        throw parse_error("Runtime error in map() lambda. Result is expected to be an doubleing point number.", input);
                    }
                    
                    doubleResult.push_back(callResult.Real);
                }
            
                result = Universal(doubleResult);
            }
            
            input.bump(lambdaExprSize);
            
            if (parse< pad< one<')'>, space> >(input) == false)
            {
                throw parse_error("Invalid map() syntax - no closing round brace.", input);
            }
            
            return true;
        }
    }    
}
