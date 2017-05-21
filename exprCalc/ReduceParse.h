# pragma once

#include <tao/pegtl.hpp>
#include "ExprCalc.h"
#include "Universal.h"

namespace ExprCalc
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;
    
    namespace Reduce
    {
        struct Stack
        {
            void PushArgument(Universal&& u)
            {
            }
            
            void PushIdentifier(std::string&& id)
            {
            }
            
            Universal Calculate()
            {
                return Universal(0);
            }
            
        private:
            std::vector<Universal> m_arguments;
            std::vector<std::string> m_identifiers;
        };
    
        template< typename Input >
        bool Parse( Input& in, const Variables& variables, Universal& result)
        {
            return false;
        }
        
        //         struct Reduce : seq< 
        //             string< 'r', 'e', 'd', 'u', 'c', 'e' >, 
        //             pad< one<'('>, space >, 
        //             Expression, 
        //             pad< one<','>, space >, 
        //             Expression, 
        //             pad< one<')'>, space > 
        //             > { };
        
//     struct ReduceLambda : seq< 
//         star<space>,
//         identifier,
//         star<space>,
//         identifier,
//         star<space>,
//         string<'-', '>'>,
//         star<space>,
//         Expression
//         > { };

    }
}
