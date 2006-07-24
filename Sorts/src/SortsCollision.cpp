#include "SortsCollision.h"
#include <limits>
#include "Rectangle.h"
#include "Circle.h"
#include "Vec2d.h"

// determines if two one dimensional segments intersect
bool oneDimIntersect (const Vec2d& a, const Vec2d& b) {
  return a(0) <= b(1) && a(1) >= b(0);
}

// project a polygon onto a unit axis, returning a vector
// w that contains the lower and upper bound values in the
// dim dimension
Vec2d polygonProjectionBounds
( const Vec2d& axis,
  int dim,
  Vec2d* p,
  int n )
{
  double min = numeric_limits<double>::infinity();
  double max = -1 * min;
  for(int i = 0; i < n; ++i) {
    double proj = p[i].projUnit(axis, dim);
    if (proj < min) {
      min = proj;
    }
    if (proj > max) {
      max = proj;
    }
  }
  return Vec2d(min, max);
}

Rectangle triangleAABB
( const Vec2d& ra,
  double xoffset,
  double yoffset )
{
  double tMinX, tMinY, tMaxX, tMaxY;
  if (xoffset > 0) {
    tMinX = ra(0);
    tMaxX = ra(0) + xoffset;
  }
  else {
    tMinX = ra(0) + xoffset;
    tMaxX = ra(0);
  }
  if (yoffset > 0) {
    tMinY = ra(1);
    tMaxY = ra(1) + yoffset;
  }
  else {
    tMinY = ra(1) + yoffset;
    tMaxY = ra(1);
  }

  return Rectangle(tMinX, tMaxX, tMinY, tMaxY);
}


// project a circle onto a unit axis, returning a vector
// w that contains the lower and upper bound values in the
// dim dimension
Vec2d circleProjectionBounds
( const Vec2d& axis,
  int dim,
  const Circle& c )
{
  double projCenter = c.center().projUnit(axis, dim);
  double radiusOffset = c.r * axis(dim);
  if (radiusOffset > 0) {
    return Vec2d(projCenter - radiusOffset, projCenter + radiusOffset);
  }
  return Vec2d(projCenter + radiusOffset, projCenter - radiusOffset);
}

bool circle_triangle_intersect
( const Circle& c,
  const Vec2d& ra,
  double xoffset,
  double yoffset )
{
  // check axis aligned bounding boxes first
  if (!triangleAABB(ra, xoffset, yoffset).intersects(c.aabb())) {
    return false;
  }

  // now check other separating axes

  // the triangle as a polygon
  Vec2d tp[3] = { Vec2d(ra(0) + xoffset, ra(1)),
                  Vec2d(ra(0), ra(1) + yoffset),
                  ra };

  Vec2d axes[4] = { // axes between center of circle and triangle vertices
                    (tp[0] - c.center()).norm(), 
                    (tp[1] - c.center()).norm(), 
                    (tp[2] - c.center()).norm(), 
                    // normal of the hypotenuse
                    (tp[0] - tp[1]).rhNormal().norm() };

  for(int i = 0; i < 4; ++i) {
    int dim;
    if (axes[i](0) == 0) {
      dim = 1;
    }
    else {
      dim = 0;
    }
    Vec2d s1 = circleProjectionBounds(axes[i], dim, c);
    Vec2d s2 = polygonProjectionBounds(axes[i], dim, tp, 3);

    if (!oneDimIntersect(s1, s2)) { return false; }
  }

  return true;
}

bool rectangle_triangle_intersect
( const Rectangle& r,
  const Vec2d& ra,
  double xoffset,
  double yoffset )
{
  if (!triangleAABB(ra, xoffset, yoffset).intersects(r)) {
    return false;
  }

  Vec2d p1(ra(0) + xoffset, ra(1));
  Vec2d p2(ra(0), ra(1) + yoffset);

  Vec2d hypNormal = (p1 - p2).rhNormal().norm();

  int dim;
  if (hypNormal(0) == 0) {
    dim = 1;
  }
  else {
    dim = 0;
  }

  Vec2d rectPoly[4] = { Vec2d(r.xmin, r.ymin), 
                        Vec2d(r.xmin, r.ymax), 
                        Vec2d(r.xmax, r.ymax),
                        Vec2d(r.xmax, r.ymin) };

  Vec2d triPoly[3] = { ra, p1, p2 };

  Vec2d seg1 = polygonProjectionBounds(hypNormal, dim, rectPoly, 4);
  Vec2d seg2 = polygonProjectionBounds(hypNormal, dim, triPoly, 3);

  return oneDimIntersect(seg1, seg2);
}

bool rectangle_circle_intersect
( const Rectangle& r,
  const Circle& c )
{
  if (r.xmin <= c.x && c.x <= r.xmax) {
    return (r.ymin - c.r <= c.y && c.y <= r.ymax + c.r);
  }
  if (r.ymin <= c.y && c.y <= r.ymax) {
    return (r.xmin - c.r <= c.x && c.x <= r.xmax + c.r);
  }
  if (c.x < r.xmin) {
    if (c.y < r.ymin) {
      return c.contains(r.xmin, r.ymin);
    }
    return c.contains(r.xmin, r.ymax);
  }
  else { // c.x > xmax
    if (c.y < r.ymin) {
      return c.contains(r.xmax, r.ymin);
    }
    return c.contains(r.xmax, r.ymax);
  }
}
