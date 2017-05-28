#pragma once

#include "Abacus.h"
#include "Universal.h"

#include <map>
#include <string>
#include <vector>

#include <tao/pegtl.hpp>

namespace Abacus
{
    namespace Stmt
    {
        using namespace tao::TAOCPP_PEGTL_NAMESPACE;
        
        template<typename Rule>
        struct IdentifierAction : nothing<Rule> { };
            
        template<>
        struct IdentifierAction<identifier>
        {
            template< typename Input >
            static void apply(const Input& in, std::string& variableName)
            {
                variableName = in.string();
            }
        };
            
        struct AssignmentBegin : seq< 
            star<space>, 
            string<'v', 'a', 'r'>,
            star<space>,
            identifier,
            star<space>,
            one<'='>,
            star<space> > { };
                
        struct Assignment
        {
            using analyze_t = analysis::generic< analysis::rule_type::ANY >;
            
            template< 
                apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input >
            static bool match(Input& input, ExecResult& programResult,  const State& variables)
            {
                std::string variableName;
                if (!parse<AssignmentBegin, IdentifierAction>(input, variableName))
                {
                    return false;
                }

                Universal variableValue;
                if (!Expr::Parse(input, variables, variableValue))
                {
                    throw parse_error("Invalid expression.", input);
                }
                
                programResult.Variables[variableName] = variableValue;
                
                return true;
            }
        };
            
        struct PrintExprBegin : seq< 
            star<space>, 
            string<'o', 'u', 't'>,
            star<space> > { };
        
        struct PrintExpr
        {
            using analyze_t = analysis::generic< analysis::rule_type::ANY >;
            
            template< 
                apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input >
            static bool match(Input& input, ExecResult& programResult,  const State& variables)
            {
                if (!parse<PrintExprBegin>(input))
                {
                    return false;
                }

                Universal expressionValue;
                if (!Expr::Parse(input, variables, expressionValue))
                {
                    throw parse_error("Invalid expression.", input);
                }
                
                programResult.Output.push_back(expressionValue.ToString());
                
                return true;
            }
        };
        
        struct PrintTextBegin : seq< 
            star<space>, 
            string<'p', 'r', 'i', 'n', 't'>,
            star<space>, 
            one<'"'>> { };
            
        struct Text : seq< star<not_one<'"'>>, at<one<'"'>> > { };
        
        template<typename Rule>
        struct TextAction : nothing<Rule> { };
            
        template<>
        struct TextAction<Text>
        {
            template< typename Input >
            static void apply(const Input& in, std::string& textValue)
            {
                textValue = in.string();
            }
        };
            
        struct PrintText
        {
            using analyze_t = analysis::generic< analysis::rule_type::ANY >;
            
            template< 
                apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input >
            static bool match(Input& input, ExecResult& programResult,  const State&)
            {
                if (!parse<PrintTextBegin>(input))
                {
                    return false;
                }

                std::string text;
                if (!parse<Text, TextAction>(input, text))
                {
                    throw parse_error("Invalid text value.", input);
                }

                programResult.Output.push_back(text);
                
                return true;
            }
        };
            
        struct Statement : sor<Assignment, PrintExpr, PrintText>  { }; 
        
        template<typename Input>
        bool Parse(Input& input, ExecResult& execResult, const State& variables)
        {
            bool result = parse<Statement>(input, execResult, variables);
            return result;
        }
    }   
}
