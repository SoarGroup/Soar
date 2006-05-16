#include "Satellite.h"
#include "Sorts.h"

Satellite::Satellite()
{
 width = 0;
 height = 0;
}

void Satellite::init()
{
 tile_points = 100;
 width = Sorts::OrtsIO->getMapXDim()/tile_points;
 height = Sorts::OrtsIO->getMapYDim()/tile_points;

 //Tilepoints define the granularity of the grid... The higher the tilepoints, the coarser the detail
 Map.resize(static_cast<int>(width*height));
 cout<<"Initializing Satellite grid: ("<<width<<","<<height<<","<<(width*height)<<")\n";
}

Satellite::~Satellite()
{
 width = 0;
 height = 0;
}

std::list<GameObj*> *Satellite::getObjectsInRegion(int x, int y)
{
 return &Map[(y*width+x)/tile_points];
}


sint4 Satellite::addObject(GameObj *gob)
{
 sint4 x = *(gob->sod.x);
 sint4 y = *(gob->sod.y);
 
 //Map[0].push_back(gob);
 Map[(y*width+x)/tile_points].push_back(gob); 
 cout<<"Satellite: Registered new Object: "<<gob->bp_name()<<"\n";

 //return 0;
 return static_cast<sint4>((y*width+x)/tile_points);
}

sint4 Satellite::updateObject(GameObj *gob, sint4 sat_loc)
{
 //return 0;
 sint4 x = *(gob->sod.x);
 sint4 y = *(gob->sod.y);
 sint4 new_sat_loc = static_cast<sint4>((y*width+x)/tile_points);

 if(sat_loc != new_sat_loc)
 {
  Map[sat_loc].remove(gob);
  Map[new_sat_loc].push_back(gob);
 }
 else
  new_sat_loc = sat_loc;

 return new_sat_loc;
}

void Satellite::removeObject(GameObj *gob, sint4 sat_loc)
{
 Map[sat_loc].remove(gob);
}

std::list<GameObj*> *Satellite::getCollisions(sint4 x, sint4 y, sint4 r)
{
 std::list<GameObj*> *cols = new std::list<GameObj*>;
 
 int cells[9];
 bool check[9] = {false};

 //1. Figure out which cells surround the target location 
 cells[0] = static_cast<int>((((y-r)*width)+(x-r))/tile_points);
 cells[1] = static_cast<int>((((y-r)*width)+(x))/tile_points);
 cells[2] = static_cast<int>((((y-r)*width)+(x+r))/tile_points);
 cells[3] = static_cast<int>((((y)*width)+(x-r))/tile_points);
 cells[4] = static_cast<int>((((y)*width)+(x))/tile_points);
 cells[5] = static_cast<int>((((y)*width)+(x+r))/tile_points);
 cells[6] = static_cast<int>((((y+r)*width)+(x-r))/tile_points);
 cells[7] = static_cast<int>((((y+r)*width)+(x))/tile_points);
 cells[8] = static_cast<int>((((y+r)*width)+(x+r))/tile_points);

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
 cells[2] = cells[1] && cells[5];
 //South
 if(cells[4] != cells[7])
  cells[7] = true;
 //SouthEasr
 cells[8] = cells[5] && cells[7];
 //West
 if(cells[4] != cells[3])
   check[3]= true;
 //SourhWest
 cells[6] = cells[7] && cells[3];
 //NorthWest
 cells[0] = cells[3] && cells[1];
 
 //Make sure we the cells are inside the map
 //Left Side
 if((x-r)<0)
  cells[0] = cells[3] = cells[6] = false;
 //Right Side
 if((x+r)>Sorts::OrtsIO->getMapXDim())
  cells[2] = cells[5] = cells[8] = false;
 //Top Side
 if((y-r)<0)
  cells[0] = cells[1] = cells[2] = false;
 //Bottom Side
 if((y+r)>Sorts::OrtsIO->getMapYDim())
  cells[6] = cells[7] = cells[8] = false;

 
 //3. For each marked cell, check all object inside of it for collisions with the circle
 std::list<GameObj*>::iterator it;
 for(int i=0; i<9; i++)
  if(check[i])
   for(it = Map[cells[i]].begin(); it != Map[cells[i]].end(); it++)
    if(true/*Inside the circle*/)
     //Should check distance between object and loc vs sum of radii
     cols->push_back((*it));
   
 //Return all items target circle collides with
 return cols;
}
