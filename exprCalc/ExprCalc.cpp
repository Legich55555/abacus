#include "ExprCalc.h"

#include <cmath>
#include <map>
#include <vector>
#include <functional>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp> // Include the analyze function that checks a grammar for possible infinite cycles.

/*
Грамматика:

expr ::= expr op expr | (expr) | identifier | { expr, expr } | number |
map(expr, identifier -> expr) | reduce(expr, expr, identifier identifier -> expr)
op ::= + | - | * | / | ^
stmt ::= var identifier = expr | out expr | print "string"
program ::= stmt | program stmt

Пояснения:
number - произвольное целое или вещественное число 
приоритеты операторов такие (в возрастающем порядке): + и -, * и /, ^
{expr1, expr2}, где expr1 и expr2 - выражения с целым результатом - последовательность чисел  { expr1, expr1 + 1, expr + 2 .. expr2 } включительно. Если результат вычисления expr1 или expr2 не целый или expr1 > expr2, результат не определен.
map - оператор над элементами последовательности, применяет отображение к элементам последовательности и получает другую последовательность. Последовательность может из целой стать вещественной. Лямбда у map имеет один параметр - элемент последовательности.
reduce - свертка последовательности. Первый аргумент - последовательность, второй - нейтральный элемент, третий - операция. Свертка применяет операцию (лямбду) ко всем элементам последовательности. Например, “reduce({5, 7}, 1, x y -> x * y)” должен вычислять 1 * 5 * 6 * 7. Можно полагаться на то, что операция в reduce будет ассоциативна.
области видимости переменных - от ее объявления (var) до конца файла. Переменные у лямбд в map / reduce - имеют областью видимости соответствующую лямбду. У лямбд отсутствует замыкание, к глобальным переменным обращаться нельзя
out, print - операторы вывода. "string" - произвольная строковая константа, не содержащая кавычек, без экранирования
*/

namespace ExprCalc
{
    struct Universal
    {
        enum class Types 
        {
            INVALID,
            INTEGER,
            REAL,
            INT_SEQUENCE,
            REAL_SEQUENCE,
            VARIABLE,
            FUTURE
        };
        
        Types Type;
        
        union
        {
            int Integer;
            float Real;
            std::string Variable;
            std::pair<int, int> IntSequence;
            std::vector<float> RealSequence;
        };
        
        Universal() : Type(Types::INVALID) { }
        
        Universal(const Universal& other) : Type(other.Type) 
        {
            switch(Type)
            {
                case Types::INTEGER: 
                    Integer = other.Integer; 
                    break;
                case Types::REAL:
                    Real = other.Real; 
                    break;
                case Types::VARIABLE:
                    Variable = other.Variable; 
                    break;
                case Types::INT_SEQUENCE:
                    IntSequence = other.IntSequence; 
                    break;
                case Types::REAL_SEQUENCE:
                    RealSequence = other.RealSequence; 
                    break;
                default:
                    Type = Types::INVALID;
            }
        }
        
        explicit Universal(int v) : Type(Types::INTEGER), Integer(v) {}
        explicit Universal(float v) : Type(Types::REAL), Real(v) {}
        explicit Universal(int first, int last) : Type(Types::INT_SEQUENCE), IntSequence(first, last) {}
        explicit Universal(std::vector<float>& realSequence) : Type(Types::REAL_SEQUENCE), RealSequence(realSequence) {}
        explicit Universal(const std::string& varName) : Type(Types::VARIABLE), Variable(varName) {}   
        
        ~Universal()
        {
            using std::string;
            using std::pair;
            using std::vector;
            
            switch(Type)
            {
                case Types::VARIABLE:
                    Variable.~string(); 
                    break;
                case Types::INT_SEQUENCE:
                    IntSequence.~pair<int, int>(); 
                    break;
                case Types::REAL_SEQUENCE:
                    RealSequence.~vector<float>(); 
                    break;
                default:
                    break;
            }
            
            Type = Types::INVALID;
        }
    };
    
    Universal Mul(const Universal& l, const Universal& r)
    {
        if (l.Type == Universal::Types::INTEGER)
        {
            if (r.Type == Universal::Types::INTEGER)
            {
                return Universal(l.Integer * r.Integer);
            }
            else if (r.Type == Universal::Types::REAL)
            {
                return Universal(l.Integer * r.Real);
            }
        }
        else if (l.Type == Universal::Types::REAL)
        {
            if (r.Type == Universal::Types::INTEGER)
            {
                return Universal(l.Integer * r.Integer);
            }
            else if (r.Type == Universal::Types::REAL)
            {
                return Universal(l.Integer * r.Real);
            }
        }

        throw 1;
    }

    Universal Add(const Universal& l, const Universal& r)
    {
        if (l.Type == Universal::Types::INTEGER)
        {
            if (r.Type == Universal::Types::INTEGER)
            {
                return Universal(l.Integer + r.Integer);
            }
            else if (r.Type == Universal::Types::REAL)
            {
                return Universal(l.Integer + r.Real);
            }
        }
        else if (l.Type == Universal::Types::REAL)
        {
            if (r.Type == Universal::Types::INTEGER)
            {
                return Universal(l.Integer + r.Integer);
            }
            else if (r.Type == Universal::Types::REAL)
            {
                return Universal(l.Integer + r.Real);
            }
        }

        throw 1;
    }

    Universal Sub(const Universal& l, const Universal& r)
    {
        if (l.Type == Universal::Types::INTEGER)
        {
            if (r.Type == Universal::Types::INTEGER)
            {
                return Universal(l.Integer - r.Integer);
            }
            else if (r.Type == Universal::Types::REAL)
            {
                return Universal(l.Integer - r.Real);
            }
        }
        else if (l.Type == Universal::Types::REAL)
        {
            if (r.Type == Universal::Types::INTEGER)
            {
                return Universal(l.Integer - r.Integer);
            }
            else if (r.Type == Universal::Types::REAL)
            {
                return Universal(l.Integer - r.Real);
            }
        }

        throw 1;
    }

    Universal Div(const Universal& l, const Universal& r)
    {
        if (l.Type == Universal::Types::INTEGER)
        {
            if (r.Type == Universal::Types::INTEGER)
            {
                return Universal(l.Integer / r.Integer);
            }
            else if (r.Type == Universal::Types::REAL)
            {
                return Universal(l.Integer / r.Real);
            }
        }
        else if (l.Type == Universal::Types::REAL)
        {
            if (r.Type == Universal::Types::INTEGER)
            {
                return Universal(l.Integer / r.Integer);
            }
            else if (r.Type == Universal::Types::REAL)
            {
                return Universal(l.Integer / r.Real);
            }
        }

        throw 1;
    }

    Universal Pow(const Universal& l, const Universal& r)
    {
        if (l.Type == Universal::Types::INTEGER)
        {
            if (r.Type == Universal::Types::INTEGER)
            {
                return Universal(static_cast<float>(std::pow(l.Integer, r.Integer)));
            }
            else if (r.Type == Universal::Types::REAL)
            {
                return Universal(static_cast<float>(std::pow(l.Integer, r.Real)));
            }
        }
        else if (l.Type == Universal::Types::REAL)
        {
            if (r.Type == Universal::Types::INTEGER)
            {
                return Universal(static_cast<float>(std::pow(l.Integer, r.Integer)));
            }
            else if (r.Type == Universal::Types::REAL)
            {
                return Universal(static_cast<float>(std::pow(l.Integer, r.Real)));
            }
        }

        throw 1;
    }

    bool IsNumber(const Universal& v)
    {
        return v.Type == Universal::Types::INTEGER || v.Type == Universal::Types::REAL;
    }
    
    struct Stacks
    {
    };
    
    
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;

    struct Integer : seq< opt< one<'+', '-'> >, plus<digit> > { };

    struct Real : seq< opt< one<'+', '-'> >, star<digit>, one<'.'>, plus<digit> > { };

    struct Variable : identifier { };
    
    struct Brackets;
    struct MapCall;
    struct ReduceCall;
    struct IntSeq;

    struct Atomic : sor< Integer, Real, Variable, Brackets, MapCall, ReduceCall, IntSeq > { };
    
    struct BinaryOp
    {
        using analyze_t = analysis::generic< analysis::rule_type::ANY >;

        template< apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input >
        static bool match( Input& in, Stacks& stacks )
        {
            // Look for the longest match of the input against the operators in the operator map.

            return match( in, stacks, std::string() );
        }

    private:
        
        struct BinaryOperators
        {
            struct Operator
            {
                int Priority;
                std::function<Universal(const Universal&, const Universal&)> Func;
            };
            
            BinaryOperators()
            {
                m_Operators = 
                { 
                    { '^', { 10, Pow } },
                    { '*', { 20, Mul } }, 
                    { '/', { 30, Div } },
                    { '+', { 40, Add } },
                    { '-', { 50, Sub } }                
                };
            }

        private:
            std::map<char, Operator> m_Operators;
        };
        
        template< typename Input >
        static bool match( Input& in, const Stacks& stacks, std::string t )
        {
    //          if( in.size( t.size() + 1 ) > t.size() ) {
    //             t += in.peek_char( t.size() );
    //             const auto i = b.ops().lower_bound( t );
    //             if( i != b.ops().end() ) {
    //                if( match( in, b, s, t ) ) {
    //                   return true;
    //                }
    //                if( i->first == t ) {
    //                   // While we are at it, this rule also performs the task of what would
    //                   // usually be an associated action: To push the matched operator onto
    //                   // the operator stack.
    //                   s.push( i->second );
    //                   in.bump( t.size() );
    //                   return true;
    //                }
    //             }
    //          }
            return false;
        }
    };
    
    struct MapCall
    {
        using analyze_t = analysis::generic< analysis::rule_type::ANY >;

        template< apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input >
        static bool match( Input& in, Stacks& stacks )
        {
            int a = 5;
            
            // Look for the longest match of the input against the operators in the operator map.

            return match( in, stacks, std::string() );
        }

    private:
//         struct Map : seq< 
//             string< 'm', 'a', 'p' >, 
//             pad< one<'('>, space >,
//             Expression, 
//             pad< one<','>, space >,
//             Expression, 
//             pad< one<'('>, space > 
//             > { };
        
        struct MapLambda;
        
        template< typename Input >
        static bool match( Input& in, Stacks& stacks, std::string t )
        {
    //          if( in.size( t.size() + 1 ) > t.size() ) {
    //             t += in.peek_char( t.size() );
    //             const auto i = b.ops().lower_bound( t );
    //             if( i != b.ops().end() ) {
    //                if( match( in, b, s, t ) ) {
    //                   return true;
    //                }
    //                if( i->first == t ) {
    //                   // While we are at it, this rule also performs the task of what would
    //                   // usually be an associated action: To push the matched operator onto
    //                   // the operator stack.
    //                   s.push( i->second );
    //                   in.bump( t.size() );
    //                   return true;
    //                }
    //             }
    //          }
            return false;
        }
    };

    struct ReduceCall
    {
        using analyze_t = analysis::generic< analysis::rule_type::ANY >;

        template< apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input >
        static bool match( Input& in, Stacks& stacks )
        {
            // Look for the longest match of the input against the operators in the operator map.

            return match( in, stacks, std::string() );
        }

    private:

//         struct Reduce : seq< 
//             string< 'r', 'e', 'd', 'u', 'c', 'e' >, 
//             pad< one<'('>, space >, 
//             Expression, 
//             pad< one<','>, space >, 
//             Expression, 
//             pad< one<')'>, space > 
//             > { };

        struct ReduceLambda;
    
        template< typename Input >
        static bool match( Input& in, Stacks& stacks, std::string t )
        {
    //          if( in.size( t.size() + 1 ) > t.size() ) {
    //             t += in.peek_char( t.size() );
    //             const auto i = b.ops().lower_bound( t );
    //             if( i != b.ops().end() ) {
    //                if( match( in, b, s, t ) ) {
    //                   return true;
    //                }
    //                if( i->first == t ) {
    //                   // While we are at it, this rule also performs the task of what would
    //                   // usually be an associated action: To push the matched operator onto
    //                   // the operator stack.
    //                   s.push( i->second );
    //                   in.bump( t.size() );
    //                   return true;
    //                }
    //             }
    //          }
            return false;
        }
    };

    struct Expression : list< Atomic, BinaryOp, space > { };
    
    struct Brackets : seq< one<'('>, pad< Expression, space>, one<')'> > { };
    
    struct IntSeq : seq< one<'{'>, pad<Expression, space>, one<','>, pad<Expression, space>, one<'}'> > { };
    
    struct String : seq< one<'"'>, star<alnum>, one<'"'> > { };

    struct Assignment : seq< Variable, pad< one<'='>, space >, Expression > { };
    
    template<typename Rule>
    struct Action : nothing<Rule> { };
    
    template<>
    struct Action<Expression>
    {
       template< typename Input >
       static void apply( const Input& in, Stacks& )
       {
           int a = 5;
          //s.push( std::stol( in.string() ) );
       }
    };

    
    void ParseProgram(const std::string& text)
    {
        analyze<Expression>();
        
        Stacks stacks;
        
        memory_input<> in(text, "std::cin");
        bool result = parse< Expression, Action >(in, stacks);

        Expression expr;
    }
}
