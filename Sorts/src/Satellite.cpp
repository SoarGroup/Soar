#include "Satellite.h"
#include "Sorts.h"

Satellite::Satellite()
{
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
