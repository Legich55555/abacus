#pragma once

#include "Universal.h"

#include <map>
#include <string>
#include <vector>

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
    /**
     * @brief Variables is a set of variables in Abacus
     */
    typedef std::map<std::string, Universal> State;
    
    /**
     * @brief ExecResult represents result of statement execution.
     */
    struct ExecResult
    {
        /** @brief If execution was successfull or not */
        bool Success;

        /** 
         * @brief Output generated during of execution
         *
         * Output can be generated event if execution failed.
         */
        std::vector<std::string> Output;

        /** @brief Variables defined and calculated during of execution. */
        State Variables;
    };

    /**
     * @brief Calculates expression 
     * 
     * @param expression String with expression to be calculated.
     * @param variables Variables which are used in the expression.
     * 
     * @note Errors are printed to std::cout.
     * 
     * @return Result of calculation in form of Universal.
     */
    Universal Calculate(const std::string& expression, const State& variables);
    
    /**
     * @brief Execute statement 
     * 
     * @param statement String with statement to be executed.
     * @param variables Variables which are used in the statement.
     * 
     * @return Result of calculation in form of Universal.
     */
    ExecResult Execute(const std::string& statement, const State& variables);
    
//     ExecResult Execute(const std::vector<std::string>& program);
}
