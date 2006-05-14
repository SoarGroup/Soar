#ifndef AttackManagerRegistry_H
#define AttackManagerRegistry_H

#include <vector>

#include "AttackManager.h"

using namespace std;

class AttackManagerRegistry {
public:
  AttackManagerRegistry() { }
  
  int add(AttackManager* m) {
    managers.push_back(m);
    return managers.size() - 1;
  }

  AttackManager* get(int i) {
    return managers[i];
  }

private:
  vector<AttackManager*> managers;
};

#endif
