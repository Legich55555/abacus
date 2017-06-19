#include "ExprCalc.h"

#include "ExprParse.h"
#include "StmtParse.h"

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp> // Include the analyze function that checks a grammar for possible infinite cycles.

namespace Abacus
{
  using namespace tao::TAOCPP_PEGTL_NAMESPACE;

  unsigned WORK_THREADS_NUM = 4U;

  Universal Calculate(const std::string& expression, const State& variables, IsTerminating isTerminating)
  {
    Universal result; // result is initialized as invalid value.

    try
    {
      memory_input<> input(expression.data(), expression.size(), "Calculate");

      Expr::Expect(input, isTerminating, WORK_THREADS_NUM, variables, result);

      return result;
    }
    catch (const parse_error& parseError)
    {
      std::cout << "Failed to parse expression" << std::endl;
    }

    return result;
  }

  ExecResult Execute(const std::string& statement, const State& variables, IsTerminating isTerminating)
  {
    ExecResult execResult = { ResultBrief::FAILED, {}, {}, {} };

    try
    {
      memory_input<> input(statement.data(), statement.size(), "Execute");

      // If it is an empty statement then it is ignored without rising any error.
      if (parse<star<space>>(input) && input.size() == 0)
      {
        execResult.Brief = ResultBrief::SUCCEEDED;
      }
      else if (Stmt::Parse(input, isTerminating, WORK_THREADS_NUM, variables, execResult.Output, execResult.Variables))
      {
        execResult.Brief = ResultBrief::SUCCEEDED;
      }
      else
      {
        execResult.Brief = ResultBrief::FAILED;
        execResult.Errors.push_back(Error { "Internal error.", {} });
      }
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

    return execResult;
  }
}
