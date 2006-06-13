#ifndef SpatialDB_H_
#define SpatialDB_H_

#include <vector>
#include <list>

#include "Map.H"
#include "ScriptObj.H"
#include "GameObj.H"
#include "GameTile.H"
#include "Global.H"
#include "Vector.H"

#include "general.h"
#include "Rectangle.h"
#include "TerrainContour.h"
#include "ERF.h"

class Sorts;

using namespace std;

struct BinInfo {
  int  binSize;
  int  binWidth;
  int  binTilePoints;
  int  bins[9];
  bool check[9];
};

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

  void getObjectCollisions
  ( sint4 x, 
    sint4 y, 
    sint4 r, 
    ERF* erf, 
    list<GameObj*>& objCol);

  bool hasObjectCollision(sint4 x, sint4 y, sint4 r, ERF* erf);

  bool hasMiningCollision(coordinate c, bool b);

  // used by building locator
  bool hasTerrainCollision(Rectangle& r);
  bool hasTerrainCollision(int cx, int cy, int r);
  void getTerrainCollisions(Rectangle&, list<TerrainContour*>&);
  void getTerrainCollisions(int, int,  int, list<TerrainContour*>&);

  bool hasObjectCollision(Rectangle* r);
  bool hasObjectCollision(coordinate c, int r, GameObj* ignoreGob);
 
  bool hasObjectCollision(sint4 x, sint4 y, sint4 r);

  void addImaginaryWorker(coordinate c);

  void addTerrainContour(TerrainContour* c);
  void removeTerrainContour(TerrainContour* c);
  void updateTerrainContour(TerrainContour* c);

  int refCount;

private: // functions

  bool hasObjectCollisionInt(coordinate, int, bool, bool, GameObj*);
  void calcBinning (sint4 x, sint4 y, sint4 r, ERF* erf, BinInfo& info);

private:
  vector<set<GameObj*> > gobMap;
  vector<list<coordinate> > imaginaryWorkerMap; // used by MineManager

  map<TerrainContour*, list<int> > contourLocs;
  vector<list<TerrainContour*> > contours;

  int getCellNumber(int x, int y);
  int cell2row(int);
  int cell2column(int);
  int rowCol2cell(int, int);
  int tile_points;
  int width;
  int height;

};

#endif
