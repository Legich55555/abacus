# pragma once

#include "BracesParse.h"
#include "ExprCalc.h"
#include "Universal.h"
#include <tao/pegtl.hpp>

#include <type_traits>
#include <future>
#include <thread>
#include <functional>

namespace ExprCalc
{
    using namespace tao::TAOCPP_PEGTL_NAMESPACE;

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
        OT GetUniversalValue(const Universal& u)
        {
            static_assert(std::is_integral<OT>::value || std::is_floating_point<OT>::value,
                          "Invalid sequence data type.");
            
            if (std::is_same<OT, int>::value)
            {
                return u.Integer;
            }
            else if (std::is_same<OT, double>::value)
            {
                return u.Real;
            }

            return OT();
        }
        
        template<typename Input, typename IT, typename OT>
        bool CalculateSubSequence(
            Input& input,
            const std::string& paramName,
            std::vector<IT>& inputSequence,
            const size_t beginIdx,
            const size_t endIdx,
            std::vector<OT>& outputSequence)
        {
            for (size_t idx = beginIdx; idx < endIdx; ++idx)
            {
                Variables lambdaParams = { {paramName, Universal(inputSequence[idx])} };

                Universal callResult = Calculate(input, "CalculateSubSequence", lambdaParams);
                
                if (GetUniversalSequenceType<OT>() != callResult.Type)
                {                    
                    throw parse_error("Runtime error in map() lambda.", input);
                }
                
                const OT v = GetUniversalValue<OT>(callResult);
                
                outputSequence[idx] = v;
            }
            
            return false;
        }

        template<typename Input, typename IT, typename OT>
        bool CalculateSequence(
            Input& input,
            const std::string& paramName,
            std::vector<IT>& inputSequence,
            std::vector<OT>& outputSequence)
        {
            outputSequence.resize(inputSequence.size());
            
            const size_t MAX_JOBS_NUM = 4U;
            const size_t MIN_JOB_SIZE = 1000U;
            
            std::vector<std::future<bool>> jobs;
            jobs.reserve(MAX_JOBS_NUM);
            
            size_t batchSize = inputSequence.size() / MAX_JOBS_NUM + 1U;
            batchSize = batchSize > MIN_JOB_SIZE ? batchSize : inputSequence.size();
            
            for (size_t jobBeginIdx = 0; jobBeginIdx < inputSequence.size(); jobBeginIdx += batchSize)
            {
                size_t jobEndIdx = std::min(jobEndIdx + batchSize, inputSequence.size());
                
                std::launch jobType = jobBeginIdx != 1U ? 
                    std::launch::async : std::launch::deferred;
                    
                auto jobFunc = std::bind(CalculateSubSequence<Input, IT, OT>,
                        std::ref(input), std::ref(paramName), std::ref(inputSequence), jobBeginIdx, jobEndIdx, std::ref(outputSequence));
                
                jobs.push_back(std::async(jobType, jobFunc));
            }
            
            bool totalResult = true;
            for (auto& job : jobs)
            {
                bool jobResult = job.get();
                totalResult = totalResult && jobResult; 
            }

            return totalResult;
            
//             return CalculateSubSequence(
//                 input, 
//                 paramName,
//                 inputSequence,
//                 0,
//                 inputSequence.size(),
//                 outputSequence);
        }

        template< typename Input >
        bool Parse(Input& input, const Variables& variables, Universal& result)
        {
            if (parse<MapBegin>(input) == false)
            {
                return false;
            }

            size_t parsed;
            Universal firstValue = Calculate(
                input.current(),
                input.size(),
                "Map parameter",
                parsed,
                variables);
            if (firstValue.Type != Universal::Types::INT_SEQUENCE ||
                firstValue.IntSequence.empty())
            {
                throw parse_error("First map parameter is not valid.", input);
            }
            
            input.bump(parsed);
            
            std::string lambdaParameter;
            if (parse<LambdaBegin, IdentifierAction>(input, lambdaParameter) == false)
            {
                throw parse_error("Invalid lambda syntax.", input);
            }
            
            // Calculate the first item of sequence
            size_t lambdaExprSize;
            auto seqIt = firstValue.IntSequence.cbegin();
            Variables lambdaParams = { {lambdaParameter, Universal(*seqIt)} };
            Universal callResult = Calculate(input.current(), input.size(), "Map lambda", lambdaExprSize, lambdaParams);
            if (!callResult.IsNumber())
            {                    
                throw parse_error("Runtime error in map() lambda. Result is expected to be a number.", input);
            }  
            
            if (Universal::Types::INTEGER == callResult.Type)
            {
                std::vector<int> intResult(0);
                
                CalculateSequence(input, lambdaParameter, firstValue.IntSequence, intResult);
            
                result = Universal(intResult);
            }
            else
            {
                std::vector<double> realResult(0);
                
                CalculateSequence(input, lambdaParameter, firstValue.IntSequence, realResult);
            
                result = Universal(realResult);
            }
            
            input.bump(lambdaExprSize);
            
            if (parse< pad< one<')'>, space> >(input) == false)
            {
                throw parse_error("Invalid map() syntax - no closing round brace.", input);
            }
            
            return true;
        }
    }    
}
