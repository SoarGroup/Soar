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

 Map.resize(width*height);
}

Satellite::~Satellite()
{
 width = 0;
 height = 0;
}

std::list<GameObj*> *Satellite::getObjectsInRegion(int x, int y)
{
 return &Map[y*width+x];
}


sint4 Satellite::addObject(GameObj *gob)
{
 sint4 x = *(gob->sod.x);
 sint4 y = *(gob->sod.y);
 
 Map[y*width+x].push_back(gob); 
 cout<<"Satellite: Registered new Object: "<<gob->bp_name()<<"\n";
 return (y*width+x);
}

sint4 Satellite::updateObject(GameObj *gob, sint4 sat_loc)
{
 sint4 x = *(gob->sod.x);
 sint4 y = *(gob->sod.y);
 sint4 new_sat_loc = y*width+x;

 if(sat_loc != new_sat_loc)
 {
  Map[sat_loc].remove(gob);
  Map[new_sat_loc].push_back(gob);
 }
 else
  new_sat_loc = sat_loc;

 return new_sat_loc;
}
