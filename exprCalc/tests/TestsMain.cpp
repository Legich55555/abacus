#include <cmath>
#include <cstdlib>
#include <climits>
#include <iostream>
#include <memory>

#include "exprCalc/ExprCalc.h"

/*
 * @brief Check expresssions
 * @return Number errors (both parsing errors and execution errors)
 */
unsigned CheckExpression(
    const std::string& expression,
    const Abacus::State& variables,
    const Abacus::Universal& expectedValue)
{
  const Abacus::Universal result = Abacus::Calculate(expression, variables, nullptr);

  if (result == expectedValue)
  {
    std::cout << "PASSED test for \"" << expression << "\"" << std::endl;
    return 0U;
  }
  else
  {
    std::cout << "FAILED test for \"" << expression << "\"" << std::endl;
    return 1U;
  }
}

unsigned CheckExpression(
    const std::string& expression,
    const Abacus::State& variables,
    const double& expectedValue,
    const double& maxDelta)
{
  const Abacus::Universal result = Abacus::Calculate(expression, variables, nullptr);
  if (result.Type != Abacus::Universal::Types::REAL)
  {
    std::cout << "FAILED test for \"" << expression << "\"" << std::endl;
    return 1U;
  }

  double delta = std::abs(expectedValue - result.Real);

  if (delta < maxDelta)
  {
    std::cout << "PASSED test for \"" << expression << "\"" << std::endl;
    return 0U;
  }
  else
  {
    std::cout << "FAILED test for \"" << expression << "\"" << std::endl;
    return 1U;
  }
}

unsigned CheckInvalidExpression(
    const std::string& expression,
    const Abacus::State& variables)
{
  if (!Abacus::Calculate(expression, variables, nullptr).IsValid())
  {
    std::cout << "PASSED test for " << expression << std::endl;
    return 0U;
  }
  else
  {
    std::cout << "FAILED test for " << expression << std::endl;
    return 1U;
  }
}

unsigned CheckStatement(
    const std::string& statement,
    const Abacus::State& variables,
    const Abacus::ExecResult& expectedResult)
{
  Abacus::ExecResult result = Abacus::Execute(statement, variables, nullptr);
  if (result.Brief != Abacus::ResultBrief::SUCCEEDED)
  {
    std::cout << "FAILED test for \"" << statement << "\"" << std::endl;
    return 1U;
  }

  if (expectedResult.Variables.size() != result.Variables.size())
  {
    std::cout << "FAILED test for \"" << statement << "\"" << std::endl;
    return 1U;
  }

  for (const auto& var : expectedResult.Variables)
  {
    auto v = result.Variables.find(var.first);

    if (v == result.Variables.cend())
    {
      std::cout << "FAILED test for \"" << statement << "\"" << std::endl;
      return 1U;
    }

    if (v->second != var.second)
    {
      std::cout << "FAILED test for \"" << statement << "\"" << std::endl;
      return 1U;
    }
  }

  if (expectedResult.Output != result.Output)
  {
    std::cout << "FAILED test for \"" << statement << "\"" << std::endl;
    return 1U;
  }

  if (expectedResult.Errors.size() != result.Errors.size())
  {
    std::cout << "FAILED test for \"" << statement << "\"" << std::endl;
    return 1U;
  }

  std::cout << "PASSED test for \"" << statement << "\"" << std::endl;

  return 0;
}

unsigned CheckProgramPi()
{
  const std::vector<std::string> program
  {
    "var n = 500",
    "var sequence = map({0, n}, i -> (-1.0)^i / (2 * i + 1))",
    "var pi = 4 * reduce(sequence, 0, x y -> x + y)",
    "print \"pi = \"",
    "out pi"
  };

  Abacus::State variables;
  std::vector<std::string> output;

  for (const auto& statement : program)
  {
    Abacus::ExecResult result = Abacus::Execute(statement, variables, nullptr);
    if (result.Brief != Abacus::ResultBrief::SUCCEEDED)
    {
      std::cout << "FAILED test for program" << std::endl;
      return 1U;
    }

    variables.insert(result.Variables.cbegin(), result.Variables.cend());
    output.insert(output.end(), result.Output.cbegin(), result.Output.cend());
  }

  std::cout << "PASSED test for program" << std::endl;

  return 0;
}

int main()
{
  static const double MAX_SLOP = 0.0005;

  unsigned errorsNumber = 0;

  errorsNumber += CheckProgramPi();

  errorsNumber += CheckStatement(
        "print \"pi = \"",
        { },
        Abacus::ExecResult { Abacus::ResultBrief::SUCCEEDED, {}, {"pi = "}, {} }
        );

  errorsNumber += CheckStatement(
        "out 2 + 2",
        { },
        Abacus::ExecResult { Abacus::ResultBrief::SUCCEEDED, {}, {"4"}, {} }
        );

  errorsNumber += CheckExpression(
        " 4 * reduce( map({0, 50000}, i -> (-1.0)^i / (2.0 * i + 1)), 0, x y -> x + y )",
        {},
        3.1415,
        MAX_SLOP);

  errorsNumber += CheckExpression(
        " 4 * reduce( map({0, 3111}, i -> (-1.0)^i / (2.0 * i + 1)), 0, x y -> x + y )",
        {},
        3.1415,
        MAX_SLOP);

  errorsNumber += CheckExpression(
        " 4 * reduce( map({0, 5000}, i -> (-1.0)^i / (2 * i + 1)), 0, x y -> x + y )",
        {},
        3.1415,
        MAX_SLOP);

  errorsNumber += CheckExpression(
        " map({1, 5}, x->x * x )",
        {},
        Abacus::Universal(std::vector<int> {1, 4, 9, 16, 25}));

  errorsNumber += CheckExpression(
        "map({1, 2*2+1}, x->x * x ) ",
        {},
        Abacus::Universal(std::vector<int> {1, 4, 9, 16, 25}));

  errorsNumber += CheckExpression(
        "map   ({1, 2*2+1}, x->x * x ) ",
        {},
        Abacus::Universal(std::vector<int> {1, 4, 9, 16, 25}));

  errorsNumber +=  CheckInvalidExpression(
        "(a + UNDEFINED_VARIABLE)",
        {
          {"a", Abacus::Universal(1)},
          {"b", Abacus::Universal(5)},
        });

  errorsNumber += CheckInvalidExpression(
        "var a = b5",
        { {"b", Abacus::Universal(1)} }
        );

  errorsNumber += CheckExpression(
        "(a + b)",
        {
          {"a", Abacus::Universal(1)},
          {"b", Abacus::Universal(5)},
        },
        Abacus::Universal(6));

  errorsNumber += CheckExpression(
        "(a + b) - ccccccccc12",
        {
          {"a", Abacus::Universal(1)},
          {"b", Abacus::Universal(5)},
          {"ccccccccc12", Abacus::Universal(6)},
        },
        Abacus::Universal(0));

  errorsNumber += CheckExpression(" 1.0E-2", {}, Abacus::Universal(1.0E-2));
  errorsNumber += CheckExpression(" 1.0E+2", {}, Abacus::Universal(1.0E+2));
  errorsNumber += CheckExpression(" 1.0", {}, Abacus::Universal(1.));
  errorsNumber += CheckExpression(" 1.0 ", {}, Abacus::Universal(1.));
  errorsNumber += CheckExpression("1.0 ", {}, Abacus::Universal(1.));
  errorsNumber += CheckExpression("1.0", {}, Abacus::Universal(1.));
  errorsNumber += CheckExpression("9.0", {},  Abacus::Universal(9.));
  errorsNumber += CheckExpression("-1.0", {},  Abacus::Universal(-1.));
  errorsNumber += CheckExpression("+1.0", {}, Abacus::Universal(1.f));
  errorsNumber += CheckExpression("-.1", {},  Abacus::Universal(-0.1));
  errorsNumber += CheckExpression("+.1", {},  Abacus::Universal(0.1));
  errorsNumber += CheckExpression("-1", {},  Abacus::Universal(-1));
  errorsNumber += CheckExpression("-1.0", {},  Abacus::Universal(-1.));
  errorsNumber += CheckExpression("map({1, 2}, x -> x * 1.0)", {}, Abacus::Universal(std::vector<double>{1.0, 2.0}));
  errorsNumber += CheckExpression(" {  1 , 2 } ", {}, Abacus::Universal(std::vector<int>{1, 2}));
  errorsNumber += CheckExpression(" { 1,2 }", {}, Abacus::Universal(std::vector<int>{1, 2}));
  errorsNumber += CheckExpression("((1))", {},  Abacus::Universal(1));
  errorsNumber += CheckExpression("((1 + -2 + -1*+2.0))", {},  Abacus::Universal(-3.0));
  errorsNumber += CheckExpression("1 + 1", {},  Abacus::Universal(2));
  errorsNumber += CheckExpression("1 +2.0", {},  Abacus::Universal(3.0));
  errorsNumber += CheckExpression("+1*+2", {}, Abacus::Universal(2));
  errorsNumber += CheckExpression("-1 + 1", {},  Abacus::Universal(0));
  errorsNumber += CheckExpression("(((1 + 2) + 3) + 4)", {},  Abacus::Universal(10));
  errorsNumber += CheckExpression("(1 + -2 + -1*+2.0 + 10 / (3 + 2))", {},  Abacus::Universal(-1.0));
  errorsNumber += CheckExpression("((+1 + -2 + -1*+2 + 10 / (3 + 2)) + (4/2 + 1))+12/3", {},  Abacus::Universal(6.0));
  errorsNumber += CheckExpression("(1 + -2 + -1*+2.0)", {},  Abacus::Universal(-3.0));
  errorsNumber += CheckExpression("((1 + -2 + -1*+2.0))", {},  Abacus::Universal(-3.0));
  errorsNumber += CheckExpression("((+1 + -2 + -1*+2.0 + 10 / (3 + 2)) + (4/2 + 1))+12/3", {},  Abacus::Universal(6.0));
  errorsNumber += CheckExpression("1 + 2^(3+1)", {},  Abacus::Universal(17.));
  errorsNumber += CheckExpression("1 + 2^(3.0+1)", {},  Abacus::Universal(17.));
  errorsNumber += CheckExpression("1 + 2.0^(3+1)", {},  Abacus::Universal(17.));
  errorsNumber += CheckExpression("1 + 2.0^(3+1.0)", {},  Abacus::Universal(17.));

  errorsNumber += CheckInvalidExpression(
        "a + b",
        {
          {"a", Abacus::Universal(INT_MAX)},
          {"b", Abacus::Universal(INT_MAX)},
        });

  errorsNumber += CheckInvalidExpression(
        "a + b",
        {
          {"a", Abacus::Universal(INT_MIN)},
          {"b", Abacus::Universal(INT_MIN)},
        });

  errorsNumber += CheckInvalidExpression(
        "a - b",
        {
          {"a", Abacus::Universal(INT_MIN)},
          {"b", Abacus::Universal(INT_MAX)},
        });

  errorsNumber += CheckInvalidExpression(
        "a - b",
        {
          {"a", Abacus::Universal(INT_MAX)},
          {"b", Abacus::Universal(INT_MIN)},
        });


  errorsNumber += CheckStatement(
        "var a = 5",
        { },
        Abacus::ExecResult { Abacus::ResultBrief::SUCCEEDED, {}, {}, {{"a", Abacus::Universal(5)}} }
        );

  errorsNumber += CheckStatement(
        "var b = map( {1, a},  x -> x + x) ",
        { {"a", Abacus::Universal(5)} },
        Abacus::ExecResult
        {
          Abacus::ResultBrief::SUCCEEDED,
          {},
          {},
          { {"b", Abacus::Universal(std::vector<int> {2, 4, 6, 8, 10} )} } }
        );

  if (errorsNumber != 0)
  {
    std::cout << "Failed " << errorsNumber << " tests." << std::endl;
  }
  else
  {
    std::cout << "All tests passed." << std::endl;
  }

  return (0 == errorsNumber) ? EXIT_SUCCESS : EXIT_FAILURE;
}
