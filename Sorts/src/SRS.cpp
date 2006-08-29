#include "SRS.h"

#define CLASS_TOKEN "SRS"
#define DEBUG_OUTPUT true 
#include "OutputDefinitionsUnique.h"
#include <sstream>

SpatialReasoningSystem* theSRS;

SpatialReasoningSystem::SpatialReasoningSystem() {
  theSRS = this;
  worldIsPolygon = false;
}

void SpatialReasoningSystem::setWorldBounds(list<pair<double, double> > worldBounds) {
  worldIsPolygon = true;
  for (list<pair<double,double> >::iterator it = worldBounds.begin();
        it != worldBounds.end();
        it++) {
    worldPolygon.push_back(CGALPoint(it->first, it->second));
  }
}

void SpatialReasoningSystem::initCanvas(double ww, double wh, double scale) {
  int status = canvas.init(ww, wh, scale);
  assert(status == 0);
}

void SpatialReasoningSystem::insertCircle(double x, double y, double radius, 
                                          double i, double j, double speed,
                                          int id, string name) {
  SRSShape* shape = new SRSShape(x, y, radius, i, j, speed, name, &canvas);
  shapes.insert(make_pair(id, shape));
  shape->draw();
  if (canvas.initted()) {
    canvas.redraw();
  }
  CGALBox* box = new CGALBox(shape->getCircle()->bbox());
  boxes.push_back(box);
  boxToShape[box] = shape;
  shape->setBox(box);
}

void SpatialReasoningSystem::insertPolygon(list<pair<double, double> >& points, 
                                           double i, double j, double speed,
                                           int id, string name) {
  SRSShape* shape = new SRSShape(points, i, j, speed, name, &canvas);
  shapes.insert(make_pair(id, shape));
  shape->draw();
  if (canvas.initted()) {
    canvas.redraw();
  }
  CGALBox* box = new CGALBox(shape->getPolygon()->bbox());
  boxes.push_back(box);
  boxToShape[box] = shape;
  shape->setBox(box);
}

void SpatialReasoningSystem::removeShape(int id) {
  map<int, SRSShape*>::iterator it = shapes.find(id);
  if (it == shapes.end()) {
    msg << "ERROR: removing shape not added!\n";
    assert(false);
  }
  else {
    CGALBox* box = it->second->getBox();
    boxToShape.erase(box);
    //boxes.erase(box);
    for (vector<CGALBox*>::iterator it2 = boxes.begin();
        it2 != boxes.end();
        it2++) {
      if (*it2 == box) {
        boxes.erase(it2);
        break;
      }
    }
    delete box;
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
    /*    dir = it->second->getAllocentricRelativeOrientationOf(it2->second);
        msg << "allocentric relative orientation of object " << it->first << " to object " << it2->first
            << " is " << dir << endl;
*/        it2++;
      }
    }
  }
}

double SpatialReasoningSystem::twoObjectQuery(twoObjectQueryType type, int referenceId, int primaryId) {
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
    case DISTANCE:
      return referenceObject->getDistanceTo(primaryObject);
      break; 
    case BIGGER:
      return referenceObject->isBiggerThan(primaryObject);
    case SHORTEST_DIST_CLEAR:
      return shortestDistanceClearQuery(referenceObject, primaryObject);
    default:
      return -1;
  }
  
  return -1;
}

void SpatialReasoningSystem::objectProjectionQuery(int objID, double time, double& x, double& y) {
  SRSShape* primaryObject;

  map<int, SRSShape*>::iterator it;
  it = shapes.find(objID);
  if (it == shapes.end()) {
    x = -1;
    y = -1;
    return;
  }
  primaryObject = it->second;
  
  CGALSegment centroidTranslation = primaryObject->translateCentroid(time);
  if (worldIsPolygon) {
    CGALPoint result = constrainEndpoint(&centroidTranslation, &worldPolygon);
    x = result.x();
    y = result.y();
  }
  else {
    x = centroidTranslation.target().x();
    y = centroidTranslation.target().y();
  }
  return;
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
    case DISTANCE:
      return "distance";
    case BIGGER:
      return "bigger than";
    case SHORTEST_DIST_CLEAR:
      return "shortest distance clear";
    default:
      return "[unknown]";
  }
} 

void SpatialReasoningSystem::drawPolygon(CGALPolygon* polygon, int r, int g, int b) {
  // only for debugging, there is no way to erase!
  SDLCanvasShape* line;
  for (CGALPolygon::Edge_const_iterator it = polygon->edges_begin();
        it != polygon->edges_end();
        it++) {
    line = canvas.makeLine((*it).source().x(), (*it).source().y(), (*it).target().x(), (*it).target().y());
    line->setShapeColor(r,g,b);
  }
  canvas.redraw();
}

bool SpatialReasoningSystem::polygonIntersect(CGALPolygon* poly) {
  vector<CGALBox*> boxList;
  boxList.push_back(new CGALBox(poly->bbox()));

  lastIntersectPolygon = poly;
  
  lastIntersect = false;
  // intersect callback will change this flag if needed
  CGAL::box_intersection_d(boxes.begin(), boxes.end(), boxList.begin(), boxList.end(), 
                          boxIntersectCallbackWrapper);
  delete *boxList.begin();
  
  return lastIntersect;
}

void boxIntersectCallbackWrapper(const CGALBox* a, const CGALBox* b) {
  theSRS->boxIntersectCallback(a,b);
}

void SpatialReasoningSystem::boxIntersectCallback(const CGALBox* ac, const CGALBox* bc) {
  SRSShape* target;
  CGALBox* a = const_cast<CGALBox*>(ac);

  if (lastIntersect) {
    dbg << "ignoring callback, something already intersected.\n";
    return;
  }

  if (boxToShape.find(a) != boxToShape.end()) {
    target = boxToShape.find(a)->second;
  }
  else {
    assert(false);
  }

  if (target->getIgnore()) {
    dbg << "target is ignored, no collision.\n";
    return;
  }
  
  dbg << "potential collision between " << target->getName() << " and box." << endl;

  
  if (lastIntersectPolygon->size() == 2) {
    // its actually a line
    CGALSegment seg(*(lastIntersectPolygon->vertices_begin()), *(lastIntersectPolygon->vertices_begin()+1));
    if (target->getIsCircle()) {
      if (doIntersect(target->getCircle(), &seg)) {
        dbg << "collided!\n";
        lastIntersect = true;
      } 
    }
    else {
      if (doIntersect(&seg, target->getPolygon())) {
        dbg << "collided!\n";
        lastIntersect = true;
      } 
    }
  }
  else {
    if (target->getIsCircle()) {
      if (doIntersect(target->getCircle(), lastIntersectPolygon)) {
        dbg << "collided!\n";
        lastIntersect = true;
      } 
    }
    else {
      if (CGAL::do_intersect(*lastIntersectPolygon, *(target->getPolygon()))) {
        dbg << "collided!\n";
        lastIntersect = true;
      }
    }
  }
  return;
}

bool SpatialReasoningSystem::shortestDistanceClearQuery(SRSShape* referenceObject, SRSShape* primaryObject) {

  CGALPolygon* region = referenceObject->getShortestDistanceRegionTo(primaryObject);
  drawPolygon(region, 0,0,255);

  // intersections are tricky because the shapes directly touch, so ignore all
  // collisions involving the two objects in question

  bool oldRefIgnore = referenceObject->getIgnore();
  bool oldPriIgnore = primaryObject->getIgnore();
  referenceObject->setIgnore(true);
  primaryObject->setIgnore(true);
  bool retval = polygonIntersect(region);
  referenceObject->setIgnore(oldRefIgnore);
  primaryObject->setIgnore(oldPriIgnore);
  delete region;
  return not retval;
}
  
