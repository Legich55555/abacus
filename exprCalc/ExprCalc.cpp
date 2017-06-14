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
            if (!Expr::Parse(input, isTerminating, WORK_THREADS_NUM, variables, result))
            {
                std::cout << "Failed to parse expression." << std::endl;
            }

            return result;
        }
        catch (const parse_error& parseError)
        {
            std::cout << parseError.what() << std::endl;
        }
        
        return result;
    }    

    ExecResult Execute(const std::string& statement, const State& variables, IsTerminating isTerminating)
    {
        ExecResult execResult = { ResultBrief::SUCCEEDED, /*{},*/ {}, {} };

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
                execResult.Output.push_back("Syntax error");
            }
        }
        // TODO: review and rework exception handling
        catch (const TerminatedException& ex)
        {
            execResult.Brief = ResultBrief::TERMINATED;
        }
        catch (const parse_error& err)
        {
            execResult.Brief = ResultBrief::FAILED;

            execResult.Output.push_back(err.what());
            execResult.Output.push_back("Statement parsing failed.");
        }
        catch (const std::runtime_error& err)
        {
            execResult.Brief = ResultBrief::FAILED;

            execResult.Output.push_back(err.what());
            execResult.Output.push_back("Statement execution failed.");
        }
        
        return execResult; 
    }    
}
