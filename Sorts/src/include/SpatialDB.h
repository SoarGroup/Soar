/*
    This file is part of Sorts, an interface between Soar and ORTS.
    (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

    Sorts is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Sorts is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sorts; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/
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
#include "GameConst.H"

#include "general.h"
#include "Rectangle.h"
#include "Circle.h"
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

  bool hasMiningCollision(coordinate c);

  // used by building locator
  bool hasTerrainCollision(Rectangle* r);
  bool hasTerrainCollision(Circle& c);
  bool hasObjectCollision(Rectangle* r);
  bool hasObjectCollision(coordinate c, int r, GameObj* ignoreGob);
 
  bool hasObjectCollision(sint4 x, sint4 y, sint4 r);

  void addImaginaryWorker(coordinate c);
  void addImaginaryObstacle(coordinate c);
  
  bool hasImaginaryTerrainObstacleCollision(Circle& c);
  void addTerrainLine(Line l);

//  void addTerrainContour(TerrainContour* c);
//  void removeTerrainContour(TerrainContour* c);
//  void updateTerrainContour(TerrainContour* c);

  // return a list of the N closest objects with the given type to
  // coordinate.
  list<GameObj*> getNClosest(coordinate point, int N, string type);

  int refCount;

private: // functions

  bool hasObjectCollisionInt(coordinate, int, bool, GameObj*);
  void calcBinning (sint4 x, sint4 y, sint4 r, ERF* erf, BinInfo& info);

private:
  vector<set<GameObj*> >    gobMap;
  vector<list<coordinate> > imaginaryWorkerMap; // used by MineManager
  vector<list<coordinate> > imaginaryObstacleMap;

  const Map<GameTile>* gameMap;

  int getCellNumber(int x, int y);
  int cell2row(int);
  int cell2column(int);
  int rowCol2cell(int, int);
  int sdbTilePoints;
  int width;
  int height;

};

#endif
