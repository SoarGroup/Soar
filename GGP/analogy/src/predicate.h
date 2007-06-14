#ifndef __PREDICATE_H__
#define __PREDICATE_H__

#include <iostream>
#include <assert.h>

using namespace std;

enum PredicateType { PERSISTENT, ENTAILMENT, MOVE, FUNCTION };

class Predicate {

  public:
    Predicate(string n, int a, PredicateType t) : name(n), arity(a), type(t) {
      if (t != PERSISTENT and t != ENTAILMENT and t != MOVE and t != FUNCTION) {
        assert(false);
      }
    }

    Predicate(const Predicate& other) 
      : name(other.name), arity(other.arity), type(other.type)
    {}

    int           get_arity() const { return arity; }
    string        get_name()  const { return name; }
    PredicateType get_type()  const { return type; }

    bool operator<(const Predicate& other) const {
      if (name != other.name) {
        return name < other.name;
      }
      assert(arity == other.arity && type == other.type);
      return true;
    }

    bool operator==(const Predicate& other) const {
      return name == other.name && 
             arity == other.arity && 
             type == other.type;
    }

    bool operator!=(const Predicate& other) const {
      return name != other.name || 
             arity != other.arity || 
             type != other.type;
    }
    
    friend ostream& operator<<(ostream& os, const Predicate& p);

  private:
    string name;
    int arity;
    PredicateType type;
};

#endif
