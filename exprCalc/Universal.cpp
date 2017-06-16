#include "Universal.h"
#include "Common.h"

#include <cmath>
#include <sstream>

#include <tao/pegtl.hpp>

namespace Abacus
{

using tao::TAOCPP_PEGTL_NAMESPACE::parse_error;


Universal::Universal(const Universal &other)
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

Universal::Universal(Universal &&other)
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
        new (&IntSequence) decltype(IntSequence)(std::move(other.IntSequence));
        Type = other.Type;
        break;
    case Types::REAL_SEQUENCE:
        new (&RealSequence) decltype(RealSequence)(std::move(other.RealSequence));
        Type = other.Type;
        break;
    default:
        Type = Types::INVALID;
    }

    other.Type = Types::INVALID;
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

Universal& Universal::operator=(Universal&& other)
{
    this->~Universal();

    new (this) Universal(std::move(other));

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

    throw parse_error("Runtime error: unexpected workflow.", {});
}

template <typename L, typename R>
struct Multiply
{
    static Universal Func(const L& l, const R& r)
    {
        return Universal(l * r);
    }
};

template <typename L, typename R>
struct Sum
{
    static Universal Func(const L& l, const R& r)
    {
        return Universal(l + r);
    }
};

template <typename L, typename R>
struct Subtract
{
    static Universal Func(const L& l, const R& r)
    {
        return Universal(l - r);
    }
};

template <typename L, typename R>
struct Divide
{
    static Universal Func(const L& l, const R& r)
    {
        if (std::is_same<L, int>::value && std::is_same<R, int>::value)
        {
            if (r != 0 && static_cast<const int&>(l) % static_cast<const int&>(r) == 0)
            {
                return Universal(l / r);
            }

            return Universal(l / double(r));
        }

        return Universal(l / r);
    }
};

template <typename L, typename R>
struct Power
{
    static Universal Func(const L& l, const R& r)
    {
        double v = std::pow(l, r);

        if (std::is_same<L, int>::value && std::is_same<R, int>::value && r > 0)
        {
            return Universal(static_cast<int>(v));
        }

        return Universal(v);
    }
};

template < template<typename L, typename R> class OP >
Universal BinOp(const Universal& l, const Universal& r)
{
    typedef decltype(Universal::Integer) Integer;
    typedef decltype(Universal::Real) Real;

    if (l.Type == Universal::Types::INTEGER)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            return OP<Integer, Integer>::Func(l.Integer, r.Integer);
        }
        else if (r.Type == Universal::Types::REAL)
        {
            return OP<Integer, Real>::Func(l.Integer, r.Real);
        }
    }
    else if (l.Type == Universal::Types::REAL)
    {
        if (r.Type == Universal::Types::INTEGER)
        {
            return OP<Real, Integer>::Func(l.Real, r.Integer);
        }
        else if (r.Type == Universal::Types::REAL)
        {
            return OP<Real, Real>::Func(l.Real, r.Real);
        }
    }

    throw parse_error("Runtime error: unexpected workflow.", {});
}

Universal Mul(const Universal& l, const Universal& r)
{
    return BinOp<Multiply>(l, r);
}

Universal Add(const Universal& l, const Universal& r)
{
    return BinOp<Sum>(l, r);
}

Universal Sub(const Universal& l, const Universal& r)
{
    return BinOp<Subtract>(l, r);
}

Universal Div(const Universal& l, const Universal& r)
{
    return BinOp<Divide>(l, r);
}

Universal Pow(const Universal& l, const Universal& r)
{
    return BinOp<Power>(l, r);
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

    throw parse_error("Runtime error: unexpected workflow.", {});
}

bool operator!=(const Universal& l, const Universal& r)
{
    return !(l == r);
}
}
