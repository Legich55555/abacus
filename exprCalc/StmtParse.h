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
      static bool match(
              Input& input,
              const IsTerminating& isTerminating,
              const unsigned threads,
              const State& variables,
              std::vector<std::string>&,
              State& newVariables)
      {
        std::string variableName;
        if (!parse<AssignmentBegin, IdentifierAction>(input, variableName))
        {
          return false;
        }

        Universal variableValue;
        Expr::Expect(input, isTerminating, threads, variables, variableValue);

        newVariables[variableName] = variableValue;

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
      static bool match(
              Input& input,
              const IsTerminating& isTerminating,
              const unsigned threads,
              const State& variables,
              std::vector<std::string>& output,
              State&)
      {
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

    struct PrintTextBegin : seq<
        star<space>,
        string<'p', 'r', 'i', 'n', 't'>,
        star<space>,
        one<'"'>> { };

    struct PrintTextEnd : one<'"'> { };

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
      static bool match(
              Input& input,
              const IsTerminating&,
              const unsigned,
              const State&,
              std::vector<std::string>& output,
              State&)
      {
        if (!parse<PrintTextBegin>(input))
        {
          return false;
        }

        std::string text;
        if (!parse<Text, TextAction>(input, text))
        {
          throw parse_error("Expected string.", input);
        }

        if (!parse<PrintTextEnd>(input))
        {
          throw parse_error("Expected \"", input);
        }

        output.push_back(text);

        return true;
      }
    };

    struct Statement : seq< sor<Assignment, PrintExpr, PrintText>, star< space > >  { };

    template<typename Input>
    bool Parse(
            Input& input,
            const IsTerminating& isTerminating,
            const unsigned threads,
            const State& variables,
            std::vector<std::string>& output,
            State& newVariables)
    {
      bool result = parse<Statement>(input, isTerminating, threads, variables, output, newVariables);
      if (result && input.empty())
      {
        return true;
      }

      if (result)
      {
        throw parse_error("Error in statement.", input);
      }

      throw parse_error("Expected statement.", input);
    }
  }
}
