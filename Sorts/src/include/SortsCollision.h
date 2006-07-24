#ifndef Collision_H_
#define Collision_H_

#include "Vec2d.h"
#include "Rectangle.h"
#include "Circle.h"

bool oneDimIntersect(const Vec2d& a, const Vec2d& b);

Rectangle triangleAABB(const Vec2d& ra, double xoffset, double yoffset);

Vec2d polygonProjectionBounds(const Vec2d& axis, int dim, Vec2d* p, int n);
Vec2d circleProjectionBounds(const Vec2d& axis, int dim, const Circle& c);

bool circle_triangle_intersect
( const Circle& c,
  const Vec2d& ra,
  double xoffset,
  double yoffset );

bool rectangle_triangle_intersect
( const Rectangle& r,
  const Vec2d& ra,
  double xoffset,
  double yoffset );

bool rectangle_circle_intersect
( const Rectangle& r,
  const Circle& c );

#endif
