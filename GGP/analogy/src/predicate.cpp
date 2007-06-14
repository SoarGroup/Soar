#include <iostream>
#include <assert.h>
#include "predicate.h"

using namespace std;

ostream& operator<<(ostream& os, const Predicate& p) {
  os << p.name << "("; 
  switch (p.type) {
    case PERSISTENT:
      os << "P,";
      break;
    case ENTAILMENT:
      os << "E,";
      break;
    case MOVE:
      os << "M,";
      break;
    case FUNCTION:
      os << "F,";
      break;
    default:
      assert(false);
  }
  os << p.arity << ")";
  return os;
}
