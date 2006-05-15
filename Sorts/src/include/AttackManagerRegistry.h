#ifndef AttackManagerRegistry_H
#define AttackManagerRegistry_H

#include <vector>
#include <set>

#include "PerceptualGroup.h"
#include "AttackManager.h"

using namespace std;

class AttackManagerRegistry {
public:
  AttackManagerRegistry() : uid(0) { }
  
  int assignManager(const list<PerceptualGroup*>& targets);

  AttackManager* getManager(int id);

  void removeManager(AttackManager* m);

private:
  typedef struct {
    int                   id;
    AttackManager*        manager;
  } ManagerRecord;

  int uid;

  list<ManagerRecord> managers;
};

#endif
