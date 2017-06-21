#pragma once

#include "ExprCalc.h"

#include "ExprParse.h"
#include "Universal.h"

#include <string>
#include <vector>

#include <tao/pegtl.hpp>

namespace Abacus
{
  namespace Stmt
  {
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;

    template< typename Rule >
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

    struct Assignment
    {
      using analyze_t = analysis::generic< analysis::rule_type::ANY >;

      template< apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input >
      static bool match(Input& input,
                        const IsTerminating& isTerminating,
                        const unsigned threads,
                        const State& variables,
                        std::vector<std::string>&,
                        State& newVariables)
      {
        struct VarDefBegin : seq< star<space>, string<'v', 'a', 'r'>, plus<space> > {};

        if (!parse< VarDefBegin >(input))
        {
          return false;
        }

        struct VarDefName : seq< identifier, star<space> > {};

        std::string variableName;
        if (!parse<VarDefName, IdentifierAction>(input, variableName))
        {
          throw parse_error("Expected variable name", input);
        }

        ExpectChar<'='>(input);

        Universal variableValue;
        Expr::Expect(input, isTerminating, threads, variables, variableValue);

        newVariables[variableName] = variableValue;

        return true;
      }
    };

    struct PrintExpr
    {
      using analyze_t = analysis::generic< analysis::rule_type::ANY >;

      template< apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename Input >
      static bool match(Input& input,
                        const IsTerminating& isTerminating,
                        const unsigned threads,
                        const State& variables,
                        std::vector<std::string>& output,
                        State&)
      {
        struct PrintExprBegin : seq< star<space>, string<'o', 'u', 't'>, plus<space> > { };

        if (!parse<PrintExprBegin>(input))
        {
          return false;
        }

        Universal expressionValue;
        Expr::Expect(input, isTerminating, threads, variables, expressionValue);

        output.push_back(expressionValue.ToString());

        return true;
      }
    };

    struct Text : seq< star< not_one<'"'> > > { };

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
      static bool match(Input& input,
                        const IsTerminating&,
                        const unsigned,
                        const State&,
                        std::vector<std::string>& output,
                        State&)
      {
        struct PrintTextBegin : seq< star<space>, string<'p', 'r', 'i', 'n', 't'>, plus<space> > { };

        if (!parse<PrintTextBegin>(input))
        {
          return false;
        }

        ExpectChar<'"'>(input, false);

        std::string text;
        if (!parse<Text, TextAction>(input, text))
        {
          throw parse_error("Expected string.", input);
        }

        ExpectChar<'"'>(input);

        output.push_back(text);

        return true;
      }
    };

    struct Statement : seq< sor<Assignment, PrintExpr, PrintText>, star< space > >  { };

    template<typename Input>
    bool Parse(Input& input,
               const IsTerminating& isTerminating,
               const unsigned threads,
               const State& variables,
               std::vector<std::string>& output,
               State& newVariables)
    {
      return parse<Statement>(input, isTerminating, threads, variables, output, newVariables);
    }
  }
}
