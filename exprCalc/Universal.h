#pragma once

#include <string>
#include <vector>
#include <utility>
#include <exception>

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

        typedef std::vector<int> IntArray;
        typedef std::vector<double> RealArray;

        union
        {
            int Integer;
            double Real;
            IntArray IntSequence;
            RealArray RealSequence;
        };
        
        Universal() : Type(Types::INVALID) { }
        
        Universal(const Universal& other);
        Universal& operator=(const Universal& other);
        
        explicit Universal(int v) : Type(Types::INTEGER), Integer(v) {}
        explicit Universal(double v) : Type(Types::REAL), Real(v) {}
        explicit Universal(IntArray&& sequence) : Type(Types::INT_SEQUENCE), IntSequence(std::move(sequence)) {}
        explicit Universal(const IntArray& sequence) : Type(Types::INT_SEQUENCE), IntSequence(sequence) {}
        explicit Universal(const RealArray& sequence) : Type(Types::REAL_SEQUENCE), RealSequence(sequence) {}
        
        ~Universal();
        
        bool IsValid() const { return Type != Universal::Types::INVALID; }
        bool IsNumber() const { return Type == Universal::Types::INTEGER || Type == Universal::Types::REAL; }

        template<class T>
        const T& GetValue() const
        {
            static_assert(
                        std::is_same<T, int>::value ||
                        std::is_same<T, double>::value ||
                        std::is_same<T, IntArray>::value ||
                        std::is_same<T, RealArray>::value,
                        "Invalid data type.");

            if (std::is_same<T, int>::value && Universal::Types::INTEGER == Type)
            {
                return Integer;
            }
            else if (std::is_same<T, double>::value && Universal::Types::REAL == Type)
            {
                return Real;
            }
            else if (std::is_same<T, IntArray>::value && Universal::Types::INT_SEQUENCE == Type)
            {
                return IntSequence;
            }
            else if (std::is_same<T, RealArray>::value && Universal::Types::REAL_SEQUENCE == Type)
            {
                return RealSequence;
            }

            throw std::exception("Invalid call for GetValue().");
        }
        
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
