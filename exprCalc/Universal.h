#pragma once

#include <string>
#include <vector>
#include <utility>

namespace Abacus
{
    // TODO: optimize Universal (move semantic and minimize its size).
    struct Universal
    {
        enum class Types 
        {
            INVALID,
            INTEGER,
            REAL,
            INT_SEQUENCE,
            REAL_SEQUENCE
        };
        
        Types Type;
        
        union
        {
            int Integer;
            double Real;
            std::vector<int> IntSequence;
            std::vector<double> RealSequence;
        };
        
        Universal() : Type(Types::INVALID) { }
        
        Universal(const Universal& other);
        Universal& operator=(const Universal& other);
        
        explicit Universal(int v) : Type(Types::INTEGER), Integer(v) {}
        explicit Universal(double v) : Type(Types::REAL), Real(v) {}
        explicit Universal(std::vector<int>&& sequence) : Type(Types::INT_SEQUENCE), IntSequence(std::move(sequence)) {}
        explicit Universal(const std::vector<int>& sequence) : Type(Types::INT_SEQUENCE), IntSequence(sequence) {}
        explicit Universal(const std::vector<double>& sequence) : Type(Types::REAL_SEQUENCE), RealSequence(sequence) {}
        
        ~Universal();
        
        bool IsValid() const { return Type != Universal::Types::INVALID; }
        bool IsNumber() const { return Type == Universal::Types::INTEGER || Type == Universal::Types::REAL; }
        
        std::string ToString() const;
    };
    
    Universal Mul(const Universal& l, const Universal& r);

    Universal Add(const Universal& l, const Universal& r);

    Universal Sub(const Universal& l, const Universal& r);

    Universal Div(const Universal& l, const Universal& r);

    Universal Pow(const Universal& l, const Universal& r);

    bool operator==(const Universal& l, const Universal& r);
    
    bool operator!=(const Universal& l, const Universal& r);
}
