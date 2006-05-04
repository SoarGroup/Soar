#ifndef Satellite_H_
#define Satellite_H_

#include "Map.H"
#include "ScriptObj.H"
#include "GameObj.H"
#include "GameTile.H"
#include "Global.H"
#include "Vector.H"

class Sorts;

using namespace std;

class Satellite {
public:
  Satellite(); 
  ~Satellite();

  void setSorts(const Sorts *);

  // querying functions
  std::list<GameObj*> *getObjectsInRegion(int x, int y);

private:

  const Sorts* sorts;

  std::vector<std::list<GameObj*> > Map;

  int tile_points;
  int width;
  int height;

};

#endif
