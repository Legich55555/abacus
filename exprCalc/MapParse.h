# pragma once

#include "BracesParse.h"
#include "ExprCalc.h"
#include "Universal.h"
#include <tao/pegtl.hpp>

namespace ExprCalc
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;

    namespace Lambda
    {
        struct LambdaBegin : seq< star<space>, list<identifier, space>, string<'-', '>'> > { };
        
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

        template<typename Input>
        std::vector<std::string> ParseLambda(Input& input)
        {
            std::vector<std::string> lambdaParameters;
            if (parse<LambdaBegin, IdentifierAction>(input, lambdaParameters) == false)
            {
                throw parse_error("Invalid lambda syntax.", input);
            }
            
            return lambdaParameters;
        }
    }
    
    namespace Map
    {
        
        struct MapBegin : seq< string<'m', 'a', 'p'>, star<space>, one<'('> > { };
        //struct LambdaBegin : seq< star<space>, identifier, star<space>, string<'-','>' > > { };
        
        template<typename Rule>
        struct LambdaAction : nothing<Rule> { };
        
        template<>
        struct LambdaAction<identifier>
        {
            template< typename Input >
            static void apply(const Input& in, std::string& identifier)
            {
                identifier = in.string();
            }
        };
        
        template< typename Input >
        bool Parse(Input& input, const Variables& variables, Universal& result)
        {
            if (parse<Map::MapBegin>(input, variables) == false)
            {
                return false;
            }

            //std::vector<SubExpr> subExpressions = ParseBraces(input, 0, BraceType::ROUND);
            //if (subExpressions.size() != 2U)
            //{
            //    throw parse_error("Invalid usage of map()", input);
            //}

            size_t bumped;
            Universal firstValue = Calculate(
                input.current(),
                input.size(),
                "Map parameter",
                bumped,
                variables);
            if (firstValue.Type != Universal::Types::INT_SEQUENCE)
            {
                throw parse_error("First map parameter is not valid.", input);
            }
            
            //memory_input<> lambdaInput(input.current() + subExpressions[1].Offset, subExpressions[1].Size, "Map");
            
            // TODO: implement Calculate which will take input by reference.
           // input.bump(subExpressions[1].Offset);
            
            std::vector<std::string> parameters = Lambda::ParseLambda(input);
            if (parameters.size() != 1U)
            {
                throw parse_error("Invalid usage of lambda in map()", input);
            }

            std::vector<int> mapResult;
            mapResult.reserve(firstValue.IntSequence.size());
            
            size_t lambdaSize;
            for (int i : firstValue.IntSequence)
            {
                Variables lambdaParams = { {parameters.front(), Universal(i)} };
                Universal callResult = Calculate(input.current(), input.size(), "Map lambda", lambdaSize, lambdaParams);
                if (callResult.Type != Universal::Types::INTEGER)
                {                    
                    throw parse_error("Runtime error in lambda", input);
                }                
                
                mapResult.push_back(callResult.Integer);
            }
            
            result = Universal(mapResult);
            
            input.bump(lambdaSize + 1U);
            
            return true;
        }
    }    
}
