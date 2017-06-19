#pragma once

#include "Universal.h"

#include <map>
#include <string>
#include <vector>
#include <functional>

/**
 * @brief Abacus is a library for calculating math expression.
 *
 * Грамматика:
 *
 * expr ::= expr op expr | (expr) | identifier | { expr, expr } | number | map(expr, identifier -> expr) | reduce(expr, expr, identifier identifier -> expr)
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
namespace Abacus
{
  /** @brief State is a set of variables in Abacus */
  typedef std::map<std::string, Universal> State;

  /** @brief Function type for "Check if terminate was requested" callback */
  typedef std::function<bool()> IsTerminating;

  /** @brief The ResultBrief enum represents brief result of execution */
  enum ResultBrief
  {
    TERMINATED,
    SUCCEEDED,
    FAILED
  };

  /**
    * @brief Position represents a location of some character in statements text.
    *
    * @note Position is used to locate errors in statements text.
    */
  struct Position
  {
    size_t CharIdx;
  };

  /** @brief Error contains all information about a particular issue in statement. */
  struct Error
  {
    std::string Message;
    std::vector<Position> Positions;
  };

  /** @brief ExecResult represents result of statement execution. */
  struct ExecResult
  {
    /** @brief Brief result of execution */
    ResultBrief Brief;

    /** @brief Errors happened during of execution */
    std::vector<Error> Errors;

    /**
     * @brief Output generated during of execution
     *
     * @note Output can be generated even if execution was terminated or failed.
     */
    std::vector<std::string> Output;

    /** @brief Variables which were defined and calculated during of execution. */
    State Variables;
  };

  /**
    * @brief Calculates expression
    *
    * @param expression String with expression to be calculated.
    * @param variables Variables which are used in the expression.
    * @param isTerminating Function which should be used to check if calculation termination was requested.
    *
    * @return Result of calculation in form of Universal.
    */
  Universal Calculate(const std::string& expression,
                      const State& variables,
                      IsTerminating isTerminating);

  /**
    * @brief Execute statement
    *
    * @param statement String with statement to be executed.
    * @param variables Variables which are used in the statement.
    * @param isTerminating Function which should be used to check if calculation termination was requested.
    *
    * @return Result of calculation in form of ExecResult.
    */
  ExecResult Execute(const std::string& statement,
                     const State& variables,
                     IsTerminating isTerminating);
}
