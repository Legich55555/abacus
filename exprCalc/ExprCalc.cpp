#include "ExprCalc.h"

#include "ExprParse.h"
#include "StmtParse.h"

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp> // Include the analyze function that checks a grammar for possible infinite cycles.

#include <iterator>

namespace Abacus
{
  using namespace tao::TAOCPP_PEGTL_NAMESPACE;

  // Simplification: expected 4-core systems. 1 core for GUI and 3 for background tasks.
  unsigned WORK_THREADS_NUM = 3U;

  Universal Calculate(const std::string& expression, const State& variables, IsTerminating isTerminating)
  {
    Universal result; // result is initialized as invalid value.

    try
    {
      memory_input<> input(expression.data(), expression.size(), "Calculate");

      Expr::Expect(input, isTerminating, WORK_THREADS_NUM, variables, result);

      return result;
    }
    catch (const parse_error& err)
    {
      std::string positionStr = err.positions.empty() ? "N/A" : to_string(err.positions.front());

      std::cout << "Failed to interpret expression. Reason: " << err.what() <<
                   ", pos: " << positionStr << std::endl;
    }

    return result;
  }

  ExecResult Execute(const std::string& statement, const State& variables, IsTerminating isTerminating)
  {
    ExecResult execResult = { ResultBrief::FAILED, {}, {}, {} };
    State currentVariables = variables;
    memory_input<> input(statement.data(), statement.size(), "Execute");

    try
    {
        parse<star<space>>(input);

        while (!input.empty())
        {
          State newVariables;

          bool stmtParseResult = Stmt::Parse(input,
                                             isTerminating,
                                             WORK_THREADS_NUM,
                                             currentVariables,
                                             execResult.Output,
                                             newVariables);

          newVariables.insert(currentVariables.cbegin(), currentVariables.cend());

          currentVariables.swap(newVariables);

          if (!stmtParseResult || !parse< sor< plus<space>, eol, eolf >  > (input))
          {
            throw parse_error("Expected 'var', 'print' or 'out'", input);
          }
        }

        execResult.Brief = ResultBrief::SUCCEEDED;
    }
    catch (const TerminatedError&)
    {
      execResult.Brief = ResultBrief::TERMINATED;
    }
    catch (const parse_error& err)
    {
      execResult.Brief = ResultBrief::FAILED;

      std::vector<Abacus::Position> errorPositions;
      errorPositions.reserve(err.positions.size());
      for (const auto& pos : err.positions)
      {
        errorPositions.push_back(Abacus::Position { pos.byte } );
      }

      execResult.Errors.push_back(Error { err.what(), std::move(errorPositions) });
    }

    execResult.Variables.swap(currentVariables);

    return execResult;
  }
}
