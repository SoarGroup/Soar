// $Id: Demo_ST_PFEngine.C,v 1.4 2006/05/10 12:34:51 ddeutscher Exp $

// This is an ORTS file 
// (c) Michael Buro, Sami Wagiaalla, David Deutscher
// licensed under the GPL

#include "Demo_ST_PFEngine.H"
#include "Options.H"
#include <iostream>

using namespace std;
using boost::array;

namespace Demo_SimpleTerrain
{

  //-----------------------------------------------------------------------------
  ST_Terrain::PFEngine::PFEngine(sint4 tiles_x_, sint4 tiles_y_, sint4 tile_points_, int gran)
    : map(tiles_x_ * gran + 1, tiles_y_ * gran + 1),
      air_map(tiles_x_ * gran + 1, tiles_y_ * gran + 1)
  {    
    init_sizes(map);
    init_sizes(air_map);
    subtile_points = tile_points_/gran;
    disp = subtile_points/2;
  }

  //-----------------------------------------------------------------------------
  bool ST_Terrain::PFEngine::find_path(const TerrainBase::Loc &start,
                                       const TerrainBase::Loc &goal, 
                                       TerrainBase::Path& output)
  {
    SimpleMap<AStarCell> amap(map.get_w(), map.get_h());
    MapPtr m = &map;
    Vector<TerrainBase::Loc> path;
    Vector<TerrainBase::Loc> clear;

    sint4 x1, y1;
    x1 = start.x;
    y1 = start.y;
    x1 = world2x(x1);
    y1 = world2y(y1);
    sint4 x2 = world2x(goal.x);
    sint4 y2 = world2y(goal.y);

    // constrain goal to inside the map
    x2 = min(max(x2,(sint4)1), map.get_w()-2);
    y2 = min(max(y2,(sint4)1), map.get_h()-2);

    bool found = false;
    int r = 3; // for workers
    int newr = r / subtile_points;
    if( r%subtile_points >= subtile_points/2 )
      newr++;
    Bucket b = BucketFactory::get_bucket(newr);
    b.set_center(TerrainBase::Loc(x1, y1));

    // clear current location (ignoring the target object)
    clear_location(b, *m, clear);

    // now find a path
    cout << "FINDING PATHS" << endl;
    found = simple_astar(*m, amap, b, x2, y2);

    // and return this object

    if (!found) {
      cout << "no path found" << endl;
    } else {
      uint4 cell = amap.index(x2, y2);

      // set flags along path and accumulate path length
      //    sint4 x1, y1, x2, y2;
      path.push_back( goal );
      //    path.push_back(TerrainBase::Loc(x2world(x2),y2world(y2)));
      //    cout << "[G]" << "\t: " << "X: "<< goal.x << " Y: " <<  goal.y << endl;
      do {
        map(cell).flags = 1;
        uint4 p = amap(cell).parent;
        if (p) {
          x2 = x2world(map.i2x(p));
          y2 = y2world(map.i2y(p));

          path.push_back(TerrainBase::Loc(x2,y2));
          //       uint4 i = path.size()-1;
          //       if (i > 1) cout << "[" << i << "]\t: " << "X: "<< path[i].x << " Y: " <<  path[i].y << endl;
        }
        cell = p;
      } while(cell);

      // remove the first "goal" which is the current location
      if (path.size() > 0)
        path.erase(path.end()-1);

      //add the part to clear current location
      FORALL (clear, i) { 
        path.push_back(*i);
      }
      //    cout << "[S]" << "\t: " << "X: "<< obj->get_int("x") << " Y: " <<  obj->get_int("y") << endl;
      if (path.size() > 0) { 
        smoothen_path(path);
      }
    }

    output.locs = path;
    //output.id = ? // fixme: output.id should be a PathId
    // fixme: is this correct? what if path.size() == 0 ?
    return found;
  }
  bool ST_Terrain::PFEngine::find_path(const Object *obj,
                                       const TerrainBase::Loc &goal, 
                                       TerrainBase::Path& output)
  {
    SimpleMap<AStarCell> amap(map.get_w(), map.get_h());
    MapPtr m = (obj->get_zcat() == Object::ON_LAND)? &map : &air_map;
    Vector<TerrainBase::Loc> path;
    Vector<TerrainBase::Loc> clear;

    sint4 x1, y1; obj->get_center(x1, y1);
    x1 = world2x(x1);
    y1 = world2y(y1);
    sint4 x2 = world2x(goal.x);
    sint4 y2 = world2y(goal.y);

    // constrain goal to inside the map
    x2 = min(max(x2,(sint4)1), map.get_w()-2);
    y2 = min(max(y2,(sint4)1), map.get_h()-2);

    bool found = false;
    int r = 3;//obj->get_radius();
    int newr = r / subtile_points;
    if( r%subtile_points >= subtile_points/2 )
      newr++;
    Bucket b = BucketFactory::get_bucket(newr);
    b.set_center(TerrainBase::Loc(x1, y1));

    // clear current location (ignoring the target object)
    remove_object(obj);
    clear_location(b, *m, clear);

    // now find a path
    cout << "FINDING PATHS" << endl;
    found = simple_astar(*m, amap, b, x2, y2);

    // and return this object
    insert_object(obj);

    if (!found) {
      cout << "no path found" << endl;
    } else {
      uint4 cell = amap.index(x2, y2);

      // set flags along path and accumulate path length
      //    sint4 x1, y1, x2, y2;
      path.push_back( goal );
      //    path.push_back(TerrainBase::Loc(x2world(x2),y2world(y2)));
          //cout << "[G]" << "\t: " << "X: "<< goal.x << " Y: " <<  goal.y << endl;
      do {
        map(cell).flags = 1;
        uint4 p = amap(cell).parent;
        if (p) {
          x2 = x2world(map.i2x(p));
          y2 = y2world(map.i2y(p));

          path.push_back(TerrainBase::Loc(x2,y2));
                 uint4 i = path.size()-1;
                 //if (i > 1) cout << "[" << i << "]\t: " << "X: "<< path[i].x << " Y: " <<  path[i].y << endl;
        }
        cell = p;
      } while(cell);

      // remove the first "goal" which is the current location
      if (path.size() > 0)
        path.erase(path.end()-1);

      //add the part to clear current location
      FORALL (clear, i) { 
        path.push_back(*i);
      }
          //cout << "[S]" << "\t: " << "X: "<< obj->get_int("x") << " Y: " <<  obj->get_int("y") << endl;
      if (path.size() > 0) { 
        smoothen_path(path);
      }
    }

    output.locs = path;
    //output.id = ? // fixme: output.id should be a PathId
    // fixme: is this correct? what if path.size() == 0 ?
    return found;
  }

  //-----------------------------------------------------------------------------
  bool ST_Terrain::PFEngine::simple_astar(SimpleMap<SizeCell> &smap, SimpleMap<AStarCell> &amap,
    Bucket b, sint4 x2, sint4 y2)
  {
    sint4 x1 = b.get_center().x;
    sint4 y1 = b.get_center().y;

    // maps must be compatible
    if (amap.get_w() != smap.get_w() || amap.get_h() != smap.get_h()) {
      cout << "amap\tw:" << amap.get_w() << "\th:" << amap.get_h() << endl;
      cout << "smap\tw:" << smap.get_w() << "\th:" << smap.get_h() << endl;    
      ERR("map dimensions don't match");
    }

    // reset open/closed set membership variables when "magic" is 1 after increment
    uint4 magic = amap.get_magic();

    ++magic;

    if (magic >= 1000000000) magic = 1; // wrap around

    if (magic == 1) {  // reset open/closed values
      FORS (i, amap.get_n()) amap(i).open = amap(i).closed = 0;
    }

    amap.set_magic(magic);

    // distances to the eight neighbour cells
    uint4 cost[8];

    cost[0] = FACTOR; cost[1] = DIAG;
    cost[2] = FACTOR; cost[3] = DIAG;
    cost[4] = FACTOR; cost[5] = DIAG;
    cost[6] = FACTOR; cost[7] = DIAG;

    priority_queue<HeapElement, vector<HeapElement>, HeapElementCmp> open;
    AStarCell &s = amap(x1, y1);

    //  assert(size >= 1);
    //  assert(smap(x1, y1).max_size >= size);

    //  if (smap(x2, y2).max_size < size) return false; // goal not reachable

    HeapElement hs;
    hs.cell = amap.index(x1, y1);
    hs.g = 0;
    hs.f = estimate(x1, y1, x2, y2);
    open.push(hs);

    //cout << "est=" << hs.f << endl;

    s.parent = 0; // no predecessor
    s.open = magic;
    s.g = 0;

    const uint4 goal = amap.index(x2, y2);

    size_t max_heap = 0;
    bool found = false;
    uint4 count = 0;

    while (!open.empty()) {

      ++count;
      HeapElement he = open.top(); open.pop();
      if (amap(he.cell).g != he.g) continue; // skip if he no longer up-to-date

      if (he.cell == goal) { found = true; break; }  // expand goal => path found
      b.set_center(TerrainBase::Loc(smap.i2x(he.cell), smap.i2y(he.cell)));

      HeapElement hen;

      // Check all directions
      bitset<8> freeDirection;
      array<sint4, 8> moveCosts;
      b.check_all_directions(smap, freeDirection, moveCosts);

      FORS (i, 8) {
        //      if (i%2) continue;

//        sint4 moveCost = 0;
//        if (!b.check_direction(smap, i, moveCost)) continue;
        if (!freeDirection[i]) continue;

        uint4 nn_index = he.cell + smap.get_d(i); // neighbour

        // cout << int(nn->max_size) << " " << int(size) << endl;

        AStarCell &nn = amap(nn_index);
//        uint4 newg = he.g + cost[i] + moveCost;
        uint4 newg = he.g + cost[i] + moveCosts[i];

        if ((nn.open == magic || nn.closed == magic) && nn.g <= newg) continue;

        nn.parent = he.cell;
        nn.g = newg;

        hen.cell = nn_index;
        hen.g = newg;
        hen.f = newg + estimate(smap.i2x(nn_index), smap.i2y(nn_index), x2, y2);

#if 0
        cout << "new g=" << hen.g
          << " h=" << estimate(smap.i2x(nn_index), smap.i2y(nn_index), x2, y2)
          << " f=" << hen.f
          << endl;
#endif

        if (nn.closed == magic) nn.closed = 0;
        open.push(hen); nn.open = magic;
        max_heap = max(open.size(), max_heap);
      }

      amap(he.cell).closed = magic;
    }

    return found;
  }

  //-----------------------------------------------------------------------------
  /**
  check the current location of the bucket if it is occupied by
  something else, move away from it.
  */
  void ST_Terrain::PFEngine::clear_location(Bucket &b, const SimpleMap<SizeCell> &smap,
    Vector<TerrainBase::Loc> &path)
  {
    path.clear();

    SimpleMap<AStarCell> amap(smap.get_w(), smap.get_h());
    sint4 x1 = b.get_center().x;
    sint4 y1 = b.get_center().y;

    // maps must be compatible
    if (amap.get_w() != smap.get_w() || amap.get_h() != smap.get_h()) {
      cout << "amap\tw:" << amap.get_w() << "\th:" << amap.get_h() << endl;
      cout << "smap\tw:" << smap.get_w() << "\th:" << smap.get_h() << endl;    
      ERR("map dimensions don't match");
    }

    // reset open/closed set membership variables when "magic" is 1 after increment
    uint4 magic = amap.get_magic();

    ++magic;

    if (magic >= 1000000000) magic = 1; // wrap around

    if (magic == 1) {  // reset open/closed values
      FORS (i, amap.get_n()) amap(i).open = amap(i).closed = 0;
    }

    amap.set_magic(magic);

    // distances to the eight neighbour cells
    uint4 cost[8];

    cost[0] = FACTOR; cost[1] = DIAG;
    cost[2] = FACTOR; cost[3] = DIAG;
    cost[4] = FACTOR; cost[5] = DIAG;
    cost[6] = FACTOR; cost[7] = DIAG;

    priority_queue<HeapElement, vector<HeapElement>, HeapElementCmp> open;
    AStarCell &s = amap(x1, y1);

    HeapElement hs;
    hs.cell = amap.index(x1, y1);
    hs.g = 0;
    hs.f = 0;//estimate(x1, y1, x2, y2);
    open.push(hs);

    //cout << "est=" << hs.f << endl;

    s.parent = 0; // no predecessor
    s.open = magic;
    s.g = 0;

    size_t max_heap = 0;
    bool found = false;
    uint4 count = 0;

    while (!open.empty()) {

      ++count;
      HeapElement he = open.top(); open.pop();
      if (amap(he.cell).g != he.g) continue; // skip if he no longer up-to-date

      sint4 stayCost = 0;
      b.check_vector(smap, b.get_points(), 0, 0, stayCost);
      if (stayCost == 0) { found = true; break; } // expand goal => path found

      b.set_center(TerrainBase::Loc(smap.i2x(he.cell), smap.i2y(he.cell)));

      HeapElement hen;

      // Check all directions
      bitset<8> freeDirection;
      array<sint4, 8> moveCosts;
      b.check_all_directions(smap, freeDirection, moveCosts);

      FORS (i, 8) {
        //      if (i%2) continue;

//        sint4 moveCost = 0;
//        if (!b.check_direction(smap, i, moveCost)) continue;
        if (!freeDirection[i]) continue;

        uint4 nn_index = he.cell + smap.get_d(i); // neighbour
           //   if (smap(nn_index).max_size < size) continue; // object does not fit

        // cout << int(nn->max_size) << " " << int(size) << endl;

        AStarCell &nn = amap(nn_index);
//        uint4 newg = he.g + cost[i] + moveCost;
        uint4 newg = he.g + cost[i] + moveCosts[i];

        if ((nn.open == magic || nn.closed == magic) && nn.g <= newg) continue;

        nn.parent = he.cell;
        nn.g = newg;

        hen.cell = nn_index;
        hen.g = newg;
        hen.f = newg; //+ estimate(smap.i2x(nn_index), smap.i2y(nn_index), x2, y2);

#if 0
        cout << "new g=" << hen.g
          << " h=" << estimate(smap.i2x(nn_index), smap.i2y(nn_index), x2, y2)
          << " f=" << hen.f
          << endl;
#endif

        if (nn.closed == magic) nn.closed = 0;
        open.push(hen); nn.open = magic;
        max_heap = max(open.size(), max_heap);
      }

      amap(he.cell).closed = magic;
    }

    if (found) {

      sint4 x2 = b.get_center().x;
      sint4 y2 = b.get_center().y;

      uint4 cell = amap.index(x2, y2);

      do {
        map(cell).flags = 1;

        uint4 p = amap(cell).parent;
        if (p) {
          x2 = x2world(map.i2x(p));
          y2 = y2world(map.i2y(p));        
          path.push_back(TerrainBase::Loc(x2, y2));
        }
        cell = p;

      } while (cell);

      if (!path.empty()) path.erase(path.end()-1);
    }
  }

  //-----------------------------------------------------------------------------
  /**
  * go throught the waypoints in the given vector and remove
  * all redundent points that fall on a straight line
  * */
  void ST_Terrain::PFEngine::smoothen_path(Vector<TerrainBase::Loc> &path)
  {
    uint4 i1 = 0;
    uint4 i2 = 1;

    for (uint4 i = 2; i < path.size(); i++) {

      while (i < path.size() && on_line(path[i1], path[i2], path[i])) {
        path.erase(path.begin()+i2);
      }

      i1 = i-1;
      i2 = i;
    }
  }

  //-----------------------------------------------------------------------------
  uint4 ST_Terrain::PFEngine::estimate(sint4 x1, sint4 y1, sint4 x2, sint4 y2)
  {
    /*
    *  -----
    *       \   
    *        \ 
    *
    * Octile distance: 
    *   dmin * sqrt(2) + dmax - dmin
    */

    sint4 dx = abs(x1-x2);
    sint4 dy = abs(y1-y2);
    sint4 dmin = min(dx, dy);
    return dmin * DIAG + (dx + dy - dmin - dmin)*FACTOR;

    // Euclidean
    //return sint4(sqrt(dx*dx + dy*dy)*FACTOR);

    // Manhattan
    // return max(dx, dy)*FACTOR;
  }

  //-----------------------------------------------------------------------------
  bool ST_Terrain::PFEngine::on_line(const TerrainBase::Loc &a, const TerrainBase::Loc &b, const TerrainBase::Loc &c)
  {
    if (a.x == b.x && b.x == c.x) { return true; }
    if (a.y == b.y && b.y == c.y) { return true; }
    double tx = (double)(b.x - a.x) / ((double)(c.x - a.x));
    double ty = (double)(b.y - a.y) / ((double)(c.y - a.y));
    return tx >=0 && tx <= 1 && tx == ty;
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::PFEngine::insert_object(const Object *obj)
  {    
    ir_object(obj, true);
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::PFEngine::remove_object(const Object *obj)
  {
    ir_object(obj, false);
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::PFEngine::update_object(const Object *obj)
  {
    // remove previous values
    ir_object(obj, false, true);
    // insert new values
    ir_object(obj, true, false);
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::PFEngine::insert_boundary(const TerrainBase::Segment& l)
  {
    bool costonly;
    Options::get("-costonly", costonly);
    if (costonly) {
      insert_line(world2x(l.l1.x), world2x(l.l1.y), world2x(l.l2.x), world2x(l.l2.y), 
                  Object::ON_LAND, CLOSED, 0, BOUND_COST);
    } else {
      insert_line(world2x(l.l1.x), world2x(l.l1.y), world2x(l.l2.x), world2x(l.l2.y), 
                  Object::ON_LAND, CLOSED, 1, 0);
    }
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::PFEngine::ir_object(const Object *obj, bool insert, bool use_prev_attributes)
  {
    sint4 cost = insert ? 1 : -1;

    if (obj->get_shape() == Object::RECTANGLE) {
      CellFlag f = (insert)? TEMP : OPEN;
      TerrainBase::Loc topleft, bottomright;
      Object::ZCat zcat;

      if( !use_prev_attributes ) {
        obj->get_p1(topleft.x, topleft.y);
        obj->get_p2(bottomright.x, bottomright.y);
        zcat = obj->get_zcat();
      } else {
        // get previous attribute values
        sint4 dummy1, dummy2;
        obj->get_p1(dummy1, dummy2, &(topleft.x), &(topleft.y));
        obj->get_p2(dummy1, dummy2, &(bottomright.x), &(bottomright.y));
        obj->get_zcat(&zcat);
      }

      insert_rectangle(topleft, bottomright, zcat, f, 0*cost, cost*UNIT_COST);

    } else if (obj->get_shape() == Object::CIRCLE) {

      sint4 r = obj->get_radius();
      TerrainBase::Loc c;
      Object::ZCat zcat;

      if( !use_prev_attributes ) {
        obj->get_center(c.x, c.y);
        zcat = obj->get_zcat();
      } else {
        // get previous attribute values
        sint4 dummy1, dummy2;
        obj->get_center(dummy1, dummy2, &(c.x), &(c.y));
        obj->get_zcat(&zcat);
      }

      int newr = r / subtile_points;
      if (r % subtile_points >= subtile_points / 2) newr++;
      Bucket b = BucketFactory::get_bucket(newr);
      Bucket b2 = BucketFactory::get_bucket(newr-1);
      Loc cworld(world2x(c.x), world2y(c.y));

      b.set_center(cworld);
      //insert_bucket(b, zcat, TEMP, 1, cost*UNIT_COST);
      insert_bucket(b, zcat, TEMP, 0, cost*UNIT_COST);

      b2.set_center(cworld);
      //insert_bucket(b2, zcat, TEMP, 1, cost*UNIT_COST/2);
      insert_bucket(b2, zcat, TEMP, 0, cost*UNIT_COST/2);

      // fixme: heading is not accessible through Object
      /* We wanted to add some cost to the position where the object will be
      in the next tick to prefer not to go there; However, heading is not
      currently accessible with the new TerrainBase interface:
      // one second distance
      Loc head = cworld;
      Loc tail = cworld;
      get_head_and_tail(head, tail, heading);

      b.set_center(head);
      insert_bucket(b, zcat, TEMP, 0, id, cost*CROSS_COST);

      b.set_center(tail);
      //  insert_bucket(b, zcat, TEMP, 0, id, cost*CROSS_COST);
      */
    } else ERR("unhandled shape");
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::PFEngine::init_sizes(SimpleMap<SizeCell> &smap)
  {
    FORALL (smap.get_cells(), i) { 
      (*i).max_size = 1; 
      i->flags = OPEN; 
    }
    const sint4 w = smap.get_w(), h = smap.get_h();  
    // mark border as non-traversable
    FORS (x, w) { 
      smap(x, 0).max_size = 0; smap(x, h-1).max_size = 0;
      smap(x, 0).flags = CLOSED; smap(x, h-1).flags = CLOSED;
    }
    FORS (y, h) { 
      smap(0, y).max_size = 0; smap(w-1, y).max_size = 0; 
      smap(0, y).flags = CLOSED; smap(w-1, y).flags = CLOSED;
    }
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::PFEngine::insert_bucket(const Bucket &b, Object::ZCat zcat, 
    CellFlag f, sint4 x, sint4 cost)
  {
    MapPtr m = (zcat  == Object::ON_LAND)? &map : &air_map;
    sint4 x1, y1;
    const TerrainBase::Loc &c = b.get_center();
    const Vector<TerrainBase::Loc> &points = b.get_points();

    FORALL (points, i) {
      x1 = c.x + i->x;
      y1 = c.y + i->y;
      set_cell(m, x1, y1, f, x, cost);
    }
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::PFEngine::insert_rectangle(const Loc &topleft,const Loc &bottomright, 
    Object::ZCat zcat, CellFlag f, sint4 x, sint4 cost)
  {
    sint4 x1 = world2x(topleft.x);
    sint4 y1 = world2y(topleft.y);
    sint4 x2 = world2x(bottomright.x);
    sint4 y2 = world2y(bottomright.y);

    insert_line(x1, y1, x1, y2, zcat, f, x, cost);
    insert_line(x1, y2, x2, y2, zcat, f, x, cost);
    insert_line(x2, y2, x2, y1, zcat, f, x, cost);
    insert_line(x2, y1, x1, y1, zcat, f, x, cost);
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::PFEngine::insert_line(sint4 x1, sint4 y1, sint4 x2, sint4 y2, 
    Object::ZCat zcat,CellFlag f, sint4 x, sint4 cost)
  {
    MapPtr m = (zcat  == Object::ON_LAND)? &map : &air_map;
    int dx = x2-x1;
    int dy = y2-y1;
    
    if (dx == 0) {
    if (y1 < y2) FORT (i, y2-y1+1) set_cell(m, x1, y1+i, f, x, cost); 
    else         FORT (i, y1-y2+1) set_cell(m, x1, y2+i, f, x, cost);
    return;
    }

    if (dy == 0) {
    if (x1 < x2) FORT (i, x2-x1+1) set_cell(m, x1 + i, y1, f, x, cost);
    else         FORT (i, x1-x2+1) set_cell(m, x2 + i, y1, f, x, cost);
    return;
    }

    if (dx == dy) {
    FORT (i, x2-x1)   set_cell(m, x1+i+1, y1+i,   f, x, cost);
    FORT (i, x2-x1+1) set_cell(m, x1+i,   y1+i,   f, x, cost);
    FORT (i, x2-x1)   set_cell(m, x1+i,   y1+i+1, f, x, cost);
    return;
    }

    if (dx == -dy) {
    FORT (i, x2 - x1    ) set_cell(m, x1+i  , y1-i-1, f, x, cost);
    FORT (i, x2 - x1 + 1) set_cell(m, x1+i  , y1-i  , f, x, cost);
    FORT (i, x2 - x1    ) set_cell(m, x1+i+1, y1-i  , f, x, cost);
    return;
    }
    
    ERR("can't insert general line");
  }

  //-----------------------------------------------------------------------------
  /**
  * set the info stored in the cell at x,y in map m to given information
  * @param m the map to be edited
  * @param x,y the x and y coordinates of the cell
  * @param f flag to set the cell to
  * @param size amount by which the size paramiter of the cell is _changed_
  * @param id   id to be stored in cell
  * @param cost amount by which the cost paramiter of the cell is _changed_
  * */
  void ST_Terrain::PFEngine::set_cell(MapPtr m, sint4 x, sint4 y,
    CellFlag f, sint4 size, sint4 cost)
  {
    if (x < 0 || x >= m->get_w() || y >= m->get_h() || y < 0  ) {
      //    cout << "invalid coodinates : " << "[" <<x << "," << y << "]" << endl;
      return;
    }

    SizeCell *c = &(*m)(x, y);

    if (c->max_size > 0 && size > 0 ) {
      c->max_size = 0;
    } else {
      c->max_size -= size;
    }

    c->flags  = f;

    c->cost += cost; // if (c->cost < 0) c->cost = 0;
  }
 
} // End of namespace Demo_SimpleTerrain
