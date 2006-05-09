#include "Satellite.h"
#include "Sorts.h"

Satellite::Satellite()
{
 refCount = 1;
}

void Satellite::init()
{
 width = Sorts::OrtsIO->getMapXDim();
 height = Sorts::OrtsIO->getMapYDim();

 tile_points = 10;
 //Map.resize(static_cast<int>((width*height)/tile_points));
 Map.resize(1);
}

Satellite::~Satellite()
{
 width = 0;
 height = 0;
}

std::list<GameObj*> *Satellite::getObjectsInRegion(int x, int y)
{
 return &Map[0];
 //return &Map[(y*width+x)/tile_points];
}


sint4 Satellite::addObject(GameObj *gob)
{
 sint4 x = *(gob->sod.x);
 sint4 y = *(gob->sod.y);
 
 Map[0].push_back(gob);
 //Map[(y*width+x)/tile_points].push_back(gob); 
 cout<<"Satellite: Registered new Object: "<<gob->bp_name()<<"\n";

 return 0;
 //return ((y*width+x)/tile_points);
}

sint4 Satellite::updateObject(GameObj *gob, sint4 sat_loc)
{
 return 0;
 sint4 x = *(gob->sod.x);
 sint4 y = *(gob->sod.y);
 sint4 new_sat_loc = (y*width+x)/tile_points;

 if(sat_loc != new_sat_loc)
 {
  Map[sat_loc].remove(gob);
  Map[new_sat_loc].push_back(gob);
 }
 else
  new_sat_loc = sat_loc;

 //return new_sat_loc;
}

