#include "Abacus.h"

#include "ExprParse.h"
#include "StmtParse.h"
#include "BinaryStack.h"

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp> // Include the analyze function that checks a grammar for possible infinite cycles.

namespace Abacus
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;
    
    Universal Calculate(const std::string& expression, const State& variables)
    {
        Universal result; // result is initialized as invalid value.

        try
        {
            memory_input<> input(expression.data(), expression.size(), "Calculate");
            if (!Expr::Parse(input, variables, result))
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

    ExecResult Execute(const std::string& statement, const State& variables)
    {
        ExecResult execResult = {false};
        
        try
        {
            memory_input<> input(statement.data(), statement.size(), "Execute");
            if (Stmt::Parse(input, execResult, variables))
            {
                execResult.Success = true;
            }
        }
        catch (const parse_error& parseError)
        {
            execResult.Output.push_back(parseError.what());
            execResult.Output.push_back("Statement failed.");
        }
        
        return execResult; 
    }
    
}
