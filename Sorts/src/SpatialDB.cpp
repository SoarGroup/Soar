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
#include "SpatialDB.h"
#include "Sorts.h"
#include "general.h"
#include "SortsCollision.h"

#include <map>
#include <limits>

#define WORKER_RADIUS 3
#define IMAG_OBSTACLE_RADIUS 8 

#define WORKER_CROWD_FACTOR 4
#define CROWD_MAX_MINERALS 8
#define CROWD_MAX_WORKERS 2

#define CLASS_TOKEN "SDB"
#define DEBUG_OUTPUT false 
#include "OutputDefinitionsUnique.h"

enum { TILE_UNKNOWN=0, TILE_WATER=1, TILE_GROUND=2, TILE_CLIFF=3 };

inline int intDivC(int x, int y) {
  return (int) ceilf(((float) x) / y);
}

SpatialDB::SpatialDB() {
  width = 0;
  height = 0;
}

void SpatialDB::init() {
  sdbTilePoints = 50;
  width = intDivC(Sorts::OrtsIO->getMapXDim(), sdbTilePoints);
  height = intDivC(Sorts::OrtsIO->getMapYDim(), sdbTilePoints);

  int mapsize = static_cast<int>(width*height);
  // Tilepoints define the granularity of the grid... 
  // The higher the tilepoints, the coarser the detail
  gobMap.resize(mapsize);
  imaginaryWorkerMap.resize(mapsize);
  msg  << "initializing SpatialDB grid: (" << width<<"," << height
       << "," << (width*height) << ")\n";

  gameMap = &Sorts::OrtsIO->getGameMap();
}

SpatialDB::~SpatialDB()
{ }

std::set<GameObj*> *SpatialDB::getObjectsInRegion(int x, int y)
{
 return &gobMap[getCellNumber(x,y)];
}


sint4 SpatialDB::addObject(GameObj *gob) {
  sint4 x = *(gob->sod.x);
  sint4 y = *(gob->sod.y);
  int cell = getCellNumber(x,y);
  if (cell >= gobMap.size()) {
    msg << "ERROR: out of bounds, not adding: " << x << "," << y << endl;
    return -1; 
  }
  
  gobMap[cell].insert(gob); 
  msg << "registered new object: " << gob->bp_name() << " gob " << gob << "\n";

  return cell;
}

void SpatialDB::addImaginaryWorker(coordinate c) {
  // no need for removal/update support
  int cell = getCellNumber(c.x, c.y);
  if (cell >= gobMap.size()) {
    msg << "ERROR: out of bounds, not adding: " << c.x << "," << c.y << endl;
    return; 
  }
  
  imaginaryWorkerMap[cell].push_back(c); 
  dbg << "registered new imaginary worker.\n";
  dbg << "iw loc: " << c.x << "," << c.y << endl;
}

int SpatialDB::getCellNumber(int x, int y) {
  return ((int)(y / sdbTilePoints)) * width + (int)(x / sdbTilePoints);
}

int SpatialDB::cell2row(int cellNum) {
  return (int)(cellNum / width);
}

int SpatialDB::cell2column(int cellNum) {
  return (cellNum % width);
}

int SpatialDB::rowCol2cell(int row, int col) {
  return (col + row*width);
}

sint4 SpatialDB::updateObject(GameObj *gob, sint4 sat_loc)
{
 sint4 x = *(gob->sod.x);
 sint4 y = *(gob->sod.y);
 sint4 new_sat_loc = getCellNumber(x,y);

 if (x < 0 || y < 0) {
   msg << "object is not on the map (this happens with under-construction builders)."
       << "placing in cell 0.\n";
   new_sat_loc = 0;
 }

 assert(new_sat_loc >= 0);
 assert(new_sat_loc < gobMap.size());
 
 if(sat_loc != new_sat_loc)
 {
  gobMap[sat_loc].erase(gob);
  gobMap[new_sat_loc].insert(gob);
 }
 else
  new_sat_loc = sat_loc;

 return new_sat_loc;
}

void SpatialDB::removeObject(GameObj *gob, sint4 sat_loc) {
  if (sat_loc < 0) return;
  gobMap[sat_loc].erase(gob);
}

void SpatialDB::calcBinning
( sint4 x, 
  sint4 y, 
  sint4 r, 
  ERF* erf, 
  BinInfo& info )
{
  // how many cells to bin together to get the size of a bin
  // to be bigger than the max expected radius
  int cr; // collision radius
  if (erf == NULL) {
    cr = r + intDivC(sdbTilePoints, 2); 
    // sdbTilePoints being the upperbound of the radii of other objects
  }
  else {
    cr = r + (int) erf->maxRadius();
  }
  int binSize = intDivC(cr, sdbTilePoints);
  int binTilePoints = binSize * sdbTilePoints;
  int binWidth = intDivC(Sorts::OrtsIO->getMapXDim(), binTilePoints);
  assert (binWidth == intDivC(width, binSize));

  for(int i = 0; i < 9; i++) {
    info.check[i] = false;
  }

  //1. Figure out which bins surround the target location 
  info.bins[0] = (y-cr)/binTilePoints*binWidth+(x-cr)/binTilePoints;
  info.bins[1]=(y-cr)/binTilePoints*binWidth+x/binTilePoints;
  info.bins[2]=(y-cr)/binTilePoints*binWidth+(x+cr)/binTilePoints;
  info.bins[3]=y/binTilePoints*binWidth+(x-cr)/binTilePoints;
  info.bins[4]=y/binTilePoints*binWidth+x/binTilePoints;
  info.bins[5]=y/binTilePoints*binWidth+(x+cr)/binTilePoints;
  info.bins[6]=(y+cr)/binTilePoints*binWidth+(x-cr)/binTilePoints;
  info.bins[7]=(y+cr)/binTilePoints*binWidth+x/binTilePoints;
  info.bins[8]=(y+cr)/binTilePoints*binWidth+(x+cr)/binTilePoints;
  
  //2. Figure out which bins are under (or partially under) the circle
  // 0 1 2
  // 3 4 5
  // 6 7 8

  //Center cell
  info.check[4] = true; 
  //North
  if(info.bins[4] != info.bins[1])
   info.check[1] = true;  
  //East
  if(info.bins[4] != info.bins[5])
    info.check[5] = true;
  //NorthEast
  info.check[2] = info.check[1] && info.check[5];
  //South
  if(info.bins[4] != info.bins[7])
    info.check[7] = true;
  //SouthEast
  info.check[8] = info.check[5] && info.check[7];
  //West
  if(info.bins[4] != info.bins[3])
    info.check[3]= true;
  //SourhWest
  info.check[6] = info.check[7] && info.check[3];
  //NorthWest
  info.check[0] = info.check[3] && info.check[1];

  //Make sure we the check are inside the map
  //Left Side
  if(x < cr)
    info.check[0] = info.check[3] = info.check[6] = false;
  //Right Side
  if(x+cr > Sorts::OrtsIO->getMapXDim())
    info.check[2] = info.check[5] = info.check[8] = false;
  //Top Side
  if(y < cr)
    info.check[0] = info.check[1] = info.check[2] = false;
  //Bottom Side
  if(y+cr > Sorts::OrtsIO->getMapYDim())
    info.check[6] = info.check[7] = info.check[8] = false;

  info.binSize = binSize;
  info.binWidth = binWidth;
  info.binTilePoints = binTilePoints;
}

void SpatialDB::getObjectCollisions
( sint4 x, 
  sint4 y, 
  sint4 r, 
  ERF* erf, 
  list<GameObj*>& objCol)
{
  objCol.clear();

  BinInfo info;
  calcBinning(x, y, r, erf, info);

  // For each marked cell, check all objects inside of it 
  // for collisions with the circle
  //map<GameObj*, int> whereFound;
  std::set<GameObj*>::iterator it;
  for(int i=0; i<9; i++) {
    if(info.check[i]) {
      int cellStartRow = (info.bins[i] / info.binWidth) * info.binSize;
      int cellStartCol = (info.bins[i] % info.binWidth) * info.binSize;
      for(int j = cellStartRow; j < cellStartRow + info.binSize; j++) {
        if (j >= height) { break; }
        for(int k = cellStartCol; k < cellStartCol + info.binSize; k++) {
          if (k >= width) { break; }

          int cell = j * width + k;
          for(it = gobMap[cell].begin(); it != gobMap[cell].end(); it++) {
            int objx =  *(*it)->sod.x;
            int objy =  *(*it)->sod.y;
            int objr;
            if (erf == NULL) {
              objr =  (*(*it)->sod.radius);
            }
            else {
              objr = (int) (*erf)(*it);
              if (objr < 0) {
                continue;
              }
            }
            if((x-objx)*(x-objx)+(y-objy)*(y-objy) < (r+objr)*(r+objr)) {
              //Inside the circle
              objCol.push_back((*it));
              /*
              if (whereFound.find(*it) != whereFound.end()) {
                msg << "Object found previously in " << whereFound[*it]
                    << "is now found at " << cell << endl;
                assert(false);
              }
              else {
                whereFound[*it] = cell;
              }
              */
            } 
          }
        }
      }
    }
  }
}

bool SpatialDB::hasObjectCollision
( sint4 x, 
  sint4 y, 
  sint4 r, 
  ERF* erf)
{
  BinInfo info;
  calcBinning(x, y, r, erf, info);

  // For each marked cell, check all objects inside of it 
  // for collisions with the circle
  std::set<GameObj*>::iterator it;
  for(int i=0; i<9; i++) {
    if(info.check[i]) {
      int cellStartRow = (info.bins[i] / info.binWidth) * info.binSize;
      int cellStartCol = (info.bins[i] % info.binWidth) * info.binSize;
      for(int j = cellStartRow; j < cellStartRow + info.binSize; j++) {
        if (j >= height) { break; }
        for(int k = cellStartCol; k < cellStartCol + info.binSize; k++) {
          if (k >= width) { break; }

          int cell = j * width + k;
          for(it = gobMap[cell].begin(); it != gobMap[cell].end(); it++) {
            int objx = *(*it)->sod.x;
            int objy = *(*it)->sod.y;
            int objr;
            if (erf == NULL) {
              objr =  (*(*it)->sod.radius);
            }
            else {
              objr = (int) (*erf)(*it);
              if (objr < 0) {
                continue;
              }
            }
            if((x-objx)*(x-objx)+(y-objy)*(y-objy) < (r+objr)*(r+objr)) {
              return true;
            } 
          }
        }
      }
    }
  }
  return false;
}

bool SpatialDB::hasObjectCollision(coordinate c, int r, GameObj* gob) {
  // ignore collisions with gob
  return hasObjectCollisionInt(c, r, false,  gob);
}

bool SpatialDB::hasObjectCollision(sint4 x, sint4 y, sint4 r) {
  coordinate c(x,y);
  return hasObjectCollisionInt(c, r, false, NULL);
}

bool SpatialDB::hasMiningCollision(coordinate c) {
  return hasObjectCollisionInt(c, WORKER_RADIUS+1, true,
                               NULL);
}

bool SpatialDB::hasObjectCollision(Rectangle* rect) {
  // overestimate as a circle
  coordinate c;
  c.x = (int)((rect->xmax + rect->xmin)/2.0);
  c.y = (int)((rect->ymax + rect->ymin)/2.0);
  int radius = (int)(sqrt(squaredDistance(c.x, c.y, rect->xmax, rect->ymax)));
  if (hasObjectCollisionInt(c, radius, false, NULL)) {
    dbg << "object collision!\n";
    return true;
  }
  else {
    dbg << "no object collision.\n";
    return false;
  }
}

bool SpatialDB::hasObjectCollisionInt
( coordinate c, 
  int        radius, 
  bool       forMining, 
  GameObj*   ignoreGob ) 
{
  int cells[9];
  bool check[9] = {false};

  sint4 bigR;
  
  bigR = sdbTilePoints;
  
  sint4 x = c.x;
  sint4 y = c.y;

   //1. Figure out which cells surround the target location 
  cells[0] = getCellNumber(x-bigR, y-bigR);
  cells[1] = getCellNumber(x, y-bigR);
  cells[2] = getCellNumber(x+bigR, y-bigR);
  cells[3] = getCellNumber(x-bigR, y);
  cells[4] = getCellNumber(x, y);
  cells[5] = getCellNumber(x+bigR, y);
  cells[6] = getCellNumber(x-bigR, y+bigR);
  cells[7] = getCellNumber(x, y+bigR);
  cells[8] = getCellNumber(x+bigR, y+bigR);
  
  //2. Figure out which cells are under (or partially under) the circle
  // 0 1 2
  // 3 4 5
  // 6 7 8

  //Center cell
  check[4] = true; 
  //North
  if(cells[4] != cells[1])
    check[1] = true;  
  //East
  if(cells[4] != cells[5])
    check[5] = true;
  //NorthEast
  check[2] = check[1] && check[5];
  //South
  if(cells[4] != cells[7])
    check[7] = true;
  //SouthEasr
  check[8] = check[5] && check[7];
  //West
  if(cells[4] != cells[3])
    check[3]= true;
  //SouthWest
  check[6] = check[7] && check[3];
  //NorthWest
  check[0] = check[3] && check[1];
  
  //Make sure we the check are inside the map
  //Left Side
  if((x-bigR)<0)
    check[0] = check[3] = check[6] = false;
  //Right Side
  if((x+bigR)>Sorts::OrtsIO->getMapXDim())
    check[2] = check[5] = check[8] = false;
  //Top Side
  if((y-bigR)<0)
    check[0] = check[1] = check[2] = false;
  //Bottom Side
  if((y+bigR)>Sorts::OrtsIO->getMapYDim())
    check[6] = check[7] = check[8] = false;

  
  //3. For each marked cell, check all object inside of it for collisions with the circle
  TerrainBase::Loc obj;
  sint4 objr;
  set<GameObj*>::iterator it;
  list<coordinate>::iterator iwit;

  Circle circle(x,y,radius);
 
  for(int i=0; i<9; i++) {
    if(check[i]) {
  //    msg << "checking cell " << cells[i] << "\n";
      assert(cells[i] < gobMap.size());
      for(it = gobMap[cells[i]].begin(); it != gobMap[cells[i]].end(); it++) {
        obj.x =  (*(*it)->sod.x);
        obj.y =  (*(*it)->sod.y);
        objr =  (*(*it)->sod.radius);
        if (*(*it)->sod.shape != SHAPE_RECTANGLE) {
          if((x-obj.x) * (x-obj.x) + (y-obj.y) * (y-obj.y) 
          < (radius+objr) * (radius+objr))  {
            // inside the circle
            if (forMining) {
              if (((*it)->bp_name() != "worker") 
                  and
                  ((*it)->bp_name() != "sheep")
                  ) {
                //msg << "mining collision with " << (*it)->bp_name() << endl;
                //msg << "at loc " << obj.x << "," << obj.y << endl;
                return true;
              }
            }
            else {
              if ((*it) != ignoreGob) {
                dbg << "object collision with " << (*it)->bp_name() << endl;
                dbg << "at loc " << obj.x << "," << obj.y << endl;
                return true;
              }
            } 
          }
        }
        else {
          // rectangle
          Rectangle r(*(*it)->sod.x1, *(*it)->sod.x2, 
                      *(*it)->sod.y1, *(*it)->sod.y2);
          if (forMining) {
            if (rectangle_circle_intersect(r, circle) and 
                (*it)->bp_name() != "controlCenter")
            {
              return true;
            }
          }
          else if (rectangle_circle_intersect(r, circle) and 
                   (*it) != ignoreGob)
          {
            dbg << "object at " << c << " with radius " << radius 
                 << " has collision with " << (*it)->bp_name() << " at " <<
               *(*it)->sod.x << "," << *(*it)->sod.y << endl;
            return true;
          }
         
        }
      }
      if (forMining) {
        for(iwit = imaginaryWorkerMap[cells[i]].begin(); 
            iwit != imaginaryWorkerMap[cells[i]].end(); 
            iwit++) {
          obj.x = (*iwit).x;
          obj.y = (*iwit).y;
          objr = WORKER_RADIUS + 1; // radius of the imaginary worker
          if((x-obj.x) * (x-obj.x) + (y-obj.y) * (y-obj.y) 
          < (radius+objr) * (radius+objr))  {
            //Inside the circle
            msg << "imaginary worker collision!\n";
            return true;
          }
        }
      }
    }
  }

  // check for terrain collisions
  /*
  Circle cc(c.x, c.y, radius);
  if (hasTerrainCollision(cc)) {
    Sorts::canvas.makeTempCircle(c.x, c.y, radius, 9999)->setShapeColor(255, 255, 255);
    return true;
  }*/

 // msg << "object at " << c << " with radius " << radius 
 //     << " has no collisions\n"; 
  return false; // no collisions
}

bool SpatialDB::hasTerrainCollision(Rectangle *rect) {
  int minCol = rect->xmin / GameConst::TILE_POINTS;
  int maxCol = rect->xmax / GameConst::TILE_POINTS;
  int minRow = rect->ymin / GameConst::TILE_POINTS;
  int maxRow = rect->ymax / GameConst::TILE_POINTS;

  if (minCol < 0 || 
      maxCol >= Sorts::OrtsIO->getMapXDim() / GameConst::TILE_POINTS ||
      minRow < 0 ||
      maxRow >= Sorts::OrtsIO->getMapYDim() / GameConst::TILE_POINTS)
  {
    // collision with edge of map
    dbg << "eom collision!\n";
    return true;
  }

  for(int r = minRow; r <= maxRow; ++r) {
    for(int c = minCol; c <= maxCol; ++c) {

      const GameTile& t = (*gameMap)(c, r);
      Tile::Split split = t.get_split();
      Tile::Type typeN = t.get_typeN();
      Tile::Type typeS = t.get_typeS();
      Tile::Type typeW = t.get_typeW();
      Tile::Type typeE = t.get_typeE();

      int xmin = c * GameConst::TILE_POINTS;
      int xmax = xmin + GameConst::TILE_POINTS;
      int ymin = r * GameConst::TILE_POINTS;
      int ymax = ymin + GameConst::TILE_POINTS;

      Vec2d rAngle;
      int xoffset, yoffset;

      switch (split) {
        case Tile::NO_SPLIT:
          // just check if entire tile is not passable
          if (typeW != TILE_GROUND and
              typeW != TILE_UNKNOWN) {
            dbg << "non-ground!\n";
            return true;
          }
          else {
            continue;
          }
        break;

        case Tile::TB_SPLIT:
          if (typeW != TILE_GROUND) {
            rAngle.setB(xmin, ymax);
            xoffset = xmax - xmin;
            yoffset = ymin - ymax;
          }
          if (typeE != TILE_GROUND) {
            rAngle.setB(xmax, ymin);
            xoffset = xmin - xmax;
            yoffset = ymax - ymin;
          }
        break;

        case Tile::BT_SPLIT:
          if (typeW != TILE_GROUND) {
            rAngle.setB(xmin, ymin);
            xoffset = xmax - xmin;
            yoffset = ymax - ymin;
          }
          if (typeE != TILE_GROUND) {
            rAngle.setB(xmax, ymax);
            xoffset = xmin - xmax;
            yoffset = ymin - ymax;
          }
        break;
      }
      if (rectangle_triangle_intersect(*rect, rAngle, xoffset, yoffset)) {
        dbg << "rti\n";
        return true;
      }
    }
  }

  dbg << "no terrain collision.\n";
  return false;
}


bool SpatialDB::hasTerrainCollision(Circle& c) {
  int minCol = (c.x - c.r) / GameConst::TILE_POINTS;
  int maxCol = (c.x + c.r) / GameConst::TILE_POINTS;
  int minRow = (c.y - c.r) / GameConst::TILE_POINTS;
  int maxRow = (c.y + c.r) / GameConst::TILE_POINTS;
/*
  if (minCol < 0 || 
      maxCol >= Sorts::OrtsIO->getMapXDim() / GameConst::TILE_POINTS ||
      minRow < 0 ||
      maxRow >= Sorts::OrtsIO->getMapYDim() / GameConst::TILE_POINTS)
  {
    // collision with edge of map
    return true;
  }
*/
  if (c.x - c.r < 0 ||
      c.y - c.r < 0 || 
      c.x + c.r > Sorts::OrtsIO->getMapXDim() || 
      c.y + c.r > Sorts::OrtsIO->getMapYDim()) {
    dbg << "edge of map collision.\n";
    return true;
  }
  for(int row = minRow; row <= maxRow; ++row) {
    for(int col = minCol; col <= maxCol; ++col) {
      const GameTile& t = gameMap->get_tile(col, row);
      int height        = t.get_min_h();
      Tile::Split split = t.get_split();
      Tile::Type typeN  = t.get_typeN();
      Tile::Type typeS  = t.get_typeS();
      Tile::Type typeW  = t.get_typeW();
      Tile::Type typeE  = t.get_typeE();

      int xmin = col * GameConst::TILE_POINTS;
      int xmax = xmin + GameConst::TILE_POINTS;
      int ymin = row * GameConst::TILE_POINTS;
      int ymax = ymin + GameConst::TILE_POINTS;

      Rectangle r(xmin, xmax, ymin, ymax);
      
      if (rectangle_circle_intersect(r, c)) {
        if (height > 0) {
          // assume all plateaus are closed. THIS IS DEFINITELY NOT THE CASE!
          dbg << "TERRAIN COLLISION" << endl;
          return true;
        }
      }

      Vec2d rAngle;
      double xoffset, yoffset;
      if(split == Tile::NO_SPLIT) {
        if (typeW == TILE_GROUND || typeW == TILE_UNKNOWN) { 
          continue; 
        }
        else {
          if (rectangle_circle_intersect(r, c)) { return true; }
        }
      }
      else {
        if(split == Tile::TB_SPLIT) {
          if (typeW != TILE_GROUND || typeW == TILE_UNKNOWN) {
            rAngle.setB(xmin, ymax);
            xoffset = xmax - xmin;
            yoffset = ymin - ymax;
          }
          if (typeE != TILE_GROUND || typeW == TILE_UNKNOWN) {
            rAngle.setB(xmax, ymin);
            xoffset = xmin - xmax;
            yoffset = ymax - ymin;
          }
        }
        else { // BT_SPLIT
          if (typeW != TILE_GROUND || typeW == TILE_UNKNOWN) {
            rAngle.setB(xmin, ymin);
            xoffset = xmax - xmin;
            yoffset = ymax - ymin;
          }
          if (typeE != TILE_GROUND || typeW == TILE_UNKNOWN) {
            rAngle.setB(xmax, ymax);
            xoffset = xmin - xmax;
            yoffset = ymin - ymax;
          }
        }
        if (circle_triangle_intersect(c, rAngle, xoffset, yoffset)) {
          dbg << "split collision.\n";
          return true;
        }
      }
    }
  }
  return false;
}

struct ltGobDouble {
  bool operator()(pair<GameObj*, double> p1, pair<GameObj*, double> p2) const {
    return (p1.second < p2.second);
  }
};

list<GameObj*> SpatialDB::getNClosest(coordinate point, int N, string type) {
  // this is expensive, just iterate through the entire DB and calc all
  // distances.
  dbg << "getNClosest..\n";

  set<pair<GameObj*, double>, ltGobDouble> found;
  double dist;
  pair<GameObj*, double> tempPair;
  
  for (int i=0; i<(int)(width*height); i++) {
    for (set<GameObj*>::iterator it = gobMap[i].begin();
        it != gobMap[i].end();
        it++) {
      if ((*it)->bp_name() == type) {
        dist = squaredDistance(point.x, point.y, gobX(*it), gobY(*it));
        tempPair.first = *it;
        tempPair.second = dist;

        found.insert(tempPair);
      }
    }
  }

  list<GameObj*> result;
  set<pair<GameObj*, double> >::iterator it = found.begin();
  for (int i=0; i<N; i++) {
    if (it != found.end()) {
      result.push_back(it->first);
      it++;
    }
    else {
      result.push_back(NULL);
      dbg << "NULL pushed\n";
    }
  }

  return result;
}
