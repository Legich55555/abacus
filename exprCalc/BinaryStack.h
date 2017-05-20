# pragma once

#include <cassert>
#include <tao/pegtl.hpp>
#include "ExprCalc.h"
#include "Universal.h"

namespace ExprCalc
{
    struct BinaryOperator
    {
        int Priority;
        Universal(*Func)(const Universal&, const Universal&);
    };
    
    const std::map<char, BinaryOperator> BIN_OPERATORS = 
    {
        { '+', { 10, Add } },
        { '-', { 10, Sub } }, 
        { '/', { 20, Div } },
        { '*', { 30, Mul } },
        { '^', { 40, Pow } }
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
            
            Universal result = op.Func(left, right);
            m_values.push_back(result);
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
