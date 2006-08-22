#include "SRS.h"

#define CLASS_TOKEN "SRS"
#define DEBUG_OUTPUT true 
#include "OutputDefinitionsUnique.h"
#include <sstream>

SpatialReasoningSystem::SpatialReasoningSystem() {
}

void SpatialReasoningSystem::initCanvas(double ww, double wh, double scale) {
  int status = canvas.init(ww, wh, scale);
  assert(status == 0);
}

void SpatialReasoningSystem::insertCircle(double x, double y, double radius, 
                                          double i, double j, int id, string name) {
  SRSShape* shape = new SRSShape(x, y, radius, i, j, name, &canvas);
  shapes.insert(make_pair(id, shape));
  shape->draw();
  if (canvas.initted()) {
    canvas.redraw();
  }
}

void SpatialReasoningSystem::insertPolygon(list<pair<double, double> >& points, 
                                           double i, double j, int id, string name) {
  SRSShape* shape = new SRSShape(points, i, j, name, &canvas);
  shapes.insert(make_pair(id, shape));
  shape->draw();
  if (canvas.initted()) {
    canvas.redraw();
  }
}

void SpatialReasoningSystem::removeShape(int id) {
  map<int, SRSShape*>::iterator it = shapes.find(id);
  if (it == shapes.end()) {
    msg << "ERROR: removing shape not added!\n";
    assert(false);
  }
  else {
    it->second->deleteShape();
    //delete it->second;
    shapes.erase(it);
  }
}

void SpatialReasoningSystem::printAllRelativeOrientations() {
  dbg << "printAllRelativeOrientations: " << shapes.size() << " shapes.\n";
  for (map<int, SRSShape*>::iterator it = shapes.begin();
       it != shapes.end();
       it++) {
    map<int, SRSShape*>::iterator it2 = it;
    it2++;
    while (it2 != it) {
      if (it2 == shapes.end()) {
        it2 = shapes.begin();
      }
      if (it2 != it) {
        int dir = it->second->getRelativeOrientationOf(it2->second);
        msg << "relative orientation of object " << it->first << " to object " << it2->first
            << " is " << dir << endl;
        dir = it->second->getAllocentricRelativeOrientationOf(it2->second);
        msg << "allocentric relative orientation of object " << it->first << " to object " << it2->first
            << " is " << dir << endl;
        it2++;
      }
    }
  }
}

int SpatialReasoningSystem::twoObjectQuery(twoObjectQueryType type, int referenceId, int primaryId) {
  SRSShape* referenceObject;
  SRSShape* primaryObject;

  map<int, SRSShape*>::iterator it;
  it = shapes.find(referenceId);
  if (it == shapes.end()) {
    return -1;
  }
  referenceObject = it->second;
  
  it = shapes.find(primaryId);
  if (it == shapes.end()) {
    return -1;
  }
  primaryObject = it->second;

  switch(type) {
    case RCC_DR:
      return (int)referenceObject->RCC_DR(primaryObject);
      break;
    case RCC_PO:
      return (int)referenceObject->RCC_PO(primaryObject);
      break;
    case RCC_EQ:
      return (int)referenceObject->RCC_EQ(primaryObject);
      break;
    case RCC_PP:
      return (int)referenceObject->RCC_PP(primaryObject);
      break;
    case RCC_PPI:
      return (int)referenceObject->RCC_PPi(primaryObject);
      break;
    case ORIENTATION:
      return referenceObject->getRelativeOrientationOf(primaryObject);
      break;
    case ALLOCENTRIC_ORIENTATION:
      return referenceObject->getAllocentricRelativeOrientationOf(primaryObject);
      break;
    default:
      return -1;
  }
  
  return -1;
}

string queryToString(twoObjectQueryType t){
  switch (t) {
    case RCC_DR:
      return "RCC_DR";
    case RCC_PO:
      return "RCC_PO";
    case RCC_PP:
      return "RCC_PP";
    case RCC_PPI:
      return "RCC_PPi";
    case RCC_EQ:
      return "RCC_EQ";
    case ORIENTATION:
      return "Relative Orientation";
    case ALLOCENTRIC_ORIENTATION:
      return "Allocentric Orientation";
    default:
      return "[unknown]";
  }
} 
