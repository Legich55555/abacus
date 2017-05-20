  #include "Universal.h"

  #include <cmath>
  #include <cassert>

  namespace ExprCalc
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
              case Types::VARIABLE:
                  Variable = other.Variable; 
                  Type = other.Type;
                  break;
              case Types::INT_SEQUENCE:
                  IntSequence = other.IntSequence; 
                  Type = other.Type;
                  break;
              case Types::REAL_SEQUENCE:
                  RealSequence = other.RealSequence; 
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
              case Types::VARIABLE:
                  Variable.~string(); 
                  break;
              case Types::INT_SEQUENCE:
                  IntSequence.~vector<int>(); 
                  break;
              case Types::REAL_SEQUENCE:
                  RealSequence.~vector<float>(); 
                  break;
              default:
                  break;
          }
          
          Type = Types::INVALID;
      }
      
      Universal& Universal::operator=(const Universal& other)
      {
          this->~Universal();
          
          switch(other.Type)
          {
              case Types::INTEGER:
                  new (this) Universal(other.Integer);
                  break;
              case Types::REAL:
                  new (this) Universal(other.Real);
                  break;
              case Types::VARIABLE:
                  new (this) Universal(other.Variable);
                  break;
              case Types::INT_SEQUENCE:
                  new (this) Universal(other.IntSequence);
                  break;
              case Types::REAL_SEQUENCE:
                  new (this) Universal(other.RealSequence);
                  break;
              default:
                  Type = Types::INVALID;
                  throw 1;
          }
          
          return *this;
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
                  return Universal(l.Integer * r.Integer);
              }
              else if (r.Type == Universal::Types::REAL)
              {
                  return Universal(l.Integer * r.Real);
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
                  return Universal(l.Integer - r.Integer);
              }
              else if (r.Type == Universal::Types::REAL)
              {
                  return Universal(l.Integer - r.Real);
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
                  return Universal(l.Integer / r.Integer);
              }
              else if (r.Type == Universal::Types::REAL)
              {
                  return Universal(l.Integer / r.Real);
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
                  return Universal(static_cast<decltype(l.Integer)>(std::pow(l.Integer, r.Integer)));
              }
              else if (r.Type == Universal::Types::REAL)
              {
                  return Universal(static_cast<float>(std::pow(l.Integer, r.Real)));
              }
          }
          else if (l.Type == Universal::Types::REAL)
          {
              if (r.Type == Universal::Types::INTEGER)
              {
                  return Universal(static_cast<float>(std::pow(l.Real, r.Integer)));
              }
              else if (r.Type == Universal::Types::REAL)
              {
                  return Universal(static_cast<float>(std::pow(l.Real, r.Real)));
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

        if (l.Type == Universal::Types::VARIABLE)
        {
          return l.Variable == r.Variable;
        }

        assert(false);

        return false;
    }
}
