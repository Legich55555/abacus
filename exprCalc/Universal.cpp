#include "Universal.h"

#include <cmath>
#include <cassert>

namespace Abacus
{
// TODO: improve error reporting

Universal::Universal(const Universal& other)
    : Type(Types::INVALID)
{
    switch(other.Type)
    {
    case Types::INTEGER:
        Integer = other.Integer;
        Type = other.Type;
        break;
    case Types::REAL:
        Real = other.Real;
        Type = other.Type;
        break;
    case Types::INT_SEQUENCE:
        new (&IntSequence) decltype(IntSequence)(other.IntSequence);
        Type = other.Type;
        break;
    case Types::REAL_SEQUENCE:
        new (&RealSequence) decltype(RealSequence)(other.RealSequence);
        Type = other.Type;
        break;
    default:
        Type = Types::INVALID;
    }
}

Universal::Universal(int start, int stop)
    : Type(Types::INVALID)
{
    if (stop < start)
    {
        throw 1;
    }

    std::vector<int> sequence;
    sequence.reserve(stop - start + 1);

    for (int i = start; i <= stop; ++i)
    {
        sequence.push_back(i);
    }

    new (&this->IntSequence) std::vector<int>(std::move(sequence));
    Type = Types::INT_SEQUENCE;
}

Universal::~Universal()
{
    using std::string;
    using std::pair;
    using std::vector;

    switch(Type)
    {
    case Types::INT_SEQUENCE:
        IntSequence.~vector<int>();
        break;
    case Types::REAL_SEQUENCE:
        RealSequence.~vector<double>();
        break;
    default:
        break;
    }

    Type = Types::INVALID;
}

Universal& Universal::operator=(const Universal& other)
{
    this->~Universal();

    new (this) Universal(other);

    return *this;
}

std::string Universal::ToString() const
{
    if (Type == Types::INTEGER)
    {
        return std::to_string(Integer);
    }
    else if (Type == Types::REAL)
    {
        return std::to_string(Real);
    }
    
    return "Not implemented";
}

Universal Mul(const Universal& l, const Universal& r)
{
    if (l.Type == Universal::Types::INTEGER)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            return Universal(l.Integer * r.Integer);
        }
        else if (r.Type == Universal::Types::REAL)
        {
            return Universal(l.Integer * r.Real);
        }
    }
    else if (l.Type == Universal::Types::REAL)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            return Universal(l.Real * r.Integer);
        }
        else if (r.Type == Universal::Types::REAL)
        {
            return Universal(l.Real * r.Real);
        }
    }

    throw 1;
}

Universal Add(const Universal& l, const Universal& r)
{
    if (l.Type == Universal::Types::INTEGER)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            return Universal(l.Integer + r.Integer);
        }
        else if (r.Type == Universal::Types::REAL)
        {
            return Universal(l.Integer + r.Real);
        }
    }
    else if (l.Type == Universal::Types::REAL)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            return Universal(l.Real + r.Integer);
        }
        else if (r.Type == Universal::Types::REAL)
        {
            return Universal(l.Real + r.Real);
        }
    }

    throw 1;
}

Universal Sub(const Universal& l, const Universal& r)
{
    if (l.Type == Universal::Types::INTEGER)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            return Universal(l.Integer - r.Integer);
        }
        else if (r.Type == Universal::Types::REAL)
        {
            return Universal(l.Integer - r.Real);
        }
    }
    else if (l.Type == Universal::Types::REAL)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            return Universal(l.Real - r.Integer);
        }
        else if (r.Type == Universal::Types::REAL)
        {
            return Universal(l.Real - r.Real);
        }
    }

    throw 1;
}

Universal Div(const Universal& l, const Universal& r)
{
    if (l.Type == Universal::Types::INTEGER)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            return Universal(l.Integer / r.Integer);
        }
        else if (r.Type == Universal::Types::REAL)
        {
            return Universal(l.Integer / r.Real);
        }
    }
    else if (l.Type == Universal::Types::REAL)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            return Universal(l.Real / r.Integer);
        }
        else if (r.Type == Universal::Types::REAL)
        {
            return Universal(l.Real / r.Real);
        }
    }

    throw 1;
}

Universal Pow(const Universal& l, const Universal& r)
{
    if (l.Type == Universal::Types::INTEGER)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            const auto v = std::pow(l.Integer, r.Integer);
            return Universal(static_cast<decltype(l.Integer)>(v));
        }
        else if (r.Type == Universal::Types::REAL)
        {
            const auto v = std::pow(l.Integer, r.Real);
            return Universal(static_cast<double>(v));
        }
    }
    else if (l.Type == Universal::Types::REAL)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            const auto v = std::pow(l.Real, r.Integer);
            return Universal(static_cast<double>(v));
        }
        else if (r.Type == Universal::Types::REAL)
        {
            const auto v = std::pow(l.Real, r.Real);
            return Universal(static_cast<double>(v));
        }
    }

    throw 1;
}

bool operator==(const Universal& l, const Universal& r)
{
    if (l.Type != r.Type)
    {
        return false;
    }

    if (l.Type == Universal::Types::INVALID)
    {
        return true;
    }

    if (l.Type == Universal::Types::INTEGER)
    {
        return l.Integer == r.Integer;
    }

    if (l.Type == Universal::Types::REAL)
    {
        return l.Real == r.Real;
    }

    if (l.Type == Universal::Types::INT_SEQUENCE)
    {
        return l.IntSequence == r.IntSequence;
    }

    if (l.Type == Universal::Types::REAL_SEQUENCE)
    {
        return l.RealSequence == r.RealSequence;
    }

    assert(false);

    return false;
}

bool operator!=(const Universal& l, const Universal& r)
{
    return !(l == r);
}
    
}
