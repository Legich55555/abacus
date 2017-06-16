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

    namespace Map
    {
        template<typename IT, typename OT>
        void CalculateSubSequence(
            const char* inputCurr,
            const size_t inputSize,
            const IsTerminating& isTerminating,
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
                    throw TerminatedError {};
                }

                State lambdaParams = { {paramName, Universal(inputSequence[idx])} };

                memory_input<> input(inputCurr, inputSize, "CalculateSubSequence");
                
                Universal callResult;
                if (Expr::Parse(input, isTerminating, 1U, lambdaParams, callResult))
                {
                    outputSequence[idx] = GetNumber<OT>(callResult);
                }
                else
                {
                    throw parse_error(Print("Runtime error in lambda. Param: %s, Value: %s",
                                            paramName.c_str(), lambdaParams.cbegin()->second.ToString().c_str()),
                                      input);
                }
            }
        }

        template<typename Input, typename IT, typename OT>
        void CalculateSequence(
            const Input& input,
            const IsTerminating& isTerminating,
            const unsigned threads,
            const std::string& paramName,
            const std::vector<IT>& inputSequence,
            std::vector<OT>& outputSequence)
        {
            outputSequence.resize(inputSequence.size());
            
            static const size_t MIN_JOB_SIZE = 1000U;
            
            std::vector<std::future<void>> jobs;
            jobs.reserve(threads);
            
            size_t batchSize = inputSequence.size() / threads + 1U;
            batchSize = batchSize > MIN_JOB_SIZE ? batchSize : inputSequence.size();
            
            const char* inputCurr = input.current();
            size_t inputSize = input.size();

            for (size_t jobBeginIdx = 0; jobBeginIdx < inputSequence.size(); jobBeginIdx += batchSize)
            {
                size_t jobEndIdx = std::min(jobBeginIdx + batchSize, inputSequence.size());
                
                // The first job should be defered, other should be async
                std::launch jobType = jobBeginIdx != 0U ?
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
                catch (const TerminatedError& ex)
                {
                    isTerminated = true;
                }
                catch (const parse_error& err)
                {
                    jobErrors.push_back(err);
                }
            }

            if (isTerminated)
            {
                throw TerminatedError { };
            }
            else if (!jobErrors.empty())
            {
                throw parse_error(jobErrors.front());
            }
        }

        template<typename Input, typename OT>
        void CalculateSequence(
            const Input& input,
            const IsTerminating& isTerminating,
            const unsigned threads,
            const std::string& paramName,
            const Universal& inputSequence,
            std::vector<OT>& outputSequence)
        {
            if (Universal::Types::INT_SEQUENCE == inputSequence.Type)
            {
                CalculateSequence(input, isTerminating, threads, paramName, inputSequence.IntSequence, outputSequence);
            }
            else if (Universal::Types::REAL_SEQUENCE == inputSequence.Type)
            {
                CalculateSequence(input, isTerminating, threads, paramName, inputSequence.RealSequence, outputSequence);
            }
            else
            {
                throw parse_error(Print("Internal runtime error. Expected sequence type but got %s",
                                        inputSequence.ToString().c_str()),
                                  input);
            }
        }

        template<typename Input>
        Universal CalculateSequence(
            const Input& input,
            const IsTerminating& isTerminating,
            const unsigned threads,
            const std::string& paramName,
            const Universal& inputSequence,
            const Universal::Types expectedType)
        {
            Universal result;

            if (Universal::Types::INTEGER == expectedType)
            {
                std::vector<int> intResult(0);

                CalculateSequence(input, isTerminating, threads, paramName, inputSequence, intResult);

                result = std::move(Universal(std::move(intResult)));
            }
            else
            {
                std::vector<double> realResult(0);

                CalculateSequence(input, isTerminating, threads, paramName, inputSequence, realResult);

                result = std::move(Universal(std::move(realResult)));
            }

            return result;
        }

        template< typename Input >
        bool Parse(Input& input, IsTerminating isTerminating, unsigned threads, const State& variables, Universal& result)
        {
            struct MapBegin : seq< string<'m', 'a', 'p'>, star<space>, one<'('> > { };

            if (parse<MapBegin>(input) == false)
            {
                return false;
            }

            Universal firstValue;
            if (!Expr::Parse(input, isTerminating, threads, variables, firstValue))
            {
                throw parse_error("Failed to parse first map() parameter.", input);
            }

            if (!((firstValue.Type == Universal::Types::INT_SEQUENCE && !firstValue.IntSequence.empty()) ||
                  (firstValue.Type == Universal::Types::REAL_SEQUENCE && !firstValue.RealSequence.empty())))
            {
                throw parse_error(Print("First map() parameter is not sequence. Param: %s",
                                        firstValue.ToString().c_str()),
                                  input);
            }

            ExpectComma(input);
            std::string lambdaParameter = ExpectIdentifier(input);
            ExpectArrow(input);

            memory_input<> inputCopy(input.current(), input.size(), "CalculateSequence");
            
            // Calculate the first item of sequence
            Universal callResult;
            State lambdaParams = { { lambdaParameter, Universal(firstValue.IntSequence.front()) } };
            if (!Expr::Parse(input, isTerminating, threads, lambdaParams, callResult))
            {
                throw parse_error("Invalid lambda.", input);
            }
            if (!callResult.IsNumber())
            {                    
                throw parse_error(Print("Runtime error in map() lambda. Expected number but labmda returned %s.",
                                        callResult.ToString().c_str()),
                                  input);
            }

            result = std::move(CalculateSequence(inputCopy, isTerminating, threads, lambdaParameter, firstValue, callResult.Type));

            ExpectClosingBracket(input);
            
            return true;
        }
    }    
}
