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

  namespace Sequence
  {
    template<typename Input>
    bool Parse(Input& input, IsTerminating isTerminating, unsigned threads, const State& variables, Universal& result)
    {
      if (!parse< seq< one<'{'>, star<space> > >(input))
      {
        return false;
      }

      Universal firstValue;
      if (!Expr::Parse(input, isTerminating, threads, variables, firstValue))
      {
        throw parse_error("Failed to parse first sequence parameter.", input);
      }

      if (firstValue.Type != Universal::Types::INTEGER)
      {
        throw parse_error(Print("First sequence parameter has wrong type. Value: %s",
                                firstValue.ToString().c_str()),
                          input);
      }

      ExpectComma(input);

      Universal secondValue;
      {
        if (!Expr::Parse(input, isTerminating, threads, variables, secondValue))
        {
          throw parse_error("Failed to parse the second parameter in sequence.", input);
        }

        if (secondValue.Type != Universal::Types::INTEGER)
        {
          throw parse_error("Second sequence parameter is not valid.", input);
        }
      }

      ExpectChar<'}'>(input);

      const int step = secondValue.Integer > firstValue.Integer ? 1 : -1;
      const size_t size = static_cast<unsigned>(secondValue.Integer > firstValue.Integer ?
                                                  secondValue.Integer - firstValue.Integer + 1 : firstValue.Integer - secondValue.Integer + 1);

      static const size_t MAX_SEQUENCE_SIZE = 2000000U;

      if (size > MAX_SEQUENCE_SIZE)
      {
        throw parse_error(Print("Runtime error. Sequence exceeded maximal possible length. Max: %u, Requested: %u.",                                         static_cast<unsigned>(MAX_SEQUENCE_SIZE), static_cast<unsigned>(size)),
                          input);
      }

      std::vector<int> sequence;
      sequence.reserve(static_cast<size_t>(size));

      static const unsigned TERMINATE_CHECK_PERIOD = 1000U;

      for (int i = firstValue.Integer; i != secondValue.Integer; i += step)
      {
        if (isTerminating != nullptr &&
            i % TERMINATE_CHECK_PERIOD &&
            isTerminating())
        {
          throw TerminatedError {};
        }

        sequence.push_back(i);
      }

      sequence.push_back(secondValue.Integer);

      result = Universal(std::move(sequence));

      return true;
    }
  }
}

