#ifndef SpatialDB_H_
#define SpatialDB_H_

#include "Map.H"
#include "ScriptObj.H"
#include "GameObj.H"
#include "GameTile.H"
#include "Global.H"
#include "Vector.H"
#include "general.h"
#include "Rectangle.h"

#include "ERF.h"

class Sorts;

using namespace std;

class SpatialDB {
public:
  SpatialDB(); 
  ~SpatialDB();

  void init();

  sint4 addObject(GameObj *gob);
  sint4 updateObject(GameObj *gob, sint4 sat_loc);
  void  removeObject(GameObj *gob, sint4 sat_loc);

  // querying functions
  set<GameObj*> *getObjectsInRegion(int x, int y);
  void getCollisions(sint4 x, sint4 y, sint4 r, ERF* erf, list<GameObj*>& collisions);
  bool hasMiningCollision(coordinate c, bool b);

  // used by building locator
  bool hasTerrainCollision(Rectangle* r);
  bool hasObjectCollision(Rectangle* r);
 
  void addImaginaryWorker(coordinate c);

  void addTerrainLine(line l);

  int refCount;

private:

  bool hasObjectCollisionInt(coordinate, int, bool, bool);
  
  vector<set<GameObj*> > gobMap;
  vector<list<coordinate> > imaginaryWorkerMap; // used by MineManager
  vector<list<line> > terrainLineMap;

  int getCellNumber(int x, int y);
  int cell2row(int);
  int cell2column(int);
  int colRow2cell(int, int);
  int tile_points;
  int width;
  int height;

};

#endif
