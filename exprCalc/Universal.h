# pragma once

#include <string>
#include <vector>
#include <utility>

namespace ExprCalc
{
    struct Universal
    {
        enum class Types 
        {
            INVALID,
            INTEGER,
            REAL,
            INT_SEQUENCE,
            REAL_SEQUENCE,
            VARIABLE,   // TODO: figure out if it is really needed.
            FUTURE
        };
        
        Types Type;
        
        union
        {
            int Integer;
            float Real;
            std::string Variable;
            std::vector<int> IntSequence;
            std::vector<float> RealSequence;
        };
        
        Universal() : Type(Types::INVALID) { }
        
        Universal(const Universal& other);
        Universal& operator=(const Universal& other);
        
        explicit Universal(int v) : Type(Types::INTEGER), Integer(v) {}
        explicit Universal(float v) : Type(Types::REAL), Real(v) {}
        explicit Universal(const std::vector<int> sequence) : Type(Types::INT_SEQUENCE), IntSequence(sequence) {}
        explicit Universal(const std::vector<float>& sequence) : Type(Types::REAL_SEQUENCE), RealSequence(sequence) {}
        explicit Universal(const std::string& varName) : Type(Types::VARIABLE), Variable(varName) {}   
        
        Universal(int start, int stop);
        
        ~Universal();
        
        bool IsValid() const { return Type != Universal::Types::INVALID; };
        bool IsNumber() const { return Type == Universal::Types::INTEGER || Type == Universal::Types::REAL; };
    };
    
    Universal Mul(const Universal& l, const Universal& r);

    Universal Add(const Universal& l, const Universal& r);

    Universal Sub(const Universal& l, const Universal& r);

    Universal Div(const Universal& l, const Universal& r);

    Universal Pow(const Universal& l, const Universal& r);

	bool operator==(const Universal& l, const Universal& r);
}
