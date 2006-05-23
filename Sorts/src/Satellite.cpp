#include "Satellite.h"
#include "Sorts.h"

#include <map>

#define WORKER_RADIUS 3

#define msg cout << "Satellite.cpp: "

inline int intDivC(int x, int y) {
  return (int) ceilf(((float) x) / y);
}

Satellite::Satellite() {
  width = 0;
  height = 0;
}

void Satellite::init() {
  tile_points = 40;
  width = intDivC(Sorts::OrtsIO->getMapXDim(), tile_points);
  height = intDivC(Sorts::OrtsIO->getMapYDim(), tile_points);

  // Tilepoints define the granularity of the grid... 
  // The higher the tilepoints, the coarser the detail
  Map.resize(static_cast<int>(width*height));
  ImaginaryWorkers.resize(static_cast<int>(width*height));
  msg  << "Initializing Satellite grid: (" << width<<"," << height
       << "," << (width*height) << ")\n";
}

Satellite::~Satellite()
{ }

std::set<GameObj*> *Satellite::getObjectsInRegion(int x, int y)
{
 return &Map[(y*width+x)/tile_points];
}


sint4 Satellite::addObject(GameObj *gob) {
  sint4 x = *(gob->sod.x);
  sint4 y = *(gob->sod.y);
  if ((y*width+x)/tile_points >= Map.size()) {
    msg << "ERROR: out of bounds, not adding: " << x << "," << y << endl;
    return -1; 
  }
  
  Map[(y*width+x)/tile_points].insert(gob); 
  msg<<"Registered new Object: "<<gob->bp_name()<<"\n";

  return static_cast<sint4>((y*width+x)/tile_points);
}

void Satellite::addImaginaryWorker(coordinate c) {
  // no need for removal/update support
  if ((c.y*width+c.x)/tile_points >= Map.size()) {
    msg << "ERROR: out of bounds, not adding: " << c.x << "," << c.y << endl;
    return; 
  }
  
  ImaginaryWorkers[(c.y*width+c.x)/tile_points].push_back(c); 
  msg << "Registered new imaginary worker.\n";
  msg << "iw loc: " << c.x << "," << c.y << endl;
}

sint4 Satellite::updateObject(GameObj *gob, sint4 sat_loc)
{
 //return 0;
 sint4 x = *(gob->sod.x);
 sint4 y = *(gob->sod.y);
 sint4 new_sat_loc = static_cast<sint4>((y*width+x)/tile_points);

 if(sat_loc != new_sat_loc)
 {
  Map[sat_loc].erase(gob);
  Map[new_sat_loc].insert(gob);
 }
 else
  new_sat_loc = sat_loc;

 return new_sat_loc;
}

void Satellite::removeObject(GameObj *gob, sint4 sat_loc) {
  if (sat_loc < 0) return;
  Map[sat_loc].erase(gob);
}

void Satellite::getCollisions
( sint4 x, sint4 y, sint4 r, ERF* erf, list<GameObj*>& collisions)
{
  collisions.clear();

  // how many cells to bin together to get the size of a bin
  // to be bigger than the max expected radius
  int cr; // collision radius
  if (erf == NULL) {
    cr = r + intDivC(tile_points, 2); 
    // tile_points being the upperbound of the radii of other objects
  }
  else {
    cr = r + (int) erf->maxRadius();
  }
  int binSize = intDivC(cr, tile_points);
  msg << "binSize: " << binSize << endl;
  int binTilePoints = binSize * tile_points;
  int binWidth = intDivC(Sorts::OrtsIO->getMapXDim(), binTilePoints);
  assert (binWidth == intDivC(width, binSize));
  
  int bins[9];
  bool check[9] = {false};

  //1. Figure out which bins surround the target location 
  bins[0] = (y - cr) / binTilePoints * binWidth + (x - cr) / binTilePoints;
  bins[1] = (y - cr) / binTilePoints * binWidth + x / binTilePoints;
  bins[2] = (y - cr) / binTilePoints * binWidth + (x + cr) / binTilePoints;
  bins[3] = y / binTilePoints * binWidth + (x - cr) / binTilePoints;
  bins[4] = y / binTilePoints * binWidth + x / binTilePoints;
  bins[5] = y / binTilePoints * binWidth + (x + cr) / binTilePoints;
  bins[6] = (y + cr) / binTilePoints * binWidth + (x - cr) / binTilePoints;
  bins[7] = (y + cr) / binTilePoints * binWidth + x / binTilePoints;
  bins[8] = (y + cr) / binTilePoints * binWidth + (x + cr) / binTilePoints;
  
  //2. Figure out which bins are under (or partially under) the circle
  // 0 1 2
  // 3 4 5
  // 6 7 8

  //Center cell
  check[4] = true; 
  //North
  if(bins[4] != bins[1])
   check[1] = true;  
  //East
  if(bins[4] != bins[5])
    check[5] = true;
  //NorthEast
  check[2] = check[1] && check[5];
  //South
  if(bins[4] != bins[7])
    check[7] = true;
  //SouthEast
  check[8] = check[5] && check[7];
  //West
  if(bins[4] != bins[3])
    check[3]= true;
  //SourhWest
  check[6] = check[7] && check[3];
  //NorthWest
  check[0] = check[3] && check[1];

  //Make sure we the check are inside the map
  //Left Side
  if(x < cr)
    check[0] = check[3] = check[6] = false;
  //Right Side
  if(x+cr > Sorts::OrtsIO->getMapXDim())
    check[2] = check[5] = check[8] = false;
  //Top Side
  if(y < cr)
    check[0] = check[1] = check[2] = false;
  //Bottom Side
  if(y+cr > Sorts::OrtsIO->getMapYDim())
    check[6] = check[7] = check[8] = false;

 
  // For each marked cell, check all objects inside of it 
  // for collisions with the circle
  map<GameObj*, int> whereFound;
  std::set<GameObj*>::iterator it;
  for(int i=0; i<9; i++) {
    if(check[i]) {
      msg << "Checking bin " << bins[i] << endl;
      int cellStartRow = (bins[i] / binWidth) * binSize;
      int cellStartCol = (bins[i] % binWidth) * binSize;
      for(int j = cellStartRow; j < cellStartRow + binSize; j++) {
        if (j >= height) { break; }
        for(int k = cellStartCol; k < cellStartCol + binSize; k++) {
          if (k >= width) { break; }

          int cell = j * width + k;
          msg << "Checking cell " << j << ", " << k << "(" << cell
              << ")" << endl;
          for(it = Map[cell].begin(); it != Map[cell].end(); it++) {
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
              collisions.push_back((*it));
              if (whereFound.find(*it) != whereFound.end()) {
                msg << "Object found previously in " << whereFound[*it]
                    << "is now found at " << cell << endl;
                assert(false);
              }
              else {
                whereFound[*it] = cell;
              }
            } 
          }
        }
      }
    }
  }
}

bool Satellite::hasMiningCollision(coordinate c) {
  int cells[9];
  bool check[9] = {false};

  sint4 x = c.x;
  sint4 y = c.y;
  sint4 r = WORKER_RADIUS;
  sint4 bigR = WORKER_RADIUS + WORKER_RADIUS;

   //1. Figure out which cells surround the target location 
  cells[0] = (y - bigR) / tile_points * width + (x - bigR) / tile_points;
  cells[1] = (y - bigR) / tile_points * width + x / tile_points;
  cells[2] = (y - bigR) / tile_points * width + (x + bigR) / tile_points;
  cells[3] = y / tile_points * width + (x - bigR) / tile_points;
  cells[4] = y / tile_points * width + x / tile_points;
  cells[5] = y / tile_points * width + (x + bigR) / tile_points;
  cells[6] = (y + bigR) / tile_points * width + (x - bigR) / tile_points;
  cells[7] = (y + bigR) / tile_points * width + x / tile_points;
  cells[8] = (y + bigR) / tile_points * width + (x + bigR) / tile_points;
  
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
  //SourhWest
  check[6] = check[7] && check[3];
  //NorthWest
  check[0] = check[3] && check[1];
  
  //Make sure we the check are inside the map
  //Left Side
  if((x-r)<0)
    check[0] = check[3] = check[6] = false;
  //Right Side
  if((x+r)>Sorts::OrtsIO->getMapXDim())
    check[2] = check[5] = check[8] = false;
  //Top Side
  if((y-r)<0)
    check[0] = check[1] = check[2] = false;
  //Bottom Side
  if((y+r)>Sorts::OrtsIO->getMapYDim())
    check[6] = check[7] = check[8] = false;

  
  //3. For each marked cell, check all object inside of it for collisions with the circle
  TerrainBase::Loc obj;
  sint4 objr;
  set<GameObj*>::iterator it;
  list<coordinate>::iterator iwit;
  
  for(int i=0; i<9; i++) {
    if(check[i]) {
      for(it = Map[cells[i]].begin(); it != Map[cells[i]].end(); it++) {
        obj.x =  (*(*it)->sod.x);
        obj.y =  (*(*it)->sod.y);
        objr =  (*(*it)->sod.radius);
        if((x-obj.x) * (x-obj.x) + (y-obj.y) * (y-obj.y) 
         < (r+objr) * (r+objr))  {
          //Inside the circle
          if (((*it)->bp_name() != "worker") 
              and
              ((*it)->bp_name() != "sheep")) {
            return true;
          }
        }
      }
      for(iwit = ImaginaryWorkers[cells[i]].begin(); 
          iwit != ImaginaryWorkers[cells[i]].end(); 
          iwit++) {
        obj.x = (*iwit).x;
        obj.y = (*iwit).y;
        objr = WORKER_RADIUS + 1;
        if((x-obj.x) * (x-obj.x) + (y-obj.y) * (y-obj.y) 
         < (r+objr) * (r+objr))  {
          //Inside the circle
          msg << "imaginary worker collision!\n";
          return true;
        }
      }
    }
  }
  return false; // no collisions
}
