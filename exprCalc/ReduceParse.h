#pragma once

#include "Common.h"
#include "Universal.h"

#include <tao/pegtl.hpp>

namespace Abacus
{
using namespace tao::TAOCPP_PEGTL_NAMESPACE;

namespace Expr
{
// Forward declaration for Expr::Parse()
template< typename Input >
bool Parse(Input& input, IsTerminating isTerminating, unsigned threads, const State& variables, Universal& result);
}

namespace Reduce
{

template <typename Input, typename IT>
Universal CalculateLambda(Input& input,
                          const std::string& firstParamName,
                          const Universal& firstParamVal,
                          const std::string& secondParamName,
                          const IT secondParamVal)
{
    State lambdaParams =
    {
        {firstParamName, std::move(firstParamVal)},
        {secondParamName, Universal(secondParamVal)}
    };

    Universal lambdaResult;
    if (!Expr::Parse(input, nullptr, 1U, lambdaParams, lambdaResult))
    {
        throw parse_error(Print("Failed to calculate reduce() lambda. %s: %s, %s: %s",
                                firstParamName.c_str(), firstParamVal.ToString().c_str(),
                                secondParamName.c_str(), Universal(secondParamVal).ToString().c_str()),
                          input);
    }
    if (!lambdaResult.IsNumber())
    {
        throw parse_error(Print("reduce() lambda returned non number value. %s: %s, %s: %s",
                                firstParamName.c_str(), firstParamVal.ToString().c_str(),
                                secondParamName.c_str(), Universal(secondParamVal).ToString().c_str()),
                          input);
    }

    return lambdaResult;
}

template< typename IT>
Universal ReduceSubSequence(const char* inputCurr,
                            const size_t inputSize,
                            const IsTerminating& isTerminating,
                            const std::string& firstParamName,
                            const std::string& secondParamName,
                            const Universal& neutralVal,
                            const std::vector<IT>& inputSequence,
                            const size_t beginIdx,
                            const size_t endIdx)
{
    static const unsigned TERMINATE_CHECK_PERIOD = 100U;

    Universal intermediateValue(neutralVal);

    for (size_t idx = beginIdx; idx < endIdx; ++idx)
    {
        if (idx % TERMINATE_CHECK_PERIOD == 0 &&
                isTerminating != nullptr &&
                isTerminating())
        {
            throw TerminatedError {};
        }

        memory_input<> input(inputCurr, inputSize, "reduce() lambda");

        intermediateValue = CalculateLambda(input,
                                            firstParamName,
                                            intermediateValue,
                                            secondParamName,
                                            inputSequence[idx]);
    }

    return intermediateValue;
}

template< typename Input, typename IT>
Universal ReduceSequence(Input& input,
                         const unsigned threads,
                         const IsTerminating& isTerminating,
                         const std::string& firstParamName,
                         const std::string& secondParamName,
                         const Universal& neutralVal,
                         const std::vector<IT>& inputSequence
                         )
{
    static const size_t MIN_JOB_SIZE = 1000U;

    if (inputSequence.empty())
    {
        throw parse_error("reduce() requires non-empty sequence.", input);
    }

    const char* inputCurr = input.current();
    const size_t inputSize = input.size();

    Universal firstLambdaResult = CalculateLambda(input,
                                                  firstParamName,
                                                  neutralVal,
                                                  secondParamName,
                                                  inputSequence.front());

    std::vector<std::future<Universal>> jobs;
    jobs.reserve(threads);

    size_t batchSize = inputSequence.size() / threads + 1U;
    batchSize = batchSize > MIN_JOB_SIZE ? batchSize : inputSequence.size();

    // Start from 1 because 0 item is already used for firstLambdaResult.
    for (size_t jobBeginIdx = 1U; jobBeginIdx < inputSequence.size(); jobBeginIdx += batchSize)
    {
        const size_t jobEndIdx = std::min(jobBeginIdx + batchSize, inputSequence.size());

        // The first job should be defered, other should be async
        std::launch jobType = jobBeginIdx != 0U ?
                    std::launch::async : std::launch::deferred;

        auto jobFunc = std::bind(ReduceSubSequence<IT>,
                                 inputCurr,
                                 inputSize,
                                 std::ref(isTerminating),
                                 std::ref(firstParamName),
                                 std::ref(secondParamName),
                                 std::ref(neutralVal),
                                 std::ref(inputSequence),
                                 jobBeginIdx,
                                 jobEndIdx);

        jobs.push_back(std::async(jobType, jobFunc));
    }

    bool isTerminated = false;

    std::vector<parse_error> jobErrors;
    jobErrors.reserve(jobs.size());

    std::vector<Universal> jobResults;
    jobResults.reserve(jobs.size());

    for (auto& job : jobs)
    {
        try
        {
            jobResults.push_back(job.get());
        }
        catch (const TerminatedError&)
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

    std::vector<double> tmpRes;
    for (const auto& r : jobResults)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            tmpRes.push_back(r.Integer);
        }
        else
        {
            tmpRes.push_back(r.Real);
        }
    }

    return ReduceSubSequence(inputCurr,
                             inputSize,
                             isTerminating,
                             firstParamName,
                             secondParamName,
                             firstLambdaResult,
                             tmpRes,
                             0,
                             tmpRes.size());
}

template< typename Input>
Universal ReduceSequence(Input& input,
                         const unsigned threads,
                         const IsTerminating& isTerminating,
                         const std::string& firstParamName,
                         const std::string& secondParamName,
                         const Universal& neutralVal,
                         const Universal& inputSequence)
{
    if (Universal::Types::REAL_SEQUENCE == inputSequence.Type)
    {
        return ReduceSequence(input,
                                threads,
                                isTerminating,
                                firstParamName,
                                secondParamName,
                                neutralVal,
                                inputSequence.RealSequence);
    }
    else if (Universal::Types::INT_SEQUENCE == inputSequence.Type)
    {
        return ReduceSequence(input,
                                threads,
                                isTerminating,
                                firstParamName,
                                secondParamName,
                                neutralVal,
                                inputSequence.IntSequence);
    }

    throw parse_error("Internal runtime error.", input);
}

template< typename Input >
bool Parse(Input& input, IsTerminating isTerminating, unsigned threads, const State& variables, Universal& result)
{
    struct ReduceBegin : seq<string< 'r', 'e', 'd', 'u', 'c', 'e' >, star<space>, one<'('> > { };

    if (parse<ReduceBegin>(input) == false)
    {
        return false;
    }

    Universal firstParamValue;
    if (!Expr::Parse(input, isTerminating, threads, variables, firstParamValue))
    {
        throw parse_error("First reduce() parameter is not valid.", input);
    }
    if (firstParamValue.Type != Universal::Types::INT_SEQUENCE &&
        firstParamValue.Type != Universal::Types::REAL_SEQUENCE)
    {
        throw parse_error("First reduce() parameter is not valid.", input);
    }

    ExpectComma(input);

    Universal secondParamValue;
    if (!Expr::Parse(input, isTerminating, threads, variables, secondParamValue))
    {
        throw parse_error("Failed to parse the second reduce() parameter.", input);
    }
    if (!secondParamValue.IsNumber())
    {
        throw parse_error(Print("Second reduce() parameter is must be a number but actual value is %s.",
                                secondParamValue.ToString().c_str()),
                          input);
    }

    ExpectComma(input);
    const std::string firstParamName = ExpectIdentifier(input);
    const std::string secondParamName = ExpectIdentifier(input);
    ExpectArrow(input);

    result = ReduceSequence(input,
                            threads,
                            isTerminating,
                            firstParamName,
                            secondParamName,
                            secondParamValue,
                            firstParamValue);

    ExpectClosingBracket(input);

    return true;
}
}
}
