#pragma once

#include "Common.h"
#include "Universal.h"

#include <tao/pegtl.hpp>

namespace Abacus
{
  using namespace tao::TAOCPP_PEGTL_NAMESPACE;

  namespace Expr
  {
    template<typename Input>
    void Expect(Input& input,
                const IsTerminating& isTerminating,
                const unsigned threads,
                const State& variables,
                Universal& result);
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
      State params =
      {
        { firstParamName, std::move(firstParamVal) },
        { secondParamName, Universal(secondParamVal) }
      };

      Universal result;
      Expr::Expect(input, nullptr, 1U, params, result);
      if (!result.IsNumber())
      {
        throw parse_error(Print("reduce() lambda returned non number value. %s: %s, %s: %s",
                                firstParamName.c_str(), firstParamVal.ToString().c_str(),
                                secondParamName.c_str(), Universal(secondParamVal).ToString().c_str()),
                          input);
      }

      return result;
    }

    template< typename IT>
    Universal ReduceSubSequence(const char* inputCurr,
                                const size_t inputSize,
                                const IsTerminating& isTerminating,
                                const std::string& firstParamName,
                                const std::string& neutralParamName,
                                const Universal& neutralVal,
                                const std::vector<IT>& inputSequence,
                                const size_t beginIdx,
                                const size_t endIdx)
    {
      Universal intermediateValue(neutralVal);

      for (size_t idx = beginIdx; idx < endIdx; ++idx)
      {
        static const unsigned TERMINATE_CHECK_PERIOD = 100U;

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
                                            neutralParamName,
                                            inputSequence[idx]);
      }

      return intermediateValue;
    }

    template< typename IT, typename OT >
    Universal ReduceSubSequence(const char* inputCurr,
                                const size_t inputSize,
                                const IsTerminating& isTerminating,
                                const std::string& firstParamName,
                                const std::string& neutralParamName,
                                const Universal& neutralVal,
                                const std::vector<Universal>& inputSequence,
                                const size_t beginIdx,
                                const size_t endIdx)
    {
      std::vector<OT> newSequence;
      newSequence.reserve(inputSequence.size() + 1U);

      for (const auto& u : inputSequence)
      {
        if (u.Type == Universal::Types::INTEGER)
        {
          newSequence.push_back(GetValue<int>(u));
        }
        else
        {
          newSequence.push_back(GetValue<double>(u));
        }
      }

      return ReduceSubSequence(inputCurr,
                               inputSize,
                               isTerminating,
                               firstParamName,
                               neutralParamName,
                               neutralVal,
                               newSequence,
                               0,
                               newSequence.size());
    }

    template< typename Input, typename IT>
    Universal ReduceSequence(Input& input,
                             const unsigned threads,
                             const IsTerminating& isTerminating,
                             const std::string& firstParamName,
                             const std::string& secondParamName,
                             const Universal& neutralVal,
                             const std::vector<IT>& inputSequence)
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

      return ReduceSubSequence(inputCurr,
                               inputSize,
                               isTerminating,
                               firstParamName,
                               secondParamName,
                               firstLambdaResult,
                               jobResults,
                               0,
                               jobResults.size());
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
    bool Parse(Input& input,
               const IsTerminating& isTerminating,
               const unsigned threads,
               const State& variables,
               Universal& result)
    {
      struct ReduceBegin : seq<string< 'r', 'e', 'd', 'u', 'c', 'e' >, star<space>, one<'('> > { };

      if (parse<ReduceBegin>(input) == false)
      {
        return false;
      }

      Universal firstParamValue;
      Expr::Expect(input, isTerminating, threads, variables, firstParamValue);
      if (firstParamValue.Type != Universal::Types::INT_SEQUENCE &&
          firstParamValue.Type != Universal::Types::REAL_SEQUENCE)
      {
        throw parse_error(Print("Expected sequence. but actual value is %s", firstParamValue.ToString().c_str()),
                          input);
      }

      ExpectComma(input);

      Universal secondParamValue;
      Expr::Expect(input, isTerminating, threads, variables, secondParamValue);
      if (!secondParamValue.IsNumber())
      {
        throw parse_error(Print("Expected a number but actual value is %s.",
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
