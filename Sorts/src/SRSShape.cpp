#include "SRSShape.h"

#define CLASS_TOKEN "SRSS"
#define DEBUG_OUTPUT true 
#include "OutputDefinitions.h"

#include <CGAL/centroid.h>
#include <sstream>

SRSShape::SRSShape(double x, double y, double radius, double i, double j, double _speed, string _name,
                   SDLCanvas* _canvas) :
  name(_name), isCircle(true), polygon(NULL), orientation(i,j), centroid(x,y), speed(_speed),
  canvas(_canvas), ignore(false){
  
  if (orientation.dy() == 0) {
    orientation = CGALDirection(i,.00001);
  }
  circle = new CGALCircle(centroid, radius*radius);
  area = -1;
  
  dbg << "created a circle, name= " << name << endl;
}

SRSShape::SRSShape(list<pair<double, double> >& points, 
                   double i, double j, double _speed, string _name, SDLCanvas* _canvas) :
  name(_name), isCircle(false), circle(NULL), orientation(i,j), speed(_speed),
  canvas(_canvas), ignore(false) {

  area = -1;
  if (orientation.dy() == 0) {
    orientation = CGALDirection(i,.00001);
  }
  polygon = new CGALPolygon();
  for (list<pair<double,double> >::iterator it = points.begin();
        it != points.end();
        it++) {
    polygon->push_back(CGALPoint(it->first, it->second));
  }

  if (not polygon->is_counterclockwise_oriented()) {
    polygon->reverse_orientation();
  }

  // non-convex polygons break distance calculations (convex hull is used)
  // area calculation also assumes non-negative area
  
  assert(polygon->is_convex());

  centroid = CGAL::centroid(polygon->vertices_begin(), polygon->vertices_end());
  dbg << "created a polygon,  name= " << name << " centroid " << centroid << endl;
}

void SRSShape::draw() {
  if (not canvas->initted()) {
    return;
  }
  if (isCircle) {
   SDLCanvasCircle* canvasCircle = 
     canvas->makeCircle(centroid.x(), centroid.y(), sqrt(circle->squared_radius()));
   //canvasCircle->setLabel(name);
   canvasObjs.push_back(canvasCircle);
  }
  else {
    bool notFirst = false;
    CGALPoint lastPoint;
    for (CGALPolygon::Vertex_const_iterator it = polygon->vertices_begin();
        it != polygon->vertices_end();
        it++) {
      if (notFirst) { 
        canvasObjs.push_back(canvas->makeLine(lastPoint.x(), lastPoint.y(), it->x(), it->y()));
      }
      lastPoint = *it;
      notFirst = true;
    }
    // link last->first
    canvasObjs.push_back(canvas->makeLine(lastPoint.x(), lastPoint.y(), 
                         polygon->vertices_begin()->x(), polygon->vertices_begin()->y()));
  }
  
  CGALRay orientationRay(centroid, orientation);
  SDLCanvasShape* orientationMark = canvas->makeLine(orientationRay.point(10).x(),
                                                      orientationRay.point(10).y(),
                                                      orientationRay.point(20).x(),
                                                      orientationRay.point(20).y());
  orientationMark->setShapeColor(0,255,0);
  orientationMark->setLabel(name);
  canvasObjs.push_back(orientationMark);
 /* 
  double x, y;
  centroidAtTime(-100, x, y); 
  SDLCanvasShape* historyMark = canvas->makeLine(centroid.x(),
                                                 centroid.y(),
                                                 x, y);
  historyMark->setShapeColor(0,0,255);
  canvasObjs.push_back(historyMark);
 */ 
} 


void SRSShape::deleteShape() {
  
  dbg << "deleting shape: " << name <<  " with " << canvasObjs.size() << " canvas objs " <<endl;
  if (circle != NULL) {
    delete circle;
  }
  if (polygon != NULL) {
    delete polygon;
  }

  for (list<SDLCanvasShape*>::iterator it = canvasObjs.begin();
       it != canvasObjs.end();
       it++) {
    // if canvas was never initted, this should be empty
    canvas->remove(*it);
  }
}

int SRSShape::getRelativeOrientationOf(SRSShape* primaryObject) {
  // return an integer corresponding to the orientation of the primary object 
  // relative to this reference object
  
  // for example, the monitor is in front of me, so the orientation of the
  // monitor relative to me is "front"
  
  // to do this, keep a list of acceptance area rays for the source object
  // (ideally, looking like fig 4.21 in Hernandez 1994)

  // in counterclockwise order, check whether the center point of the target
  // object lies on the left of
  // one ray and right of the the next, if so, give it the orientation number of
  // the index of the ray it lies left of

  // TODO: can a point exist that does not meet this criteria if it is inside
  // the source object?
  
  if (acceptanceAreaRays.size() == 0) {
    dbg << "generating acceptance area rays..\n";
    

    if (isCircle) { 
      // for now, just make 8 rays from the centroid
      // first, rotate the orientation right by pi/4 (so "straight ahead" surrounds
      // it)
      CGALTransformation firstRotate(CGAL::ROTATION, -1*SIN_PI_8, COS_PI_8);
      CGALTransformation rotate45(CGAL::ROTATION, SIN_PI_4, COS_PI_4);
      
      CGALDirection direction(orientation.transform(firstRotate));
      for (int i=0; i<8; i++) {
        acceptanceAreaRays.push_back(CGALRay(centroid, direction));
        direction = direction.transform(rotate45);
      }
    }
    else {
      // we need to make a bounding box in the direction of the orientation of
      // the object
      // to do that, rotate the polygon so that the orientation faces up, then
      // get a bounding box. make the rays from the bounding box, then rotate
      // them all back to where the box actually faces
   
      CGALTransformation moveToOrigin(CGAL::TRANSLATION, CGALVector(centroid, CGALPoint(CGAL::ORIGIN)));
      CGALTransformation moveBack(CGAL::TRANSLATION, CGALVector(CGALPoint(CGAL::ORIGIN), centroid));
      
      CGALPolygon tempPoly = CGAL::transform(moveToOrigin, *polygon);
      
      assert(orientation != CGALDirection(0,0));
      CGALTransformation rotateBack(CGAL::ROTATION, orientation, 1, 100);
      CGALTransformation makeOrientationVertical(CGAL::ROTATION, -orientation, 1, 100);
      tempPoly = CGAL::transform(makeOrientationVertical, tempPoly);
      
      CGALBBox box = tempPoly.bbox();

      // convert the bbox to a polygon
      CGALPolygon boxPoly;
      boxPoly.push_back(CGALPoint(box.xmax(), box.ymax()));
      boxPoly.push_back(CGALPoint(box.xmin(), box.ymax()));
      boxPoly.push_back(CGALPoint(box.xmin(), box.ymin()));
      boxPoly.push_back(CGALPoint(box.xmax(), box.ymin()));

      // generate the rays
      CGALPolygon::Vertex_const_iterator it = boxPoly.vertices_begin();
      vector<CGALRay> rays;
      rays.push_back(CGALRay(*it, CGALDirection(COS_PI_8, SIN_PI_8)));
      rays.push_back(CGALRay(*it, CGALDirection(COS_3PI_8, SIN_3PI_8)));
      ++it;
      rays.push_back(CGALRay(*it, CGALDirection(COS_5PI_8, SIN_5PI_8)));
      rays.push_back(CGALRay(*it, CGALDirection(COS_7PI_8, SIN_7PI_8)));
      ++it;
      rays.push_back(CGALRay(*it, CGALDirection(COS_9PI_8, SIN_9PI_8)));
      rays.push_back(CGALRay(*it, CGALDirection(COS_11PI_8, SIN_11PI_8)));
      ++it;
      rays.push_back(CGALRay(*it, CGALDirection(COS_13PI_8, SIN_13PI_8)));
      rays.push_back(CGALRay(*it, CGALDirection(COS_15PI_8, SIN_15PI_8)));
      
      tempPoly = CGAL::transform(rotateBack, boxPoly);

      // we now have a bounding-box polygon centered at the origin 
      // with a face in the direction of the orientation
      
      // rotate and translate all the rays to the correct location
      for (int i=0; i<8; i++) {
        acceptanceAreaRays.push_back((rays[i].transform(rotateBack)).transform(moveBack));
      }

      if (canvas->initted()) {
        // draw the polygon
        tempPoly = CGAL::transform(moveBack, tempPoly);

        bool notFirst = false;
        CGALPoint lastPoint;
        SDLCanvasShape* line;
        for (CGALPolygon::Vertex_const_iterator it = tempPoly.vertices_begin();
            it != tempPoly.vertices_end();
            it++) {
          if (notFirst) {
            line = canvas->makeLine(lastPoint.x(), lastPoint.y(), it->x(), it->y());
            line->setShapeColor(255,255,0);
            canvasObjs.push_back(line);
          }
          lastPoint = *it;
          notFirst = true;
        }
        // link last->first
        line = canvas->makeLine(lastPoint.x(), lastPoint.y(), 
                                tempPoly.vertices_begin()->x(), tempPoly.vertices_begin()->y());
        line->setShapeColor(255,255,0);
        canvasObjs.push_back(line);
      }
    }
    if (canvas->initted()) {
      // draw the rays
      int count = 1;
      for (list<CGALRay>::iterator it = acceptanceAreaRays.begin();
          it != acceptanceAreaRays.end();
          it++) {
        SDLCanvasShape* line =  canvas->makeLine(it->point(60).x(), it->point(60).y(),
                                                it->source().x(), it->source().y());
        line->setShapeColor(255,0,0);
        line->setLabel(SRS_catStrInt("s",count));
        canvasObjs.push_back(line);
        count++;
        if (count == 8) {
          count = 0;
        }
      }
      canvas->redraw();
    }
  }
  
  CGALPoint target = primaryObject->getCentroid();
  bool leftOfPrevious = false;
  int index = 0;
  list<CGALRay>::iterator it = acceptanceAreaRays.begin();
  bool stop = false;
  while (not stop) {
    if (it == acceptanceAreaRays.end()) {
      it = acceptanceAreaRays.begin();
      stop = true;
    }
    // these need to be in ccw order!
    if (CGAL::orientation(it->source(), it->point(1), target) == CGAL::LEFT_TURN) {
      leftOfPrevious = true;
    }
    else if (leftOfPrevious) { // include COLLINEAR as RIGHT_TURN
      return index;
    }

    ++index;
    if (index == 8) {
      index = 0;
    }
    ++it;
  }

  // the condition was never met, is the target inside the source?
  return -1;

}

int SRSShape::getAllocentricRelativeOrientationOf(SRSShape* primaryObject) {
  // return an integer corresponding to the orientation of the primary object 
  // relative to this reference object, ignoring the reference object's
  // orientation (like "a is north of b", instead of "a is in front of b")
  
  // to do this, keep a list of acceptance area rays for the source object
  // (ideally, looking like fig 4.21 in Hernandez 1994)

  if (allocentricAcceptanceAreaRays.size() == 0) {
    dbg << "generating allocentric acceptance area rays..\n";
    

    if (isCircle) { 
      // for now, just make 8 rays from the centroid
      // first, rotate the orientation right by pi/4 (so "straight ahead" surrounds
      // it)
      CGALTransformation rotate45(CGAL::ROTATION, SIN_PI_4, COS_PI_4);
      
      CGALDirection direction(-1*SIN_PI_8, COS_PI_8);
      for (int i=0; i<8; i++) {
        allocentricAcceptanceAreaRays.push_back(CGALRay(centroid, direction));
        direction = direction.transform(rotate45);
      }
    }
    else {
      CGALBBox box = polygon->bbox();

      allocentricAcceptanceAreaRays.push_back(
          CGALRay(CGALPoint(box.xmax(), box.ymax()), CGALDirection(COS_PI_8, SIN_PI_8)));
      allocentricAcceptanceAreaRays.push_back(
          CGALRay(CGALPoint(box.xmax(), box.ymax()), CGALDirection(COS_3PI_8, SIN_3PI_8)));
      allocentricAcceptanceAreaRays.push_back(
          CGALRay(CGALPoint(box.xmin(), box.ymax()), CGALDirection(COS_5PI_8, SIN_5PI_8)));
      allocentricAcceptanceAreaRays.push_back(
          CGALRay(CGALPoint(box.xmin(), box.ymax()), CGALDirection(COS_7PI_8, SIN_7PI_8)));
      allocentricAcceptanceAreaRays.push_back(
          CGALRay(CGALPoint(box.xmin(), box.ymin()), CGALDirection(COS_9PI_8, SIN_9PI_8)));
      allocentricAcceptanceAreaRays.push_back(
          CGALRay(CGALPoint(box.xmin(), box.ymin()), CGALDirection(COS_11PI_8, SIN_11PI_8)));
      allocentricAcceptanceAreaRays.push_back(
          CGALRay(CGALPoint(box.xmax(), box.ymin()), CGALDirection(COS_13PI_8, SIN_13PI_8)));
      allocentricAcceptanceAreaRays.push_back(
          CGALRay(CGALPoint(box.xmax(), box.ymin()), CGALDirection(COS_15PI_8, SIN_15PI_8)));
      
    }
    if (canvas->initted()) {
      // draw the rays
      int count = 1;
      for (list<CGALRay>::iterator it = allocentricAcceptanceAreaRays.begin();
          it != allocentricAcceptanceAreaRays.end();
          it++) {
        SDLCanvasShape* line =  canvas->makeLine(it->point(60).x(), it->point(60).y(),
                                                it->source().x(), it->source().y());
        line->setShapeColor(0,0,255);
        line->setLabel(SRS_catStrInt("s",count));
        canvasObjs.push_back(line);
        count++;
        if (count == 8) {
          count = 0;
        }
      }
      canvas->redraw();
    }
  }
  
  CGALPoint target = primaryObject->getCentroid();
  bool leftOfPrevious = false;
  int index = 0;
  list<CGALRay>::iterator it = allocentricAcceptanceAreaRays.begin();
  bool stop = false;
  while (not stop) {
    if (it == allocentricAcceptanceAreaRays.end()) {
      it = allocentricAcceptanceAreaRays.begin();
      stop = true;
    }
    // these need to be in ccw order!
    if (CGAL::orientation(it->source(), it->point(1), target) == CGAL::LEFT_TURN) {
      leftOfPrevious = true;
    }
    else if (leftOfPrevious) { // include COLLINEAR as RIGHT_TURN
      return index;
    }

    ++index;
    if (index == 8) {
      index = 0;
    }
    ++it;
  }

  // the condition was never met, is the target inside the source?
  return -1;

}

bool SRSShape::RCC_DR(SRSShape* target) {
  if (isCircle) {
    if (target->getIsCircle()) {
      return (not doIntersect(circle, target->getCircle()));
    }
    else {
      return (not doIntersect(circle, target->getPolygon()));
    }
  }
  else {
    if (target->getIsCircle()) {
      return (not doIntersect(target->getCircle(), polygon));
    }
    else {
      return (not CGAL::do_intersect(*polygon, *(target->getPolygon())));
    }
  }
}

bool SRSShape::RCC_PO(SRSShape* target) {
  return (not RCC_DR(target) and
          not RCC_PP(target) and
          not RCC_PPi(target) and
          not RCC_EQ(target));
}

bool SRSShape::RCC_PP(SRSShape* target) {
  // target is inside this
  if (not RCC_DR(target)) {
    CGALComputeSquaredDistance sqdist;
    if (isCircle) {
      if (target->getIsCircle()) {
        if (*target->getCircle() == *circle) {
          return false;
        }
        // circle is enclosed in circle if dist between centers plus small
        // radius is less than large radius
        return (
            (sqrt(sqdist(centroid, target->getCentroid())) + sqrt(target->getCircle()->squared_radius()))
            < sqrt(circle->squared_radius())
            );
      }
      else {
        // polygon is inside circle if every vertex is
        for (CGALPolygon::Vertex_const_iterator it = target->getPolygon()->vertices_begin();
             it != target->getPolygon()->vertices_end();
             it++) {
          if (not doIntersect(circle, &(*it))) {
            return false;
          }
        }
        return true;
      }
    }
    else {
      // this is a polygon
      if (target->getIsCircle()) {
        // circle is enclosed in polygon if the shapes intersect (which they
        // must, since we checked RCC_DR), but none of the
        // edges of the polygon intersect the circle
        for (CGALPolygon::Edge_const_iterator it = polygon->edges_begin();
             it != polygon->edges_end();
             it++) {
          CGALSegment seg = *it;
          if (doIntersect(target->getCircle(), &seg)) {
            return false;
          }
        }
        return true;
      }
      else {
        if (*target->getPolygon() == *polygon) {
          return false;
        }
        // polygon is enclosed in polygon if each vertex of target is enclosed
        // (assuming convex polygons)
        for (CGALPolygon::Vertex_const_iterator it = target->getPolygon()->vertices_begin();
             it != target->getPolygon()->vertices_end();
             it++) {
          if (polygon->has_on_unbounded_side(*it)) {
            return false;
          }
        }
        return true;
      }
    }
  }
  else {
    // RCC_DR true
    return false;
  }
}

bool SRSShape::RCC_PPi(SRSShape* target) {
  return target->RCC_PP(this);
}

bool SRSShape::RCC_EQ(SRSShape* target) {
  if (isCircle) {
    if (target->getIsCircle()) {
      return *circle == *(target->getCircle());
    }
    else {
      return false;
    }
  }
  else {
    if (not target->getIsCircle()) {
      return *polygon == *(target->getPolygon());
    }
    else {
      return false;
    }
  }
}

CGALSegment SRSShape::translateCentroid(double deltaT) {
  // project the centroid at the current speed and orientation
  // deltaT units (in the past or future)

  // I can't believe CGAL has no way of doing this..
  double angle = atan2(orientation.dy(), orientation.dx());
  double x,y;
  x = centroid.x() + deltaT*speed*cos(angle);
  y = centroid.y() + deltaT*speed*sin(angle);
  dbg << "centroidAtTime " << deltaT << " is " << x << "," << y << " (speed " << speed << ")\n";
  return CGALSegment(centroid, CGALPoint(x,y));
}

double SRSShape::getDistanceTo(SRSShape* target) {
  if (isCircle && target->getIsCircle()) {
    CGALSegment connector(centroid, target->getCentroid());
    double dist = (sqrt(connector.squared_length())
            - sqrt(circle->squared_radius())
            - sqrt(target->getCircle()->squared_radius()));
    if (dist > 0) {
      return dist;
    }
    else {
      return 0;
    }
  }
  else if (not isCircle and not target->getIsCircle()) {
    CGALPolytopeDistance pd(polygon->vertices_begin(), polygon->vertices_end(),
                            target->getPolygon()->vertices_begin(), target->getPolygon()->vertices_end());
    // this is smallest distance between the convex hulls of the vertices of
    // the polygons, if they are non-convex, this could be inaccurate
    
    // distance-point selection testing
    //dbg << "making test poly\n";
    //dbg << "sp p" << pd.number_of_support_points_p() << endl;
    //dbg << "sp q" << pd.number_of_support_points_q() << endl;
    //CGALPolygon poly(pd.support_points_q_begin(), pd.support_points_q_end());
    //poly.insert(poly.vertices_begin(), pd.support_points_p_begin(), pd.support_points_p_end());
    //drawPolygon(poly, 255,0,0);
    CGALPolytopeDistance::Coordinate_iterator ci = pd.realizing_point_p_coordinates_begin();
    CGALPolytopeDistance::Coordinate_iterator ci2 = pd.realizing_point_q_coordinates_begin();
    //canvas->makeLine(((*ci)/(*(ci+2))), ((*(ci+1))/(*(ci+2))), ((*ci2)/(*(ci2+2))), ((*(ci2+1))/(*(ci2+2))));
    
    //canvas->redraw();

    return sqrt(pd.squared_distance());
  }
  else if (isCircle and not target->getIsCircle()) {
    list<CGALPoint> circleCenter;
    circleCenter.push_back(centroid);
    CGALPolytopeDistance pd(circleCenter.begin(), circleCenter.end(),
                            target->getPolygon()->vertices_begin(), target->getPolygon()->vertices_end());
    
    double dist = sqrt(pd.squared_distance()) - sqrt(circle->squared_radius());
    if (dist > 0) {
      return dist;
    }
    else {
      return 0;
    }
  }
  else {
    // poly and target circle
    list<CGALPoint> circleCenter;
    circleCenter.push_back(target->getCentroid());
    CGALPolytopeDistance pd(circleCenter.begin(), circleCenter.end(),
                            polygon->vertices_begin(), polygon->vertices_end());
    double dist = sqrt(pd.squared_distance()) - sqrt(target->getCircle()->squared_radius());
    if (dist > 0) {
      return dist;
    }
    else {
      return 0;
    }
  }
}

CGALSegment SRSShape::getShortestLineTo(SRSShape* target) {
  if (isCircle && target->getIsCircle()) {
    CGALSegment connector(centroid, target->getCentroid());
    connector = shrinkSegment(&connector, sqrt(circle->squared_radius()), true);
    return shrinkSegment(&connector, sqrt(target->getCircle()->squared_radius()), false);
  }
  else if (not isCircle and not target->getIsCircle()) {
    CGALPolytopeDistance pd(polygon->vertices_begin(), polygon->vertices_end(),
                            target->getPolygon()->vertices_begin(), target->getPolygon()->vertices_end());
    // this is smallest distance between the convex hulls of the vertices of
    // the polygons, if they are non-convex, this could be inaccurate

    // make a segment from a p-point to a q-point
    assert(pd.points_p_begin() != pd.points_p_end());
    assert(pd.points_q_begin() != pd.points_q_end());
    return CGALSegment(*(pd.points_p_begin()), *(pd.points_q_begin()));
  }
  else if (isCircle and not target->getIsCircle()) {
    list<CGALPoint> circleCenter;
    circleCenter.push_back(centroid);
    CGALPolytopeDistance pd(circleCenter.begin(), circleCenter.end(),
                            target->getPolygon()->vertices_begin(), target->getPolygon()->vertices_end());
    // make a segment from a p-point to a q-point
    assert(pd.points_p_begin() != pd.points_p_end());
    assert(pd.points_q_begin() != pd.points_q_end());
    CGALSegment connector(*(pd.points_p_begin()), *(pd.points_q_begin()));
    return shrinkSegment(&connector, sqrt(circle->squared_radius()), true);
  }
  else {
    // poly and target circle
    list<CGALPoint> circleCenter;
    circleCenter.push_back(target->getCentroid());
    CGALPolytopeDistance pd(circleCenter.begin(), circleCenter.end(),
                            polygon->vertices_begin(), polygon->vertices_end());
    // make a segment from a p-point to a q-point
    assert(pd.points_p_begin() != pd.points_p_end());
    assert(pd.points_q_begin() != pd.points_q_end());
    CGALSegment connector(*(pd.points_p_begin()), *(pd.points_q_begin()));
    return shrinkSegment(&connector, sqrt(target->getCircle()->squared_radius()), false);
  }
}

CGALPolygon* SRSShape::getShortestDistanceRegionTo(SRSShape* target) {
  if (isCircle && target->getIsCircle()) {
    CGALSegment connector(centroid, target->getCentroid());
    connector = shrinkSegment(&connector, sqrt(circle->squared_radius()), true);
    return (newPolygonFromSegment(
            shrinkSegment(&connector, sqrt(target->getCircle()->squared_radius()), false)));
  }
  else if (not isCircle and not target->getIsCircle()) {
    // this is the interesting case, two polygons
    // they can have many closest-distance connectors, and we need to return a
    // region defining them. this region must be a rectangle, with two edges
    // connecting vertices of one polygon to edges of the other
    
    // to get the region, rely on the behavior of CGALPolytopeDistance to return
    // one of the extreme realizing lines with a given p and q, and the other
    // if p and q switch places. if the realizing lines are the same, the
    // result is a line, otherwise it is the region between the two lines 
    CGALPolytopeDistance pd1(polygon->vertices_begin(), polygon->vertices_end(),
                            target->getPolygon()->vertices_begin(), target->getPolygon()->vertices_end());
    
    CGALPolytopeDistance pd2(target->getPolygon()->vertices_begin(), target->getPolygon()->vertices_end(),
                             polygon->vertices_begin(), polygon->vertices_end());
    

    if (pd1.realizing_point_p() == pd2.realizing_point_q() &&
        pd1.realizing_point_q() == pd2.realizing_point_p()) {
      dbg << "polygon distance is shortest at only one place.\n";
      return newPolygonFromSegment(CGALSegment(pd1.realizing_point_p(), pd1.realizing_point_q()));
    }
    else {
      dbg << "polygon distance is shortest in a region.\n";
      list<CGALPoint> newPoly;
      newPoly.push_back(pd1.realizing_point_p());
      newPoly.push_back(pd1.realizing_point_q());
      newPoly.push_back(pd2.realizing_point_p());
      newPoly.push_back(pd2.realizing_point_q());

      CGALPolygon* poly = new CGALPolygon(newPoly.begin(), newPoly.end());
      if (poly->is_clockwise_oriented()) {
        poly->reverse_orientation();
      }
      if (not poly->is_counterclockwise_oriented()) {
        assert(false);
      }
      return poly;
    }
      
  }
  else if (isCircle and not target->getIsCircle()) {
    list<CGALPoint> circleCenter;
    circleCenter.push_back(centroid);
    CGALPolytopeDistance pd(circleCenter.begin(), circleCenter.end(),
                            target->getPolygon()->vertices_begin(), target->getPolygon()->vertices_end());
    // make a segment from a p-point to a q-point
    assert(pd.points_p_begin() != pd.points_p_end());
    assert(pd.points_q_begin() != pd.points_q_end());
    CGALSegment connector(*(pd.points_p_begin()), *(pd.points_q_begin()));
    return newPolygonFromSegment(shrinkSegment(&connector, sqrt(circle->squared_radius()), true));
  }
  else {
    // poly and target circle
    list<CGALPoint> circleCenter;
    circleCenter.push_back(target->getCentroid());
    CGALPolytopeDistance pd(circleCenter.begin(), circleCenter.end(),
                            polygon->vertices_begin(), polygon->vertices_end());
    // make a segment from a p-point to a q-point
    assert(pd.points_p_begin() != pd.points_p_end());
    assert(pd.points_q_begin() != pd.points_q_end());
    CGALSegment connector(*(pd.points_p_begin()), *(pd.points_q_begin()));
    return newPolygonFromSegment(
            shrinkSegment(&connector, sqrt(target->getCircle()->squared_radius()), false));
  }
}
      
string SRS_catStrInt(const char* str, int x) {
  // copied from general.cpp so this file can be used outside sorts
  ostringstream sstr;
  sstr << str << x;
  return sstr.str();
}

double SRSShape::getArea() {
  if (area == -1) {
    if (isCircle) {
      area = PI*(circle->squared_radius());
    }
    else {
      // area is signed, negative if polygon is oriented clockwise
      // we don't care about orientation, so abs it
      area = abs(polygon->area());
    }
  }
  return area;
}

bool SRSShape::isBiggerThan(SRSShape* target) {
  return getArea() > target->getArea();
}

void SRSShape::drawPolygon(CGALPolygon& polygon, int r, int g, int b) {
  dbg << "drawPoly\n";
  // only for debugging, there is no way to erase!
  SDLCanvasShape* line;
  for (CGALPolygon::Edge_const_iterator it = polygon.edges_begin();
        it != polygon.edges_end();
        it++) {
    dbg << "edge " << (*it).source() << " to " << (*it).target() << "\n";
    line = canvas->makeLine((*it).source().x(), (*it).source().y(), (*it).target().x(), (*it).target().y());
    line->setShapeColor(r,g,b);
  }
  canvas->redraw();
}
