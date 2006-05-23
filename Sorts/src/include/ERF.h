#ifndef ERF_H
#define ERF_H

#include "GameObj.H"

class ERF {
public:
  virtual ~ERF() { };
  virtual double operator()(GameObj* gob) = 0;
  virtual double maxRadius() = 0;
};

#endif
