#pragma once

#include "ExprCalc.h"

#include "MapParse.h"
#include "BinaryStack.h"
#include "ReduceParse.h"
#include "SequenceParse.h"

#include <map>
#include <vector>
#include <functional>
#include <tao/pegtl.hpp>

namespace Abacus
{
    namespace Expr
    {
        using namespace tao::TAOCPP_PEGTL_NAMESPACE;
        
        template<typename Input>
        bool Parse(Input& input, const State& variables, Universal& result);
        
        struct Real : seq<
            opt< one<'+', '-'> >,
            star<digit>,
            one<'.'>,
            plus<digit> > { };
        
        struct Integer : seq<
            opt< one<'+', '-'> >,
            plus<digit>,
            not_at< one<'.'> > > { };
        
        struct Variable : seq<
            identifier, 
            not_at< seq< star<space>, one<'('> > > > { };
        
        struct RoundBraces;
        struct MapCall;
        struct ReduceCall;
        struct IntSeq;
        struct Expression;
        
        struct Atomic : sor< 
            MapCall,
            ReduceCall,
            Integer,
            Real,
            Variable,
            RoundBraces,
            IntSeq >  { };
        
        struct BinaryOp
        {
            using analyze_t = analysis::generic< analysis::rule_type::ANY >;
            
            template< 
                apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input >
            static bool match(Input& input, BinaryStacks& stacks, const State&)
            {
                const auto opIt = BIN_OPERATORS.find(input.peek_char(0));
                if (opIt != BIN_OPERATORS.cend())
                {
                    stacks.PushOperator(opIt->second);
                    input.bump(1);
                    
                    return true;
                }
                
                return false;
            }
        };
        
        struct MapCall
        {
            using analyze_t = analysis::generic< analysis::rule_type::ANY >;
            
            template< 
                apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input>
            static bool match(Input& input, BinaryStacks& stacks, const State& variables)
            {
                Universal result;
                if (Map::Parse(input, variables, result))
                {
                    stacks.PushUniversal(result);
                    return true;
                }
                
                return false;
            }
        };
        
        struct ReduceCall
        {
            using analyze_t = analysis::generic< analysis::rule_type::ANY >;
            
            template< 
                apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input>
            static bool match(Input& input, BinaryStacks& stacks, const State& variables)
            {
                Universal result;
                if (Reduce::Parse(input, variables, result))
                {
                    stacks.PushUniversal(result);
                    return true;
                }
                
                return false;
            }
        };
        
        struct IntSeq 
        {
            using analyze_t = analysis::generic< analysis::rule_type::ANY >;
            
            template< 
                apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input>
            static bool match(Input& input, BinaryStacks& stacks, const State& variables)
            {
                Universal result;
                if (Sequence::Parse(input, variables, result))
                {
                    stacks.PushUniversal(result);
                    return true;
                }
                
                return false;
            }
        };

        struct Expression : seq<
                star<space>,
                sor<
                    list_must<Atomic, BinaryOp, space>,
                    Atomic
                >,
                star<space>
                > { };
        
        struct RoundBraces : seq< one<'('>, pad< Expression, space>, one<')'> > { };
        
        template<typename Rule>
        struct Action : nothing<Rule> { };
        
        template<>
        struct Action<Integer>
        {
            template< typename Input >
            static void apply(const Input& in, BinaryStacks& stacks, const State&)
            {
                std::string strVal = in.string();
                int val = std::stoi(strVal);
                stacks.PushUniversal(Universal(val));
            }
        };
        
        template<>
        struct Action<Real>
        {
            template< typename Input >
            static void apply(const Input& in, BinaryStacks& stacks, const State&)
            {
                std::string strVal = in.string();
                double val = std::stof(strVal);
                stacks.PushUniversal(Universal(val));
            }
        };
        
        template<>
        struct Action<Variable>
        {
            template< typename Input >
            static void apply(const Input& in, BinaryStacks& stacks, const State& variables)
            {
                std::string strVal = in.string();
                const auto it = variables.find(strVal);
                if (it != variables.cend())
                {
                    stacks.PushUniversal(it->second);
                    return;
                }
                
                throw parse_error("Undefined variable", in);
            }
        };
        
        template<>
        struct Action< one<'('> >
        {
            static void apply0(BinaryStacks& stacks, const State&)
            {
                stacks.Open();
            }
        };
        
        template<>
        struct Action< one<')'> >
        {
            static void apply0(BinaryStacks& stacks, const State&)
            {
                stacks.Close();
            }
        };
        
        template<typename Input>
        bool Parse(Input& input, const State& variables, Universal& result)
        {
            BinaryStacks stacks;
            
            if (parse<Expression, Action>(input, stacks, variables))
            {
                result = stacks.Calculate();
                
                return true;
            }
            
            return false;
        }
    }
}
