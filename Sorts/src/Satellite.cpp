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


void Satellite::addObject(GameObj *gob)
{
 sint4 x = *(gob->sod.x);
 sint4 y = *(gob->sod.y);
 
 Map[y*width+x].push_back(gob); 
 return;
}
