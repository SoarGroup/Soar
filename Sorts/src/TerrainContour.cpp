#include "TerrainContour.h"
#include "Sorts.h"
#include <assert.h>

TerrainContour::TerrainContour(int x1, int y1, int x2, int y2, int x3, int y3, bool _open)
{
  vertices.push_back(Point_2(x1, y1));
  vertices.push_back(Point_2(x2, y2));
  vertices.push_back(Point_2(x3, y3));
  assert(!(x1==x2 && y1==y2) && !(x2==x3 && y2==y3) && !(x3==x1 && y3==y1));
  if (testReconstruct()) {
    reconstruct();
  }
  open = _open;

  bbox = Rectangle(x1, y1);
  bbox.accomodate(x2, y2);
  bbox.accomodate(x3, y3);
}

TerrainContour::TerrainContour(list<Point> cycle) {
  bbox = Rectangle(cycle.begin()->x, cycle.begin()->y);

  for(list<Point>::iterator
      i  = cycle.begin();
      i != cycle.end();
      ++i)
  {
    list<Point>::iterator j = i;
    ++j;
    if (j != cycle.end()) {
      Sorts::canvas.drawLine(i->x, i->y, j->x, j->y);
    }
    contour.push_back(Point_2(i->x, i->y));
    bbox.accomodate(i->x, i->y);
  }
  Sorts::canvas.update();

  assert(contour.is_simple());
  if (contour.orientation() == CGAL::NEGATIVE) {
    contour.reverse_orientation();
  }
  open = false;
}

void TerrainContour::reconstruct() {
  contour.clear();
  contour.insert(contour.vertices_begin(), vertices.begin(), vertices.end());
  
  assert(contour.is_simple());
  if (contour.orientation() == CGAL::NEGATIVE) {
    contour.reverse_orientation();
  }

/*
  if (!open) {
    Polygon::Vertex_iterator i = contour.vertices_begin();
    Polygon::Vertex_iterator j = i++;
    for(; i != contour.vertices_end(); ++i, ++j) {
      Sorts::canvas.drawLine
      ( CGAL::to_double(j->x()), 
        CGAL::to_double(j->y()), 
        CGAL::to_double(i->x()), 
        CGAL::to_double(i->y()) );
    }
  }
*/
}

bool TerrainContour::testReconstruct() {
  Polygon test(vertices.begin(), vertices.end());
  return test.is_simple();
}

bool TerrainContour::segmentBelongs(int x1, int y1, int x2, int y2) const {
  Point_2 p1(x1, y1);
  Point_2 p2(x2, y2);

  if (vertices.front() == p1) return true;
  if (vertices.front() == p2) return true;
  if (vertices.back()  == p1) return true;
  if (vertices.back()  == p2) return true;

  return false;
}

bool TerrainContour::canJoin(const TerrainContour& other) {
  if (vertices.front() == other.vertices.front()) return true; 
  if (vertices.front() == other.vertices.back()) return true; 
  if (vertices.back() == other.vertices.front()) return true; 
  if (vertices.back() == other.vertices.back()) return true; 

  return false;
}

// insert a segment into the contour, always keeping
// the points list ordered correctly
void TerrainContour::addSegment(int x1, int y1, int x2, int y2) {
  assert(open);

  Sorts::canvas.drawLine(x1, y1, x2, y2);
  Sorts::canvas.update();

  Point_2 p1(x1, y1);
  Point_2 p2(x2, y2);
 
  for(list<Point_2>::iterator
      i  = vertices.begin();
      i != vertices.end();
      ++i)
  {
    if (*i != vertices.front() && 
        *i != vertices.back() &&
        (*i == p1 || *i == p2))
    {
      return;
    }
  }

  assert(p1 != p2);

  if (vertices.front() == p1) {
    vertices.push_front(p2);
  }
  else if (vertices.front() == p2) {
    vertices.push_front(p1);
  }
  else if (vertices.back() == p1) {
    vertices.push_back(p2);
  }
  else if (vertices.back() == p2) {
    vertices.push_back(p1);
  }
  else {
    // the segment doesn't connect with the existing segments, so
    // it shouldn't belong in this contour
    assert(false);
  }

  if (vertices.front() == vertices.back()) {
    vertices.pop_front();
    open = false;
    assert(testReconstruct());
    reconstruct();
  }
  else {
    if (testReconstruct()) {
      reconstruct();
    }
    // otherwise wait until we get more points
  }

  bbox.accomodate(x1, y1);
  bbox.accomodate(x2, y2);
}

void TerrainContour::join(const TerrainContour& other) {
  assert(open);

  if (vertices.front() == other.vertices.front()) {
    vertices.pop_front();
    copy( other.vertices.begin(), 
          other.vertices.end(), 
          front_inserter(vertices) );
  }
  else if (vertices.back() == other.vertices.front()) {
    vertices.pop_back();
    copy( other.vertices.begin(), 
          other.vertices.end(), 
          back_inserter(vertices) );
  }
  else if (vertices.front() == other.vertices.back()) {
    vertices.pop_front();
    copy( other.vertices.rbegin(), 
          other.vertices.rend(), 
          front_inserter(vertices) );
  }
  else if (vertices.back() == other.vertices.back()) {
    vertices.pop_back();
    copy( other.vertices.rbegin(), 
          other.vertices.rend(), 
          back_inserter(vertices) );
  }
  else {
    // these contours don't connect
    assert(false);
  }

  if (vertices.front() == vertices.back()) {
    vertices.pop_front();
    open = false;
    assert(testReconstruct());
    reconstruct();
  }
  else {
    if (testReconstruct()) {
      reconstruct();
    }
  }
  bbox.accomodate(other.bbox);
}

bool TerrainContour::intersectsCircle(int cx, int cy, int r) const {
  Point_2 p[4]= { 
    Point_2(cx-r,cy-r),
    Point_2(cx+r,cy-r),
    Point_2(cx+r,cy+r),
    Point_2(cx-r,cy+r)};

  Polygon cpgn(p, p + 4);

  return CGAL::do_intersect(contour, cpgn);
}

bool TerrainContour::intersectsRectangle(int x1,int y1,int x2,int y2) const {
  Point_2 p[4]={Point_2(x1,y1),Point_2(x2,y1),Point_2(x2,y2),Point_2(x1,y2)};
  Polygon rect(p, p+4);
  return CGAL::do_intersect(contour, rect);
}

bool TerrainContour::intersectsContour(const TerrainContour& other) const {
  return CGAL::do_intersect(contour, other.contour);
}

