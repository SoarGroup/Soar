#include "SpatialDB.h"
#include "Sorts.h"
#include "general.h"

#include <map>

#define WORKER_RADIUS 3

#define WORKER_CROWD_FACTOR 4
#define CROWD_MAX_MINERALS 8
#define CROWD_MAX_WORKERS 2

#define msg cout << "SDB: "

inline int intDivC(int x, int y) {
  return (int) ceilf(((float) x) / y);
}

SpatialDB::SpatialDB() {
  width = 0;
  height = 0;
}

void SpatialDB::init() {
  tile_points = 50;
  width = intDivC(Sorts::OrtsIO->getMapXDim(), tile_points);
  height = intDivC(Sorts::OrtsIO->getMapYDim(), tile_points);

  int mapsize = static_cast<int>(width*height);
  // Tilepoints define the granularity of the grid... 
  // The higher the tilepoints, the coarser the detail
  gobMap.resize(mapsize);
  imaginaryWorkerMap.resize(mapsize);
  terrainLineMap.resize(mapsize);
  msg  << "Initializing SpatialDB grid: (" << width<<"," << height
       << "," << (width*height) << ")\n";
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
  msg<<"Registered new Object: "<<gob->bp_name()<<" gob " << (int)gob << "\n";

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
  msg << "Registered new imaginary worker.\n";
  msg << "iw loc: " << c.x << "," << c.y << endl;
}

int SpatialDB::getCellNumber(int x, int y) {
  return (y / tile_points) * width + x / tile_points;
}

int SpatialDB::cell2row(int cellNum) {
  return (int)(cellNum / width);
}

int SpatialDB::cell2column(int cellNum) {
  return (cellNum % width);
}

int SpatialDB::colRow2cell(int row, int col) {
  return (col + row*width);
}

void SpatialDB::addTerrainLine(Line l) {
  // no need for removal/update support
  int cell1 = getCellNumber(l.a.x, l.a.y);
  int cell2 = getCellNumber(l.b.x, l.b.y);
  if (cell1 >= gobMap.size() || cell2 >= gobMap.size()) {
    msg << "ERROR: out of bounds, not adding: " << l.a << " - " << l.b << endl;
    return; 
  }
  //msg << "adding terrain line from " << l.a << " to " << l.b << endl;
  int col1 = cell2column(cell1);
  int row1 = cell2row(cell1);
  int col2 = cell2column(cell2);
  int row2 = cell2row(cell2);

  int count = 0;
  int cellNum;
  
  if (col1 > col2) {
    int tmp = col1;
    col1 = col2;
    col2 = tmp;
  }
  if (row1 > row2) {
    int tmp = row1;
    row1 = row2;
    row2 = tmp;
  }
    
  for (int i=col1; i<= col2; i++) {
    for (int j=row1; j<= row2; j++) {
      cellNum = colRow2cell(i,j);
    //  msg << "register in cell " << cellNum << endl;
      terrainLineMap[cellNum].push_back(l);
      count++;
    }
  }
  //msg << "registered in " << count << " cells.\n";

}

sint4 SpatialDB::updateObject(GameObj *gob, sint4 sat_loc)
{
 //return 0;
 sint4 x = *(gob->sod.x);
 sint4 y = *(gob->sod.y);
 sint4 new_sat_loc = getCellNumber(x,y);

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

void SpatialDB::getCollisions
( sint4 x, sint4 y, sint4 r, ERF* erf, list<GameObj*>& collisions)
{
  msg << "checking circle (getColl)\n";
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
      //msg << "Checking bin " << bins[i] << endl;
      int cellStartRow = (bins[i] / binWidth) * binSize;
      int cellStartCol = (bins[i] % binWidth) * binSize;
      for(int j = cellStartRow; j < cellStartRow + binSize; j++) {
        if (j >= height) { break; }
        for(int k = cellStartCol; k < cellStartCol + binSize; k++) {
          if (k >= width) { break; }

          int cell = j * width + k;
       //   msg << "Checking cell " << j << ", " << k << "(" << cell
        //      << ")" << endl;
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

bool SpatialDB::hasMiningCollision(coordinate c, bool checkCrowding) {
  return hasObjectCollisionInt(c, WORKER_RADIUS+1, true, checkCrowding);
}

bool SpatialDB::hasObjectCollision(Rectangle* rect) {
  // overestimate as a circle
  coordinate c;
  c.x = (int)((rect->xmax + rect->xmin)/2.0);
  c.y = (int)((rect->ymax + rect->ymin)/2.0);
  int radius = (int)(sqrt(squaredDistance(c.x, c.y, rect->xmax, rect->ymax)));
  return hasObjectCollisionInt(c, radius, false, false);
}

bool SpatialDB::hasObjectCollision(sint4 x, sint4 y, sint4 r) {
  coordinate c(x,y);
  return hasObjectCollisionInt(c, r, false, false);
}

bool SpatialDB::hasObjectCollisionInt(coordinate c, 
                                      int radius, bool forMining, 
                                      bool checkCrowding) {
  msg << "checking circle\n";
  int cells[9];
  bool check[9] = {false};

  sint4 r, bigR;
  if (checkCrowding) {
    r = WORKER_CROWD_FACTOR*WORKER_RADIUS;
  }
  else {
    r = radius;
  }

  bigR = r + tile_points;
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
  int crowdCount = 0;
  
  for(int i=0; i<9; i++) {
    if(check[i]) {
      assert(cells[i] < gobMap.size());
      for(it = gobMap[cells[i]].begin(); it != gobMap[cells[i]].end(); it++) {
        obj.x =  (*(*it)->sod.x);
        obj.y =  (*(*it)->sod.y);
        objr =  (*(*it)->sod.radius);
        if((x-obj.x) * (x-obj.x) + (y-obj.y) * (y-obj.y) 
         < (r+objr) * (r+objr))  {
          //Inside the circle
          if (not checkCrowding) {
            if (forMining) {
              if (((*it)->bp_name() != "worker") 
                  and
                  ((*it)->bp_name() != "sheep")
                  and
                  ((*it)->bp_name() != "controlCenter")) {
                msg << "mining collision with " << (*it)->bp_name() << endl;
                return true;
              }
            }
            else {
              msg << "object collision with " << (*it)->bp_name() << endl;
              return true;
            } 
          }
          else if ((*it)->bp_name() == "mineral") {
            crowdCount++;
            if (crowdCount >= CROWD_MAX_MINERALS) {
              msg << "too many minerals!\n";
              return true;
            }
          }
          
        }
      }
      crowdCount = 0;
      for(iwit = imaginaryWorkerMap[cells[i]].begin(); 
          iwit != imaginaryWorkerMap[cells[i]].end(); 
          iwit++) {
        obj.x = (*iwit).x;
        obj.y = (*iwit).y;
        objr = WORKER_RADIUS + 1; // radius of the imaginary worker
        if((x-obj.x) * (x-obj.x) + (y-obj.y) * (y-obj.y) 
        < (r+objr) * (r+objr))  {
          //Inside the circle
          if (not checkCrowding) {
            msg << "imaginary worker collision!\n";
            return true;
          }
          else {
            crowdCount++;
            if (crowdCount >= CROWD_MAX_WORKERS) {
              msg << "too many imaginary workers!\n";
              return true;
            }
          }
        }
      }
    }
  }
  msg << "object at " << c << " with radius " << radius 
      << " has no collisions\n"; 
  return false; // no collisions
}

bool SpatialDB::hasTerrainCollision(Rectangle *rect) {
 // determine which sectors the gob is in
 // only supports rectangles (buildings) now!
  
  int upperRightSector = getCellNumber(rect->xmax, rect->ymin);
  int lowerLeftSector = getCellNumber(rect->xmin, rect->ymax);

  int minCol = cell2column(lowerLeftSector);
  int maxCol = cell2column(upperRightSector);
  int minRow = cell2row(upperRightSector);
  int maxRow = cell2row(lowerLeftSector);

  assert (minCol <= maxCol && minRow <= maxRow);

  int cellNum;
  list<Line>::iterator it;
  for (int i=minCol; i<=maxCol; i++) {
    for (int j=minRow; j<=maxRow; j++) {
      cellNum = colRow2cell(i,j);
      for (it = terrainLineMap[cellNum].begin();
          it != terrainLineMap[cellNum].end();
          it++) {
        msg << "checking intersection\n";
        if (rect->intersects(*it)) {
          msg << "rect " << *rect << " intersects line " << (*it).a 
              << "-" << (*it).b << endl;
          return true;
        }
      }
    }
  }

  return false;
}


