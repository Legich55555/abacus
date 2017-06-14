#include "Universal.h"

#include <cmath>
#include <sstream>

namespace Abacus
{

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

Universal::~Universal()
{
    switch(Type)
    {
    case Types::INT_SEQUENCE:
        IntSequence.~IntArray();
        break;
    case Types::REAL_SEQUENCE:
        RealSequence.~RealArray();
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
    else if (Type == Types::INT_SEQUENCE)
    {
        std::stringstream stream;
        stream << "{" << IntSequence.front() << " ... " << IntSequence.back() << "}";
        return stream.str();
    }
    else if (Type == Types::REAL_SEQUENCE)
    {
        std::stringstream stream;
        stream << "{" << RealSequence.front() << " ... " << RealSequence.back() << "}";
        return stream.str();
    }
    else if (Type == Types::INVALID)
    {
        return std::string("Invalid");
    }

    throw std::runtime_error("Runtime error: unexpected workflow.");
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

    throw std::runtime_error("Runtime error: unexpected workflow.");
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

    throw std::runtime_error("Runtime error: unexpected workflow.");
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

    throw std::runtime_error("Runtime error: unexpected workflow.");
}

Universal Div(const Universal& l, const Universal& r)
{
    if (r.Type == Universal::Types::INTEGER)
    {
        if (l.Type == Universal::Types::INTEGER)
        {
            if (r.Integer != 0 && l.Integer % r.Integer == 0)
            {
                return Universal(l.Integer / r.Integer);
            }

            return Universal(l.Integer / double(r.Integer));
        }
        else if (l.Type == Universal::Types::REAL)
        {
            return Universal(l.Real / r.Integer);
        }
    }
    else if (r.Type == Universal::Types::REAL)
    {
        if (l.Type == Universal::Types::INTEGER)
        {
            return Universal(l.Integer / r.Real);
        }
        else if (l.Type == Universal::Types::REAL)
        {
            return Universal(l.Real / r.Real);
        }
    }

    throw std::runtime_error("Runtime error: unexpected workflow.");
}

Universal Pow(const Universal& l, const Universal& r)
{
    if (l.Type == Universal::Types::INTEGER)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            double v = std::pow(l.Integer, r.Integer);

            if (r.Integer > 0)
            {
                return Universal(static_cast<decltype(l.Integer)>(v));
            }

            return Universal(v);
        }
        else if (r.Type == Universal::Types::REAL)
        {
            double v = std::pow(l.Integer, r.Real);
            return Universal(v);
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

    throw std::runtime_error("Runtime error: unexpected workflow.");
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

    throw std::runtime_error("Runtime error: unexpected workflow.");
}

bool operator!=(const Universal& l, const Universal& r)
{
    return !(l == r);
}

}
