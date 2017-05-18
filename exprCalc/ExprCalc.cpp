#include "ExprCalc.h"
#include "Universal.h"

#include <map>
#include <vector>

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
    struct BinaryOperator
    {
        int Priority;
        Universal(*Func)(const Universal&, const Universal&);
    };

    const std::map<char, BinaryOperator> BIN_OPERATORS = 
    {
        { '+', { 10, Add } },
        { '-', { 10, Sub } }, 
        { '/', { 20, Div } },
        { '*', { 30, Mul } },
        { '^', { 40, Pow } }
    };

    struct BinaryOperatorsStack
    {
        void PushOperator(const BinaryOperator& op)
        {
            if ((!m_operators.empty()) && (m_operators.back().Priority >=  op.Priority))
            {
                Reduce();
            }

            m_operators.push_back(op);
        }
        
        void PushUniversal(const Universal& u)
        {
            m_values.push_back(u);
        }
        
        Universal Calculate()
        {
            assert(m_values.size() > 0U);
            assert(m_values.size() > m_operators.size());
            assert((m_values.size() - m_operators.size()) == 1U);
            
            while (!m_operators.empty())
            {
                Reduce();
            }
            
            Universal result = m_values.front();
            
            return result;
        }
        
    private:
        
        void Reduce()
        {
            assert(m_values.size() > 1U);
            assert(m_values.size() > m_operators.size());
            assert(m_values.size() - m_operators.size() ==  1U);
            
            Universal right = m_values.back();
            m_values.pop_back();
            
            Universal left = m_values.back();
            m_values.pop_back();
            
            BinaryOperator op = m_operators.back();
            m_operators.pop_back();
            
            Universal result = op.Func(left, right);
            m_values.push_back(result);

            int a = 5;
        }

        std::vector<BinaryOperator> m_operators;
        std::vector<Universal> m_values;
    };
    
    struct BinaryStacks
    {
        BinaryStacks()
        {
            Open();
        }

        void Open()
        {
            m_stacks.push_back(BinaryOperatorsStack());
        }

        void PushOperator(const BinaryOperator& op)
        {
            assert(!m_stacks.empty());

            m_stacks.back().PushOperator(op);
        }

        void PushUniversal(const Universal& u)
        {
            assert(!m_stacks.empty());

            m_stacks.back().PushUniversal(u);
        }

        void Close()
        {
            assert(m_stacks.size() > 1);

            Universal u = m_stacks.back().Calculate();
            m_stacks.pop_back();

            m_stacks.back().PushUniversal(u);
        }

        Universal Calculate()
        {
            assert(m_stacks.size() == 1U);

            Universal result = m_stacks.back().Calculate();
            m_stacks.pop_back();

            return result;
        }

    private:
        std::vector<BinaryOperatorsStack> m_stacks;
    };
    
    struct MapStack
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
    
    struct ReduceStack
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
    
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;

	struct Real : seq< opt< one<'+', '-'> >, star<digit>, one<'.'>, plus<digit> > { };

	struct Integer : seq< opt< one<'+', '-'> >, plus<digit>, not_at< one<'.'> > > { };

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
        static bool match(Input& in, BinaryStacks& stacks)
        {
            const auto opIt = BIN_OPERATORS.find(in.peek_char(0));
            if (opIt != BIN_OPERATORS.cend())
            {
                stacks.PushOperator(opIt->second);
                in.bump(1);

                return true;
            }
            
            return false;;
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
        static bool match( Input& in, BinaryStacks& stacks )
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
        static bool match( Input& in, BinaryStacks& stacks, std::string t )
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
        static bool match( Input& in, BinaryStacks& stacks )
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
        static bool match( Input& in, BinaryStacks& stacks, std::string t )
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
       static void apply(const Input& in, BinaryStacks&)
       {
           int a = 5;
           
           auto b = in.begin();
           auto e = in.end();
           
           std::string s = in.string();
           std::string s1 = in.string();
          //s.push( std::stol( in.string() ) );
       }
    };
    
    // This action will be called when the number rule matches; it converts the
   // matched portion of the input to a long and pushes it onto the operand
   // stack.
/*
   template<>
   struct action< number >
   {
      template< typename Input >
      static void apply( const Input& in, const operators&, stacks& s )
      {
         s.push( std::stol( in.string() ) );
      }
   };
*/
    template<>
    struct Action<Integer>
    {
        template< typename Input >
        static void apply(const Input& in, BinaryStacks& stacks)
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
        static void apply(const Input& in, BinaryStacks& stacks)
        {
            std::string strVal = in.string();
            float val = std::stof(strVal);
            stacks.PushUniversal(Universal(val));
        }
    };

    template<>
    struct Action< one<'('> >
    {
        static void apply0(BinaryStacks& stacks)
        {
            stacks.Open();
        }
    };

    template<>
    struct Action< one<')'> >
    {
        static void apply0(BinaryStacks& stacks)
        {
            stacks.Close();
        }
    };

	Universal Calculate(const std::string& expression)
    {
        analyze<Expression>();

        BinaryStacks stacks;
        memory_input<> input(expression, "entryExpr");
        bool parseResult = parse<Expression, Action>(input, stacks);

		if (parseResult)
		{
			Universal result = stacks.Calculate();

			return result;
		}
		else
		{
			assert(parseResult);
		}
		
		// Return invalid value
		return Universal();
    }
}
