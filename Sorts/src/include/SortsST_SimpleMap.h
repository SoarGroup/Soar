#ifndef Sortssmap_h
#define Sortssmap_h

// $Id: Demo_ST_SimpleMap.H,v 1.3 2006/05/10 12:34:51 ddeutscher Exp $

// This is an ORTS file 
// (c) Michael Buro, Sami Wagiaalla
// licensed under the GPL

#include "SortsSimpleTerrain.h"
#include "Global.H"

namespace SortsSimpleTerrain
{
  enum CellFlag { OPEN, TEMP, CLOSED }; // open, temporarily closed, open

  struct PACKED_STRUCT SizeCell
  {
    //TerrainBase::ObjId obj_id;   // id of the object occupying this cell
    uint1 flags;
    sint1 max_size; // maximum radius of objects with center on this (sub)tile
    sint4 cost;

    SizeCell() {
      flags = OPEN;
      max_size = 0;
      cost = 0;
    }

    void write(std::ostream &os = std::cout) const {
      os << sint4(max_size);
      if (flags) os << '*'; else os << ' ';
    }

  };

  struct PACKED_STRUCT AStarCell
  {
    uint4 parent;       // path predecessor
    uint4 open, closed; // set membership: in set <=> (value == magic)
    uint4 g;
  
    AStarCell() { parent = 0; open = closed = g = 0; }
  };


  struct HeapElement
  {
    uint4 cell; // index
    uint4 f, g; // can't use cell->g because otherwise heap gets corrupted
    // when cell->g is changed and used for heap element comparison

    HeapElement() { cell = 0; f = g = 0; }
  };


  struct HeapElementCmp
  {
    bool operator()(HeapElement &h1, HeapElement &h2) const {
      sint4 d = h1.f - h2.f;
      if (d != 0) return d > 0;
      return h1.g < h2.g;
    }

  };


  // mailbox representation with boundary (width->width+2, height->height+2)

  template <typename T> struct SortsSimpleMap {

  public:

    SortsSimpleMap(sint4 w_, sint4 h_) : w(w_), h(h_)
    { 
      assert(w >= 0 && h >= 0);
    
      d[0] =  1; d[1] = 1+w;  d[2] =  w; d[3] = w-1;
      d[4] = -1; d[5] = -1-w; d[6] = -w; d[7] = 1-w;
    
      n = w * h;
      cells.reserve(n);
      T c = T();
      FORT (i, n) cells.push_back(c);
      magic = 0;
    }
  
    sint4 get_d(sint4 i) const { return d[i]; }
    T &operator()(sint4 i) { return cells[i]; }
    const T &operator()(sint4 i) const { return cells[i]; }

    // important: add 1 to both when using original coordinates  
    T &operator()(sint4 x, sint4 y) { return cells[index(x, y)]; }
    const T &operator()(sint4 x, sint4 y) const { return cells[index(x, y)]; }  

    sint4 get_n() const { return n; }
    sint4 get_w() const { return w; }
    sint4 get_h() const { return h; }  
  
    void write(std::ostream &os = std::cout) const;

    inline sint4 i2x(sint4 index) const { return index % w; }
    inline sint4 i2y(sint4 index) const { return index / w; }

    inline sint4 index(sint4 x, sint4 y) const {
      assert(x >= 0 && x < w);
      assert(y >= 0 && y < w);    
      return y*w + x;
    }

    Vector<T> &get_cells() { return cells; }

    uint4 get_magic() const { return magic; }
    void  set_magic(uint4 m) { magic = m; }
    
  private:
  
    uint4 magic; // used for set membership test
    sint4 w, h, n;
    Vector<T> cells;
    sint4 d[8];
  };

}

REGISTER_TYPEOF(2701,Vector<SortsSimpleTerrain::SizeCell>::iterator);
REGISTER_TYPEOF(2702,Vector<SortsSimpleTerrain::SizeCell>::const_iterator);

#endif
