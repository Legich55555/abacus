# pragma once

#include "ExprCalc.h"
#include "Universal.h"

#include <cstdio>
#include <string>
#include <vector>
#include <typeinfo>

#include <tao/pegtl.hpp>

namespace Abacus
{

  using namespace tao::TAOCPP_PEGTL_NAMESPACE;

  template<typename ... Args>
  std::string Print(const char* format, Args ... args)
  {
    size_t msgSize = snprintf(nullptr, 0, format, args ... ) + 1U;
    std::string msg(msgSize, ' ');
    snprintf(const_cast<char*>(msg.data()), msgSize, format, args ... );
    return msg;
  }

  struct TerminatedError { };

  template< char C, typename Input >
  void ExpectChar(Input& input)
  {
    if (!parse< seq< one<C>, star<space> > >(input))
    {
      throw parse_error(Print("Invalid syntax. Expected '%c'", C), input);
    }
  }

  template< typename Input >
  void ExpectComma(Input& input)
  {
    ExpectChar<','>(input);
  }

  template< typename Input >
  void ExpectArrow(Input& input)
  {
    if (!parse< seq< string<'-','>' >, star<space> >>(input))
    {
      throw parse_error("Invalid syntax. Expected '->'.", input);
    }
  }

  template< typename Input >
  void ExpectClosingBracket(Input& input)
  {
    ExpectChar<')'>(input);
  }

  template<typename Rule>
  struct IdentifierAction : nothing<Rule> { };

  template<>
  struct IdentifierAction<identifier>
  {
    template< typename Input >
    static void apply(const Input& input, std::string& name)
    {
      name = input.string();
    }
  };

  template< typename Input >
  std::string ExpectIdentifier(Input& input)
  {
    std::string name;
    if (!parse< seq< identifier, star<space> >, IdentifierAction >(input, name))
    {
      throw parse_error("Invalid map() syntax. Expected lambda parameter", input);
    }

    return name;
  }

  template<typename T>
  const T& GetValue(const Universal& u)
  {
    static_assert(
          std::is_same<T, int>::value ||
          std::is_same<T, double>::value ||
          std::is_same<T, Universal::IntArray>::value ||
          std::is_same<T, Universal::RealArray>::value,
          "Invalid data type.");

    if (std::is_same<T, int>::value && Universal::Types::INTEGER == u.Type)
    {
      return u.Integer;
    }
    else if (std::is_same<T, double>::value && Universal::Types::REAL == u.Type)
    {
      return u.Real;
    }
    else if (std::is_same<T, Universal::IntArray>::value && Universal::Types::INT_SEQUENCE == u.Type)
    {
      return u.IntSequence;
    }
    else if (std::is_same<T, Universal::RealArray>::value && Universal::Types::REAL_SEQUENCE == u.Type)
    {
      return u.RealSequence;
    }

    throw parse_error(
          Print("Internal error: invalid call for GetValue(). Requested: %s, actual: %u",
                typeid(T).name(), static_cast<unsigned>(u.Type)),
    { });
  }

  template<typename T>
  T GetNumber(const Universal& u)
  {
    static_assert(
          std::is_same<T, int>::value ||
          std::is_same<T, double>::value,
          "Invalid data type.");

    if (std::is_same<T, int>::value && Universal::Types::INTEGER == u.Type)
    {
      return u.Integer;
    }
    else if (std::is_same<T, double>::value && Universal::Types::REAL == u.Type)
    {
      return u.Real;
    }

    throw parse_error(Print("Internal error: invalid call for GetNumber(). Requested: %s, actual: %u",
                            typeid(T).name(), static_cast<unsigned>(u.Type)),
    { });
  }

}
