# pragma once

#include <tao/pegtl.hpp>
#include "ExprCalc.h"
#include "Universal.h"

namespace ExprCalc
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;
    
    template<typename Input>
    size_t findOpeningBracket(const std::string& funcName, const Input& in)
    {
        
        
        return 0;
    }
    
    template<typename Input>
    size_t findClosingBracket(size_t openingBracketPos, const Input& in)
    {
        return 0;
    }
    
    
    
    namespace Map
    {
        struct Stack
        {
            void PushArgument(Universal&& u)
            {
                if (!u.IsValid())
                {
                    throw 1;
                }
                
                if (m_argument.IsValid())
                {
                    throw 1;
                }
                
                m_argument = u;
            }
            
            void PushIdentifier(std::string&& id)
            {
                if (id.empty())
                {
                    throw 1;
                }
                
                if (m_identifier.empty())
                {
                    m_identifier = id;
                }
                else
                {
                    throw 1;
                }
            }
            
            Universal Calculate()

            {
                return Universal(0, 1);
            }
            
        private:
            Universal m_argument;
            std::string m_identifier;
        };
    
//     struct MapLambda : seq< 
//         star<space>,
//         identifier,
//         star<space>,
//         string< '-', '>'>,
//         star<space>,
//         Expression
//         > { };
    
        
        template<typename Rule>
        struct Action : nothing<Rule> { };
    
        template<>
        struct Action<seq<string<'m', 'a', 'p'>, star<space>, one<'('>>>
        {
            template< typename Input >
            static void apply(const Input& in)
            {
                auto b = in.begin();
                auto e = in.end();
                
                std::string s = in.string();

                std::string s1 = in.string();
                //s.push( std::stol( in.string() ) );
            }
        };
        
        
        struct MapBegin : seq<string<'m', 'a', 'p'>, star<space>, one<'('>> { };
            
        template< typename Input >
        bool Parse( Input&& in, const Variables& variables, Universal& result)
        {
            return false;
        }
    }    
}
