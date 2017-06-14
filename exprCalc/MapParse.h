#pragma once

#include "ExprCalc.h"
#include "Common.h"
#include "Universal.h"

#include <tao/pegtl.hpp>

#include <future>
#include <thread>
#include <functional>
#include <type_traits>

namespace Abacus
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;
    
    namespace Expr
    {
        // Forward declaration for Expr::Parse()
        template< typename Input >
        bool Parse(Input& input, IsTerminating isTerminating, unsigned threads, const State& variables, Universal& result);
    }

    // TODO: introduce own class for exceptions. Catch all std::exceptions.

    namespace Map
    {
        struct MapBegin : seq< string<'m', 'a', 'p'>, star<space>, one<'('> > { };
        
        struct LambdaBegin : seq< pad< one<','>, space>, identifier, pad< string<'-','>' >, space> > { };
        
        template<typename Rule>
        struct IdentifierAction : nothing<Rule> { };
            
        template<>
        struct IdentifierAction<identifier>
        {
            template< typename Input >
            static void apply(const Input& in, std::string& lambdaParameter)
            {
                lambdaParameter = in.string();
            }
        };
        
        template<typename IT, typename OT>
        void CalculateSubSequence(
            const char* inputCurr,
            size_t inputSize,
            IsTerminating isTerminating,
            const std::string& paramName,
            const std::vector<IT>& inputSequence,
            const size_t beginIdx,
            const size_t endIdx,
            std::vector<OT>& outputSequence)
        {
            static const unsigned TERMINATE_CHECK_PERIOD = 100U;

            for (size_t idx = beginIdx; idx < endIdx; ++idx)
            {
                if (isTerminating != nullptr &&
                    idx % TERMINATE_CHECK_PERIOD == 0 &&
                    isTerminating())
                {
                    throw TerminatedException {};
                    //return ParseResult { ResultBrief::TERMINATED, { } };
                }

                State lambdaParams = { {paramName, Universal(inputSequence[idx])} };

                memory_input<> input(inputCurr, inputSize, "CalculateSubSequence");
                
                Universal callResult;
                if (Expr::Parse(input, isTerminating, 1U, lambdaParams, callResult))
                {
                    outputSequence[idx] = GetNumber<OT>(callResult);
                    //outputSequence[idx] = outputSequence[idx] + 1;
                }
                else
                {
                    throw parse_error("Map lambda execution error", input);
                }

//                try
//                {
//                    if (Expr::Parse(input, isTerminating, 1U, lambdaParams, callResult))
//                    {
//                        outputSequence[idx] = callResult.GetValue<OT>();
//                    }
//                    else
//                    {
//                        throw parse_error("Map lambda execution error", input);
////                        return ParseResult
////                        {
////                            ResultBrief::FAILED, { { "Map lambda execution error", { input.position() } } }
////                        };
//                    }
//                }
//                catch (const parse_error& err)
//                {
//                    return ParseResult
//                    {
//                        ResultBrief::FAILED, { { err.what(), err.positions } }
//                    };
//                }
//                catch (const std::runtime_error& err)
//                {
//                    return ParseResult
//                    {
//                        ResultBrief::FAILED, { { err.what(), { input.position() } } }
//                    };
//                }
            }
            
//            return ParseResult { ResultBrief::SUCCEEDED, { } };
        }

        template<typename Input, typename IT, typename OT>
        void CalculateSequence(
            const Input& input,
            IsTerminating isTerminating,
            unsigned threads,
            const std::string& paramName,
            std::vector<IT>& inputSequence,
            std::vector<OT>& outputSequence)
        {
            outputSequence.resize(inputSequence.size());
            
            const size_t MIN_JOB_SIZE = 1000U;
            
            std::vector<std::future<void>> jobs;
            jobs.reserve(threads);
            
            size_t batchSize = inputSequence.size() / threads + 1U;
            batchSize = batchSize > MIN_JOB_SIZE ? batchSize : inputSequence.size();
            
            const char* inputCurr = input.current();
            size_t inputSize = input.size();

            for (size_t jobBeginIdx = 0; jobBeginIdx < inputSequence.size(); jobBeginIdx += batchSize)
            {
                size_t jobEndIdx = std::min(jobBeginIdx + batchSize, inputSequence.size());
                
                std::launch jobType = jobBeginIdx != 1U ? 
                    std::launch::async : std::launch::deferred;

                auto jobFunc = std::bind(CalculateSubSequence<IT, OT>,
                        inputCurr, inputSize, isTerminating, std::ref(paramName), std::ref(inputSequence), jobBeginIdx, jobEndIdx, std::ref(outputSequence));
                
                jobs.push_back(std::async(jobType, jobFunc));
            }
            
            bool isTerminated = false;
            std::vector<parse_error> jobErrors;
            for (auto& job : jobs)
            {
                try
                {
                    job.get();
                }
                catch (const TerminatedException& ex)
                {
                    isTerminated = true;
                }
                catch (const parse_error& err)
                {
                    jobErrors.push_back(err);
                }
                catch (const std::exception& err)
                {
                    jobErrors.push_back(parse_error(err.what(), input));
                }
//                const JobResult jobResult = job.get();

//                // Result TERMINATED overrides FAILED because there is no way to link errors with source code
//                // in case of execution termination.
//                if (jobResult.Brief == ResultBrief::TERMINATED && result.Brief != ResultBrief::TERMINATED)
//                {
//                    result.Brief = ResultBrief::TERMINATED;
//                    result.Errors.swap(jobResult.Errors);
//                    break;
//                }
//                else if (jobResult.Brief == ResultBrief::FAILED && result.Brief != ResultBrief::FAILED)
//                {
//                    result.Brief = ResultBrief::FAILED;
//                }

//                result.Errors.insert(result.Errors.end(), jobResult.Errors.cbegin(), jobResult.Errors.cend());
            }

            if (isTerminated)
            {
                throw TerminatedException { };
            }
            else if (!jobErrors.empty())
            {
                throw parse_error(jobErrors.front());
            }
        }

        template< typename Input >
        bool Parse(Input& input, IsTerminating isTerminating, unsigned threads, const State& variables, Universal& result)
        {
            if (parse<MapBegin>(input) == false)
            {
                return false;
            }

            Universal firstValue;
            if (!Expr::Parse(input, isTerminating, threads, variables, firstValue))
            {
                throw parse_error("First map() parameter is not valid.", input);
            }
            if (firstValue.Type != Universal::Types::INT_SEQUENCE ||
                firstValue.IntSequence.empty())
            {
                // TODO: add support for real number sequences.
                throw parse_error("First map() parameter is not valid integer sequence.", input);
            }
            
            std::string lambdaParameter;
            if (parse<LambdaBegin, IdentifierAction>(input, lambdaParameter) == false)
            {
                throw parse_error("Invalid lambda syntax.", input);
            }

//            // Backup position for concurrent run.
//            const char* inputCurr = input.current();
//            size_t inputSize = input.size();
            memory_input<> inputCopy(input.current(), input.size(), "CalculateSequence");
            
            // Calculate the first item of sequence
            Universal callResult;
            State lambdaParams = { { lambdaParameter, Universal(firstValue.IntSequence.front()) } };
            if (!Expr::Parse(input, isTerminating, threads, lambdaParams, callResult))
            {
                throw parse_error("Invalid lambda syntax.", input);
            }
            if (!callResult.IsNumber())
            {                    
                throw parse_error("Runtime error in map() lambda. Result is not a number.", input);
            }  
            
            if (Universal::Types::INTEGER == callResult.Type)
            {
                std::vector<int> intResult(0);
                
                CalculateSequence(inputCopy, isTerminating, threads, lambdaParameter, firstValue.IntSequence, intResult);
            
                result = Universal(intResult);
            }
            else
            {
                std::vector<double> realResult(0);
                
                CalculateSequence(inputCopy, isTerminating, threads, lambdaParameter, firstValue.IntSequence, realResult);
            
                result = Universal(realResult);
            }
            
            if (parse< pad< one<')'>, space> >(input) == false)
            {
                throw parse_error("Invalid map() syntax - no closing round brace.", input);
            }
            
            return true;
        }
    }    
}
