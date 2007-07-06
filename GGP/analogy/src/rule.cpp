#include <iostream>
#include "rule.h"

using namespace std;

void Rule::str() const {
  cout << *this << endl;
}

ostream& operator<<(ostream& os, const Rule& r) {
  os << r.head << "<=";
  for(vector<Condition>::const_iterator i = r.body.begin(); i != r.body.end(); ++i) {
    if (i->negated) {
      os << "!" << i->pred << ":";
    }
    else {
      os << i->pred << ":";
    }
  }
  return os;
}
