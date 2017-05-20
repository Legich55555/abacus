#include "ExprCalc.h"
#include "Universal.h"
#include "BinaryStack.h"
#include "MapParse.h"
#include "ReduceParse.h"

#include <map>
#include <vector>
#include <functional>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp> // Include the analyze function that checks a grammar for possible infinite cycles.

/*
 * Грамматика:
 * 
 * expr ::= expr op expr | (expr) | identifier | { expr, expr } | number |
 * map(expr, identifier -> expr) | reduce(expr, expr, identifier identifier -> expr)
 * op ::= + | - | * | / | ^
 * stmt ::= var identifier = expr | out expr | print "string"
 * program ::= stmt | program stmt
 * 
 * Пояснения:
 * number - произвольное целое или вещественное число 
 * приоритеты операторов такие (в возрастающем порядке): + и -, * и /, ^
 * {expr1, expr2}, где expr1 и expr2 - выражения с целым результатом - последовательность чисел  { expr1, expr1 + 1, expr + 2 .. expr2 } включительно. Если результат вычисления expr1 или expr2 не целый или expr1 > expr2, результат не определен.
 * map - оператор над элементами последовательности, применяет отображение к элементам последовательности и получает другую последовательность. Последовательность может из целой стать вещественной. Лямбда у map имеет один параметр - элемент последовательности.
 * reduce - свертка последовательности. Первый аргумент - последовательность, второй - нейтральный элемент, третий - операция. Свертка применяет операцию (лямбду) ко всем элементам последовательности. Например, “reduce({5, 7}, 1, x y -> x * y)” должен вычислять 1 * 5 * 6 * 7. Можно полагаться на то, что операция в reduce будет ассоциативна.
 * области видимости переменных - от ее объявления (var) до конца файла. Переменные у лямбд в map / reduce - имеют областью видимости соответствующую лямбду. У лямбд отсутствует замыкание, к глобальным переменным обращаться нельзя
 * out, print - операторы вывода. "string" - произвольная строковая константа, не содержащая кавычек, без экранирования
 * 
 * Пример:
 *   var n = 500
 *   var sequence = map({0, n}, i -> (-1)^i / (2 * i + 1))
 *   var pi = 4 * reduce(sequence, 0, x y -> x + y)
 *   print "pi = "
 *   out pi
 */

// TODO: add types for parse exceptions and execution execeptions.
// TODO: report errors with description.
// TODO: implement map() and reduce().
// TODO: restructure code.
// TODO: optimize Universal (move semantic and minimize its size).
// TODO: implement statements.

namespace ExprCalc
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;
    
    struct Real : seq< opt< one<'+', '-'> >, star<digit>, one<'.'>, plus<digit> > { };
    
    struct Integer : seq< opt< one<'+', '-'> >, plus<digit>, not_at< one<'.'> > > { };
    
    struct Variable : identifier { };
    
    struct Brackets;
    struct MapCall;
    struct ReduceCall;
    struct IntSeq;
    struct Expression;
    
    struct Atomic : sor< MapCall, Integer, Real, Variable, Brackets, ReduceCall, IntSeq > { };
    
    struct BinaryOp
    {
        using analyze_t = analysis::generic< analysis::rule_type::ANY >;
        
        template< 
            apply_mode,
            rewind_mode,
            template< typename... > class Action,
            template< typename... > class Control,
            typename Input >
        static bool match(Input& in, BinaryStacks& stacks, const Variables&)
        {
            const auto opIt = BIN_OPERATORS.find(in.peek_char(0));
            if (opIt != BIN_OPERATORS.cend())
            {
                stacks.PushOperator(opIt->second);
                in.bump(1);
                
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
        static bool match(Input& in, BinaryStacks& stacks, const Variables& variables)
        {
            Universal result;
            if (Map::Parse(in, variables, result))
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
        static bool match(Input& in, BinaryStacks& stacks, const Variables& variables)
        {
            Universal result;
            if (Reduce::Parse(in, variables, result))
            {
                stacks.PushUniversal(result);
                return true;
            }
            
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
        static void apply(const Input& in, BinaryStacks&, const Variables&)
        {
            int a = 5;
            
            auto b = in.begin();
            auto e = in.end();
            
            std::string s = in.string();
            std::string s1 = in.string();
            //s.push( std::stol( in.string() ) );
        }
    };
    
    template<>
    struct Action<Integer>
    {
        template< typename Input >
        static void apply(const Input& in, BinaryStacks& stacks, const Variables&)
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
        static void apply(const Input& in, BinaryStacks& stacks, const Variables&)
        {
            std::string strVal = in.string();
            float val = std::stof(strVal);
            stacks.PushUniversal(Universal(val));
        }
    };
    
    template<>
    struct Action<Variable>
    {
        template< typename Input >
        static void apply(const Input& in, BinaryStacks& stacks, const Variables& variables)
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
        static void apply0(BinaryStacks& stacks, const Variables&)
        {
            stacks.Open();
        }
    };
    
    template<>
    struct Action< one<')'> >
    {
        static void apply0(BinaryStacks& stacks, const Variables&)
        {
            stacks.Close();
        }
    };
    
    Universal Calculate(const std::string& expression, const Variables& variables)
    {
        analyze<Expression>();
        
        BinaryStacks stacks;
        memory_input<> input(expression, "entryExpr");
        
        try
        {
            bool parseResult = parse<Expression, Action>(input, stacks, variables);
            
            if (parseResult)
            {
                Universal result = stacks.Calculate();
                
                return result;
            }
        }
        catch (const parse_error& parseError)
        {
            std::cout << parseError.what() << std::endl;
            
            return Universal();
        }
        
        // Return invalid value
        return Universal();
    }
}
