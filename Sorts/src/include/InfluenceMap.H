#ifndef InfluenceMap_H
#define InfluenceMap_H

// $Id: InfluenceMap.H,v 1.2 2007/05/23 23:32:18 orts_furtak Exp $

// This is an ORTS file (c) Tim Furtak, licensed under the GPL

#include "Global.H"
#include "EventHandler.H"
#include "ClientModule.H"
#include "PlayerInfo.H"
#include "LinAlg.H"
#include "DrawOnTerrain2D.H"

class Game;
class InfluenceMap {
protected:
  
  sint4 points_x, points_y;
  sint4 cell_size;
  sint4 cells_x, cells_y, cell_num;

  Vector<real4> data;

public:  
  InfluenceMap(sint4 px, sint4 py, sint4 cell_size);
  InfluenceMap();

  void init(sint4 px, sint4 py, sint4 cell_size);
  void init_game(const Game *game, sint4 cid);
  void init_map(const Game *game);
  void refresh_units(const Game *game, sint4 cid);

  void clear_all();
  void clear_units();
  void make_unwalkable(sint4 px, sint4 py);
  void make_unwalkable(sint4 px1, sint4 px2, sint4 py1, sint4 py2);

  void add_gaussian(real4 cx, real4 cy, real4 s, real4 r, real4 val);
  void add_circle(real4 cx, real4 cy, real4 r, real4 val);
  void add_aacircle(real4 cx, real4 cy, real4 r, real4 val);

  void invert();
  void print();
  void draw(DrawOnTerrain2D *dot);

  ScalarPoint get_highest();
  sint4 get_walkable();
  sint4 get_width() const { return cells_x; }
  sint4 get_height() const { return cells_y; }
  sint4 get_width_pixels() const { return cells_x * cell_size; }
  sint4 get_height_pixels() const { return cells_y * cell_size; }

  sint4 ind2x(sint4 ind) const { assert(ind >= 0 && ind < cell_num); return ind % cells_x; }
  sint4 ind2y(sint4 ind) const { assert(ind >= 0 && ind < cell_num); return ind / cells_x; }  
  sint4 xy2ind(sint4 x, sint4 y) const {
    assert(x >= 0 && x < cells_x);
    assert(y >= 0 && y < cells_y);
    return y * cells_x + x;
  }
  real4 value_at(sint4 x, sint4 y) const {
    return get_cell(x / cell_size, y / cell_size);
  }
  real4& get_cell(sint4 ind) { assert(ind >= 0 && ind < cell_num); return data[ind]; }
  const real4& get_cell(sint4 ind) const { assert(ind >= 0 && ind < cell_num); return data[ind]; }

  real4& get_cell(sint4 x, sint4 y) { return get_cell(xy2ind(x, y)); }
  const real4& get_cell(sint4 x, sint4 y) const { return get_cell(xy2ind(x, y)); }

  real4& operator()(sint4 ind) { return get_cell(ind); }
  const real4& operator()(sint4 ind) const { return get_cell(ind); }

  real4& operator()(sint4 x, sint4 y) { return get_cell(x, y); }
  const real4& operator()(sint4 x, sint4 y) const { return get_cell(x, y); }
};
#endif
