#pragma once

#include "ExprCalc.h"
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
        
        template<typename T>
        Universal::Types GetUniversalSequenceType()
        {
            static_assert(std::is_same<T, int>::value || std::is_same<T, double>::value, "Invalid sequence data type.");

            if (std::is_same<T, int>::value)
            {
                return Universal::Types::INTEGER;
            }
            else if (std::is_same<T, double>::value)
            {
                return Universal::Types::REAL;
            }
            
            return Universal::Types::INVALID;
        }
        
        template<class OT>
        OT GetValue(const Universal& u)
        {
            static_assert(std::is_integral<OT>::value || std::is_floating_point<OT>::value,
                          "Invalid sequence data type.");
            
            if (std::is_same<OT, int>::value)
            {
                assert(Universal::Types::INTEGER == u.Type);
                return u.Integer;
            }
            else if (std::is_same<OT, double>::value)
            {
                assert(Universal::Types::REAL == u.Type);
                return u.Real;
            }

            // It is not reachible.
            return OT();
        }

        struct JobResult
        {
            ResultBrief Brief;
            std::vector<Error> Errors;
        };

        template<typename IT, typename OT>
        JobResult CalculateSubSequence(
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
                    return JobResult { ResultBrief::TERMINATED, Error { { }, { 0U } } };
                }

                State lambdaParams = { {paramName, Universal(inputSequence[idx])} };

                memory_input<> input(inputCurr, inputSize, "CalculateSubSequence");
                
                Universal callResult;
                try
                {
                    if (Expr::Parse(input, isTerminating, 1U, lambdaParams, callResult))
                    {
                        const OT v = GetValue<OT>(callResult);

                        outputSequence[idx] = v;
                    }
                    else
                    {
                        return JobResult
                        {
                            ResultBrief::FAILED,
                            { { "Map lambda execution error"}, {input.byte()} }
                        };
                    }
                }
                catch (const parse_error& err)
                {
                    return JobResult
                    {
                        ResultBrief::FAILED,
                        { { "Map lambda execution error", err.what()}, err.positions }
                    };
                }
                catch (const std::runtime_error& err)
                {
                    return JobResult
                    {
                        ResultBrief::FAILED,
                        { { "Runtime error", err.what() }, { 0U } }
                    };
                }
                catch (...)
                {
                    return JobResult
                    {
                        ResultBrief::FAILED,
                        { { "Unnknown error" }, { 0U } }
                    };
                }
            }
            
            return JobResult
            {
                ResultBrief::SUCCEEDED,
                { { }, { } }
            };
        }

        template<typename IT, typename OT>
        bool CalculateSequence(
            const char* inputCurr,
            size_t inputSize,
            IsTerminating isTerminating,
            unsigned threads,
            const std::string& paramName,
            std::vector<IT>& inputSequence,
            std::vector<OT>& outputSequence)
        {
            outputSequence.resize(inputSequence.size());
            
            const size_t MIN_JOB_SIZE = 1000U;
            
            std::vector<std::future<JobResult>> jobs;
            jobs.reserve(threads);
            
            size_t batchSize = inputSequence.size() / threads + 1U;
            batchSize = batchSize > MIN_JOB_SIZE ? batchSize : inputSequence.size();
            
            for (size_t jobBeginIdx = 0; jobBeginIdx < inputSequence.size(); jobBeginIdx += batchSize)
            {
                size_t jobEndIdx = std::min(jobBeginIdx + batchSize, inputSequence.size());
                
                std::launch jobType = jobBeginIdx != 1U ? 
                    std::launch::async : std::launch::deferred;

                auto jobFunc = std::bind(CalculateSubSequence<IT, OT>,
                        inputCurr, inputSize, isTerminating, std::ref(paramName), std::ref(inputSequence), jobBeginIdx, jobEndIdx, std::ref(outputSequence));
                
                jobs.push_back(std::async(jobType, jobFunc));
            }
            
            bool totalResult = true;
            for (auto& job : jobs)
            {
                JobResult jobResult = job.get();
                totalResult = totalResult && jobResult.first;
            }

            return totalResult;
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
                throw parse_error("First map() parameter is not valid.", input);
            }
            
            std::string lambdaParameter;
            if (parse<LambdaBegin, IdentifierAction>(input, lambdaParameter) == false)
            {
                throw parse_error("Invalid lambda syntax.", input);
            }

            // Backup position for concurrent run.
            const char* inputCurr = input.current();
            size_t inputSize = input.size();
            
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
            
            bool calculateSequenceResult = false;
            if (Universal::Types::INTEGER == callResult.Type)
            {
                std::vector<int> intResult(0);
                
                calculateSequenceResult = 
                    CalculateSequence(inputCurr, inputSize, isTerminating, threads, lambdaParameter, firstValue.IntSequence, intResult);
            
                result = Universal(intResult);
            }
            else
            {
                std::vector<double> realResult(0);
                
                calculateSequenceResult = 
                    CalculateSequence(inputCurr, inputSize, isTerminating, threads, lambdaParameter, firstValue.IntSequence, realResult);
            
                result = Universal(realResult);
            }
            
            if (!calculateSequenceResult)
            {
                throw parse_error("Runtime error.", input);
            }

            if (parse< pad< one<')'>, space> >(input) == false)
            {
                throw parse_error("Invalid map() syntax - no closing round brace.", input);
            }
            
            return true;
        }
    }    
}
