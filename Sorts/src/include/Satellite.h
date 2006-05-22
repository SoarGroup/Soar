#ifndef Satellite_H_
#define Satellite_H_

#include "Map.H"
#include "ScriptObj.H"
#include "GameObj.H"
#include "GameTile.H"
#include "Global.H"
#include "Vector.H"
#include "general.h"

class Sorts;

using namespace std;

class Satellite {
public:
  Satellite(); 
  ~Satellite();

  void init();

  sint4 addObject(GameObj *gob);
  sint4 updateObject(GameObj *gob, sint4 sat_loc);
  void removeObject(GameObj *gob, sint4 sat_loc);
  // querying functions
  std::set<GameObj*> *getObjectsInRegion(int x, int y);
  void getCollisions(sint4 x, sint4 y, sint4 r, list<GameObj*>& collisions);
 
  // for MineManager
  void addImaginaryWorker(coordinate c);
  bool hasMiningCollision(coordinate c);

  int refCount;

private:

  vector<set<GameObj*> > Map;
  vector<list<coordinate> > ImaginaryWorkers; // used by MineManager

  int tile_points;
  int width;
  int height;

};

#endif
