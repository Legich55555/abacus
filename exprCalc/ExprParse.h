#pragma once

#include "ExprCalc.h"

#include "Common.h"
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
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;

    namespace Expr
    {
        template<typename Input>
        bool Parse(Input& input, IsTerminating isTerminating, unsigned threads, const State& variables, Universal& result);
        
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
            static bool match(Input& input, IsTerminating, unsigned, BinaryStacks& stacks, const State&)
            {
                struct BinaryOperatorDef
                {
                    int Priority;
                    BinaryOperator::FuncType Func;
                };

                static const std::map<char, BinaryOperatorDef> BIN_OPERATORS =
                {
                    { '+', { 10, Add } },
                    { '-', { 10, Sub } },
                    { '/', { 20, Div } },
                    { '*', { 30, Mul } },
                    { '^', { 40, Pow } }
                };

                const auto opDefIt = BIN_OPERATORS.find(input.peek_char(0));
                if (opDefIt != BIN_OPERATORS.cend())
                {
                    const BinaryOperatorDef& opDef = opDefIt->second;

                    stacks.PushOperator(BinaryOperator {opDef.Priority, opDef.Func, input.position() } );

                    input.bump(1U);
                    
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
            static bool match(Input& input, IsTerminating isTerminating, unsigned threads, BinaryStacks& stacks, const State& variables)
            {
                Universal result;
                if (Map::Parse(input, isTerminating, threads, variables, result))
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
            static bool match(Input& input, IsTerminating isTerminating, unsigned threads, BinaryStacks& stacks, const State& variables)
            {
                Universal result;
                if (Reduce::Parse(input, isTerminating, threads, variables, result))
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
            static bool match(Input& input, IsTerminating isTerminating, unsigned threads, BinaryStacks& stacks, const State& variables)
            {
                Universal result;
                if (Sequence::Parse(input, isTerminating, threads, variables, result))
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
            static void apply(const Input& input, IsTerminating, unsigned, BinaryStacks& stacks, const State&)
            {
                std::string strVal = input.string();

                int val;

                try
                {
                    val = std::stoi(strVal);
                }
                catch (const std::exception& ex)
                {
                    throw parse_error(ex.what(), input);
                }

                stacks.PushUniversal(Universal(val));
            }
        };
        
        template<>
        struct Action<Real>
        {
            template< typename Input >
            static void apply(const Input& input, IsTerminating, unsigned, BinaryStacks& stacks, const State&)
            {
                std::string strVal = input.string();

                double val;

                try
                {
                    val = std::stod(strVal);
                }
                catch (const std::exception& ex)
                {
                    throw parse_error(ex.what(), input);
                }

                stacks.PushUniversal(Universal(val));
            }
        };
        
        template<>
        struct Action<Variable>
        {
            template< typename Input >
            static void apply(const Input& input, IsTerminating, unsigned, BinaryStacks& stacks, const State& variables)
            {
                std::string strVal = input.string();

                const auto it = variables.find(strVal);
                if (it != variables.cend())
                {
                    stacks.PushUniversal(it->second);
                    return;
                }
                
                throw parse_error(Print("Undefined variable: %s", strVal.c_str()), input);
            }
        };
        
        template<>
        struct Action< one<'('> >
        {
            static void apply0(IsTerminating, unsigned, BinaryStacks& stacks, const State&)
            {
                stacks.Open();
            }
        };
        
        template<>
        struct Action< one<')'> >
        {
            static void apply0(IsTerminating, unsigned, BinaryStacks& stacks, const State&)
            {
                stacks.Close();
            }
        };
        
        template<typename Input>
        bool Parse(Input& input, IsTerminating isTerminating, unsigned threads, const State& variables, Universal& result)
        {
            BinaryStacks stacks;
            
            if (parse<Expression, Action>(input, isTerminating, threads, stacks, variables))
            {
                result = stacks.Calculate();
                
                return true;
            }
            
            return false;
        }
    }
}
