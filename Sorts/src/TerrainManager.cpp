#include "TerrainManager.h"
#include "SpatialDB.h"
#include "BFS.h"
#include "Sorts.h"

TerrainManager::~TerrainManager() {
  for(list<BFS*>::iterator
      i  = searches.begin();
      i != searches.end();
      ++i)
  {
    delete *i;
  }
  for(list<TerrainContour*>::iterator
      i  = contours.begin();
      i != contours.end();
      ++i)
  {
    delete *i;
  }
}

void TerrainManager::addSegment(int x1, int y1, int x2, int y2) {
  cout << "SEARCHES: " << searches.size() << endl;
  Point p1(x1, y1);
  Point p2(x2, y2);

  map<Point, BFS*>::iterator i1 = openPoints.find(p1);
  map<Point, BFS*>::iterator i2 = openPoints.find(p2);

  if (i1 == openPoints.end() && i2 == openPoints.end()) {
    // this is a new line altogether
    BFS* newSearch = new BFS(p1);
    newSearch->insertEdge(p1, p2, NULL);
    searches.push_back(newSearch);
    openPoints[p1] = newSearch;
    openPoints[p2] = newSearch;
    return;
  }

  list<Point> cycle;
  if (i1 == openPoints.end()) {
    // p2 is part of a search
    i2->second->insertEdge(p1, p2, &cycle);
    openPoints[p1] = i2->second;
  }
  else if (i2 == openPoints.end()) {
    // p1 is part of a search
    i1->second->insertEdge(p1, p2, &cycle);
    openPoints[p2] = i1->second;
  }
  else if (i1->second == i2->second) {
    // part of the same search
    i1->second->insertEdge(p1, p2, &cycle);
  }

  if (cycle.size() > 0) {
    TerrainContour* c = new TerrainContour(cycle);
    contours.push_back(c);
    Sorts::spatialDB->addTerrainContour(c);
  }

  if (i1 != openPoints.end() &&
      i2 != openPoints.end() &&
      i1->second != i2->second)
  {
    // combine these two searches
    BFS* takeOver;
    BFS* retired;
    Point takeOverPoint;
    Point retiredPoint;
    if (i1->second->size() > i2->second->size()) {
      takeOver = i1->second;
      retired = i2->second;
      takeOverPoint = p1;
      retiredPoint = p2;
    }
    else {
      takeOver = i2->second;
      retired = i1->second;
      takeOverPoint = p2;
      retiredPoint = p1;
    }
    takeOver->takeOver(retired, takeOverPoint, retiredPoint);
    for(map<Point, BFS*>::iterator
        i  = openPoints.begin();
        i != openPoints.end();
        ++i)
    {
      if (i->second == retired) {
        i->second = takeOver;
      }
    }
    searches.erase(find(searches.begin(), searches.end(), retired));
    delete retired;
  }
}

