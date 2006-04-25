// $Id: SEnvironment.C,v 1.16 2005/06/28 08:42:18 orts_furtak Exp $

// This is an ORTS file 
// (c) Michael Buro,
// (c) Sami Wagiaalla 
// licensed under the GPL

#include "Global.H"
#include "SEnvironment.H"
#include "Game.H"
#include "ClientState.H"

using namespace std;

SEnvironment::SEnvironment(GameStateModule *gsm, int width, int height, int tile_points, int gran)
  : state(gsm)
{
  map = new SimpleMap<SizeCell>(width*gran+1, height*gran+1);
  init_sizes(*map);
  
  air_map = new SimpleMap<SizeCell>(width*gran+1, height*gran+1);
  init_sizes(*air_map);
  
  subtile_points = tile_points/gran;

  disp = subtile_points/2;
}


SEnvironment::~SEnvironment()
{
  delete map;
  delete air_map;
}

/**
   check the current location of the bucket if it is occupied by
   something else, evacuate it.

void SEnvironment::clear_location(Bucket& b, const SimpleMap<SizeCell> &map, Vector<Point2i>& path) {
  Point2i c = b.get_center();

  sint4 costn = 0;
  sint4 costs = 0;
  sint4 coste = 0;
  sint4 costw = 0;

  bool n = !b.check_vector(map, b.get_n(),0,0,costn);
  bool s = !b.check_vector(map, b.get_s(),0,0,costs);
  bool e = !b.check_vector(map, b.get_e(),0,0,coste);
  bool w = !b.check_vector(map, b.get_w(),0,0,costw);
    
  if (n || s || e || w) {
  
    if (n) { c.y++; }else
    if (s) { c.y--; }else
    if (e) { c.x--; }else
    if (w) { c.x++; }

    path.push_back(Point2i(x2world(c.x),x2world(c.y)));
    b.set_center(c);  
  }

  n = costn > 0;
  s = costs > 0;
  e = coste > 0;
  w = costw > 0;


  if (n || s || e || w) {
    if (n) { c.y++; }else
    if (s) { c.y--; }else
    if (e) { c.x--; }else
    if (w) { c.x++; }
    path.push_back(Point2i(x2world(c.x),x2world(c.y)));
    b.set_center(c);  
  }  
}
*/


UnitPath SEnvironment::find_path(GameObj *obj,  const Point2i &goal)
{
  SimpleMap<AStarCell> amap(map->get_w(), map->get_h());  
  Vector<Point2i> path;
  Vector<Point2i> clear;

  sint4 x1 = world2x(obj->get_int("x"));
  sint4 y1 = world2y(obj->get_int("y"));
  sint4 x2 = world2x(goal.x);
  sint4 y2 = world2y(goal.y);
  
  if (x2 > map->get_w()-2) x2 = map->get_w()-2;
  if (y2 > map->get_w()-2) y2 = map->get_h()-2;
  if (x2 < 0 ) x2 = 1;
  if (y2 < 0 ) y2 = 1;
  
  bool found = false;
  int r = obj->get_int("radius");
  int newr = r / subtile_points;
  if (r%subtile_points >= subtile_points/2) { newr++; }
  Bucket b = BucketFactory::get_bucket(newr);
  b.set_center(Point2i(x1, y1));

  SimpleMap<SizeCell> *m; 

  if (obj->get_int("zcat") == Object::ON_LAND) m = map;
  else                                         m = air_map;
  
  remove_object(obj);
  // clear current location 
  clear_location(b, *m, clear);
  
  // now find a path
  found = simple_astar(*m, amap, b, x2, y2);
  printf("FINDING PATHS");
  insert_object(obj);
  
  if (!found) {    

    cout << "no path found" << endl;

  } else {

    uint4 cell = amap.index(x2, y2);
    
    // set flags along path and accumulate path length
    //    sint4 x1, y1, x2, y2; 
    path.push_back( goal );
    //    path.push_back(Point2i(x2world(x2),y2world(y2)));
    //    cout << "[G]" << "\t: " << "X: "<< goal.x << " Y: " <<  goal.y << endl;
    do {
      (*map)(cell).flags = 1;
      
      uint4 p = amap(cell).parent;

      if (p) {
        x2 = x2world(map->i2x(p));
        y2 = y2world(map->i2y(p));
        
        path.push_back(Point2i(x2,y2));
        //       uint4 i = path.size()-1;
        //       if (i > 1) cout << "[" << i << "]\t: " << "X: "<< path[i].x << " Y: " <<  path[i].y << endl;
      }
      cell = p;
    } while(cell);
    
    if (path.size() > 0) path.erase(path.end()-1);

    //add the part to clear current location
    FORALL (clear, i) { path.push_back(*i); }
    //    cout << "[S]" << "\t: " << "X: "<< obj->get_int("x") << " Y: " <<  obj->get_int("y") << endl;
    if (path.size() > 0) { 
      smoothen_path(path);
      return UnitPath(obj, path);
    }
  }

  return UnitPath(obj, path);
}

/**
 * go throught the waypoints in the given vector and remove
 * all redundent points that fall on a straight line
 * */
void SEnvironment::smoothen_path(Vector<Point2i> &path)
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

/**
 * returns if the given three points are on a straight line
 * */
bool SEnvironment::on_line(const Point2i &a, const Point2i &b, const Point2i &c)
{
  if (a.x == b.x && b.x == c.x) { return true; }
  if (a.y == b.y && b.y == c.y) { return true; }
  double tx = (double)(b.x - a.x) / ((double)(c.x - a.x));
  double ty = (double)(b.y - a.y) / ((double)(c.y - a.y));
  return tx >=0 && tx <= 1 && tx == ty;
}

/**
 * set the info stored in the cell at x,y in map m to given information
 * @param m the map to be edited
 * @param x,y the x and y coordinates of the cell
 * @param f flag to set the cell to
 * @param size amount by which the size paramiter of the cell is _changed_
 * @param id   id to be stored in cell
 * @param cost amount by which the cost paramiter of the cell is _changed_
 * */
void SEnvironment::set_cell(SimpleMap<SizeCell> *m, sint4 x, sint4 y,
                           CellFlag f, sint4 size, sint4 id, sint4 cost)
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
  c->obj_id = id;

  c->cost += cost; // if (c->cost < 0) c->cost = 0;
}

void SEnvironment::insert_line(sint4 x1, sint4 y1, sint4 x2, sint4 y2, sint4 zcat,
                              CellFlag f, sint4 x, sint4 cost) {
  int dx = x2-x1;
  int dy = y2-y1;
  
  SimpleMap<SizeCell> *m;
  
  if (zcat  == Object::ON_LAND) {
    m = map;
  } else {
    m = air_map;
  }
  
  const sint4 id = 0;

  if (dx == 0) {
    if (y1 < y2) FORT (i, y2-y1+1) set_cell(m, x1, y1+i, f, x, id, cost); 
    else         FORT (i, y1-y2+1) set_cell(m, x1, y2+i, f, x, id, cost);
    return;
  }
  
  if (dy == 0) {
    if (x1 < x2) FORT (i, x2-x1+1) set_cell(m, x1 + i, y1, f, x, id, cost);
    else         FORT (i, x1-x2+1) set_cell(m, x2 + i, y1, f, x, id, cost);
    return;
  }
  
  if (dx == dy) {
    FORT (i, x2-x1)   set_cell(m, x1+i+1, y1+i,   f, x, id, cost);
    FORT (i, x2-x1+1) set_cell(m, x1+i,   y1+i,   f, x, id, cost);
    FORT (i, x2-x1)   set_cell(m, x1+i,   y1+i+1, f, x, id, cost);
    return;
  }

  if (dx == -dy) {
    FORT (i, x2 - x1    ) set_cell(m, x1+i  , y1-i-1, f, x, id, cost);
    FORT (i, x2 - x1 + 1) set_cell(m, x1+i  , y1-i  , f, x, id, cost);
    FORT (i, x2 - x1    ) set_cell(m, x1+i+1, y1-i  , f, x, id, cost);
    return;
  }

  ERR("can't insert general line");
}

void SEnvironment::ir_object(GameObj *obj, const Point2i &c, sint4 heading, bool insert)
{
  uint4 id = insert ? state->get_game().get_cplayer_info().get_id(obj) : 0;

  sint4 zcat = *obj->sod.zcat;
  sint4 shape = *obj->sod.shape;

  sint4 cost = insert ? 1 : -1;
  
  if (shape == Object::RECTANGLE) {
    if (!insert) {
      if (obj->attr_changed("zcat")) {
        if (zcat == Object::ON_LAND) {
          zcat = Object::IN_AIR;
        } else {
          zcat = Object::ON_LAND;
        }
      }
    }

    sint4 w = obj->get_int("width");
    sint4 h = obj->get_int("height");

    if (insert) {
      insert_shape(c, w, h, 0, shape, zcat, TEMP, 0*cost, cost*UNIT_COST);
    } else {
      insert_shape(c, w, h, 0, shape, zcat, OPEN, 0*cost, cost*UNIT_COST);
    }

  } else if (shape == Object::CIRCLE) {

    sint4 r = obj->get_int("radius");

    int newr = r / subtile_points;
    if (r % subtile_points >= subtile_points / 2) newr++;
    Bucket b = BucketFactory::get_bucket(newr);
    Bucket b2 = BucketFactory::get_bucket(newr-1);
    Point2i cworld(world2x(c.x), world2y(c.y));

    b.set_center(cworld);
    insert_bucket(b, zcat, TEMP, 0, id, cost*UNIT_COST);
    
    b2.set_center(cworld);
    insert_bucket(b2, zcat, TEMP, 0, id, cost*UNIT_COST/2);

    // one second distance
    Point2i head = cworld;
    Point2i tail = cworld;
    get_head_and_tail(head, tail, heading);
    
    b.set_center(head);
    insert_bucket(b, zcat, TEMP, 0, id, cost*CROSS_COST);
    
    b.set_center(tail);
//  insert_bucket(b, zcat, TEMP, 0, id, cost*CROSS_COST);
 
  } else ERR("unhandled shape");

}

//===================================================================

void SEnvironment::remove_object(GameObj *obj)
{
  std::map<GameObj*, HistItem>::iterator it = history.find(obj);
  if (it == history.end()) return;
  HistItem &hi = it->second;

  Point2i c = hi.a;
  sint4 heading = hi.speed;

  ir_object(obj, c, heading, false);
  
  history.erase(it);
}

//-------------------------------------------------------------------

void SEnvironment::insert_object(GameObj *obj) {
  
  sint4 x  = *obj->sod.x;
  sint4 y  = *obj->sod.y;
  Point2i c(x, y);

  sint4 speed = *obj->sod.speed;
  sint4 heading = -1;

  sint4 *headingp = obj->get_int_ptr("heading");
  if (headingp) heading = *headingp;
  if (speed == 0) heading = 0;

  ir_object(obj, c, heading, true);
  
  history[obj] = HistItem(Point2i(x, y), heading);
}

//===================================================================

/**
   Given a heading returns two points; one displaced one unit in the
   direction of movement and the other in the opposite direction.
*/

void SEnvironment::get_head_and_tail(Point2i &head, Point2i &tail, sint4 heading)
{
  heading = heading/4;
  switch (heading) {
  case 1:
    head.x--; tail.x++; 
    head.y--; tail.y++; 
    break;
  case 2:
    head.y--; tail.y++; 
    break;
  case 3:
    head.x++; tail.x--; 
    head.y--; tail.y++; 
    break;
  case 4:
    head.x++; tail.x--;
    break;
  case 5:
    head.x++; tail.x--;
    head.y++; tail.y--;
    break;
  case 6:
    head.y++; tail.y--;
    break;
  case 7:
    head.x--; tail.x++;
    head.y++; tail.y--;
    break;
  case 8:
    head.x--; tail.x++;
    break;
  }
}

void SEnvironment::insert_shape(const Point2i &c, 
                               sint4 w, 
                               sint4 h, 
                               sint4 r, 
                               sint4 shape, 
                               sint4 zcat, 
                               CellFlag f, 
                               sint4 x,
                               sint4 cost)
{
  sint4 x1, x2, y1, y2;
  
  if (shape == Object::RECTANGLE) {

    x1 = world2x(c.x-w/2);
    x2 = world2x(c.x+w/2);
    y1 = world2x(c.y-h/2);
    y2 = world2x(c.y+h/2);

    insert_line(x1, y1, x1, y2, zcat, f, x, cost);
    insert_line(x1, y2, x2, y2, zcat, f, x, cost);
    insert_line(x2, y2, x2, y1, zcat, f, x, cost);
    insert_line(x2, y1, x1, y1, zcat, f, x, cost);
    
  } else if (shape == Object::CIRCLE) {
    
    x1 = world2x(c.x-r);
    x2 = world2x(c.x+r);
    y1 = world2x(c.y-r);
    y2 = world2x(c.y+r);
    
    insert_line(x1, y1, x1, y2, zcat, f, x, cost);
    insert_line(x1, y2, x2, y2, zcat, f, x, cost);
    insert_line(x2, y2, x2, y1, zcat, f, x, cost);
    insert_line(x2, y1, x1, y1, zcat, f, x, cost); 

  } else ERR("unhandled shape");
}


void SEnvironment::insert_bucket(const Bucket &b, sint4 zcat, CellFlag f,
                                sint4 x, sint4 id, sint4 cost)
{
  SimpleMap<SizeCell>* m;
  sint4 x1, y1;

  if (zcat  == Object::ON_LAND) {
    m = map;
  }else{
    m = air_map;
  }
  const Point2i &c = b.get_center();

  const Vector<Point2i> &points = b.get_points();

  FORALL (points, i) {
    x1 = c.x + i->x;
    y1 = c.y + i->y;
    set_cell(m, x1, y1, f, x, id, cost);
  }
}

REGISTER_TYPEOF(310, Vector<SizeCell>::iterator);

void SEnvironment::init_sizes(SimpleMap<SizeCell> &smap)
{
  FORALL (smap.get_cells(), i) { (*i).max_size = 1; i->flags = OPEN; i->obj_id = -1; }

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

/*

priorityqueue Open
list    Closed

AStarSearch
s.g = 0   // s is the start node
s.h = GoalDistEstimate( s )
s.f = s.g + s.h
s.parent = null
push s on Open
while Open is not empty {
pop node n from Open  // n has the lowest f
if n is a goal node {
construct path 
return success
}  
for each successor n' of n {
newg = n.g + cost(n,n')
if n' is in Open or Closed, and n'.g <= newg skip
n'.parent = n
n'.g = newg
n'.h = GoalDistEstimate( n' )
n'.f = n'.g + n'.h
if n' is in Closed remove it from Closed
if n' is in Open remove it from Open
push n' to Open
}      
push n onto Closed
}  
return failure  // if no path found

*/


// lower bound on distance between two points

uint4 SEnvironment::estimate(sint4 x1, sint4 y1, sint4 x2, sint4 y2)
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


void SEnvironment::clear(CellFlag f) {
  FORALL (map->get_cells(), i) {
    if (i->flags == f) { i->max_size = 1; i->flags = OPEN; }
  }
}

void SEnvironment::clear_air(CellFlag f) {
  FORALL (air_map->get_cells(), i) {
    if ((*i).flags == f) { i->max_size = 1; i->flags = OPEN; }
  }
}

void SEnvironment::clear_id(sint4 id) {
  FORALL (map->get_cells(), i) {
    if ((*i).obj_id == id ) { i->max_size = 1; i->flags = OPEN; }
  }
}

void SEnvironment::clear_id_air(sint4 id) {
  FORALL (air_map->get_cells(), i) {
    if ((*i).obj_id == id) { i->max_size = 1; i->flags = OPEN; }
  }
}


bool SEnvironment::simple_astar(SimpleMap<SizeCell> &smap, SimpleMap<AStarCell> &amap,
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
    b.set_center(Point2i(smap.i2x(he.cell), smap.i2y(he.cell)));

    HeapElement hen;

    FORS (i, 8) {
      //      if (i%2) continue;

      sint4 moveCost = 0;
      if (!b.check_direction(smap, i, moveCost)) continue;

      uint4 nn_index = he.cell + smap.get_d(i); // neighbour

      // cout << int(nn->max_size) << " " << int(size) << endl;
      
      AStarCell &nn = amap(nn_index);
      uint4 newg = he.g + cost[i] + moveCost;

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



/**
   check the current location of the bucket if it is occupied by
   something else, move away from it.
*/
void SEnvironment::clear_location(Bucket &b, const SimpleMap<SizeCell> &smap,
                                 Vector<Point2i> &path)
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

    b.set_center(Point2i(smap.i2x(he.cell), smap.i2y(he.cell)));

    HeapElement hen;

    FORS (i, 8) {
      //      if (i%2) continue;

      sint4 moveCost = 0;
      if (!b.check_direction(smap, i, moveCost)) continue;

      uint4 nn_index = he.cell + smap.get_d(i); // neighbour
      //      if (smap(nn_index).max_size < size) continue; // object does not fit

      // cout << int(nn->max_size) << " " << int(size) << endl;

      AStarCell &nn = amap(nn_index);
      uint4 newg = he.g + cost[i] + moveCost;
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
      (*map)(cell).flags = 1;
    
      uint4 p = amap(cell).parent;
      if (p) {
        x2 = x2world(map->i2x(p));
        y2 = y2world(map->i2y(p));        
        path.push_back(Point2i(x2, y2));
      }
      cell = p;
      
    } while (cell);

    if (!path.empty()) path.erase(path.end()-1);
  }
}
