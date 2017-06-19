#pragma once

#include "Common.h"
#include "Universal.h"

#include <tao/pegtl.hpp>

#include <cassert>
#include <vector>

namespace Abacus
{
  using namespace tao::TAOCPP_PEGTL_NAMESPACE;

  struct BinaryOperator
  {
    typedef Universal(*FuncType)(const Universal&, const Universal&);

    int Priority;
    FuncType Func;
    position Pos;
  };

  struct BinaryOperatorsStack
  {
    void PushOperator(const BinaryOperator& op)
    {
      if ((!m_operators.empty()) && (m_operators.back().Priority >=  op.Priority))
      {
        RollUp();
      }

      m_operators.push_back(op);
    }

    void PushUniversal(const Universal& u)
    {
      m_values.push_back(u);
    }

    Universal Calculate()
    {
      assert(m_values.size() > 0U);
      assert(m_values.size() > m_operators.size());
      assert((m_values.size() - m_operators.size()) == 1U);

      while (!m_operators.empty())
      {
        RollUp();
      }

      Universal result = m_values.front();

      return result;
    }

  private:

    void RollUp()
    {
      assert(m_values.size() > 1U);
      assert(m_values.size() > m_operators.size());
      assert(m_values.size() - m_operators.size() ==  1U);

      Universal right = m_values.back();
      m_values.pop_back();

      Universal left = m_values.back();
      m_values.pop_back();

      BinaryOperator op = m_operators.back();
      m_operators.pop_back();

      m_values.reserve(m_values.size() + 1U);

      try
      {
        Universal result = op.Func(left, right);
        m_values.push_back(result);
      }
      catch (const std::exception& err)
      {
        throw parse_error(err.what(), op.Pos);
      }
    }

    std::vector<BinaryOperator> m_operators;
    std::vector<Universal> m_values;
  };

  struct BinaryStacks
  {
    BinaryStacks()
    {
      Open();
    }

    void Open()
    {
      m_stacks.push_back(BinaryOperatorsStack());
    }

    void PushOperator(const BinaryOperator& op)
    {
      assert(!m_stacks.empty());

      m_stacks.back().PushOperator(op);
    }

    void PushUniversal(const Universal& u)
    {
      assert(!m_stacks.empty());

      m_stacks.back().PushUniversal(u);
    }

    void Close()
    {
      assert(m_stacks.size() > 1);

      Universal u = m_stacks.back().Calculate();
      m_stacks.pop_back();

      m_stacks.back().PushUniversal(u);
    }

    Universal Calculate()
    {
      assert(m_stacks.size() == 1U);

      Universal result = m_stacks.back().Calculate();
      m_stacks.pop_back();

      return result;
    }

  private:
    std::vector<BinaryOperatorsStack> m_stacks;
  };
}
