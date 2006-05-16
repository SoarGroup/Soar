/** @file MapTool.C
    @see MapTool.H

    $Id: MapTool.C,v 1.65 2006/05/09 22:41:53 orts_furtak Exp $
    $Source: /usr/bodo1/cvs/cvs/orts3/libs/serverclient/src/MapTool.C,v $
*/
// This is an ORTS file (c) Michael Buro, licensed under the GPL

#include "Global.H"
#include "MapTool.H"
#include "GameConst.H"
#include "Random.H"
#include "Options.H"

using namespace std;

MapTool::Options MapTool::opt;

void MapTool::Options::add()
{
  ::Options o("terrain generation");
  o.put("-bp",       string("testgame/main.bp"), "blueprints file to load");
  o.put("-hfile",    string(""),                 "create map from height/ramp file");
  o.put("-seed",     sint4(0),     "rng seed");
  o.put("-nobjs",    sint4(10),    "number of objects per player");
  o.put("-nplayers", sint4(1),     "number of players");
  o.put("-x",        sint4(64),    "playing field width");
  o.put("-y",        sint4(64),    "playing field height");  
  o.put("-fplat",    real8(0.6),   "fraction of terrain that is plateau");
  o.put("-clump",    sint4(800),   "degree to which terrain is clustered (1=none)");
  o.put("-smooth",   true,         "smooth borders");
  o.put("-holes",    true,         "try harder to plug terrain holes");
  o.put("-water",                  "create water");
  o.put("-rot",      false,        "rotate board 45 degrees");
  o.put("-tree1",    sint4(7),     "tree clusters");
  o.put("-tree2",    sint4(20),    "cluster size");
  
  o.put("-island",   sint4(0),     "island map with islands of given size");
  o.put("-cliff",    bool(0),      "cliff tiles for island map");
  o.put("-convex",   real8(0),     "convex islands with given max angle");
}

void MapTool::Options::get()
{
  ::Options::get("-bp", bp_file);
  ::Options::get("-hfile", hfile);
  ::Options::get("-seed", seed);
  ::Options::get("-nobjs", nobjs);
  ::Options::get("-nplayers", nplayers);
  ::Options::get("-x", width);
  ::Options::get("-y", height);
  ::Options::get("-fplat", fplat);
  ::Options::get("-clump", clump);
  ::Options::get("-smooth", smooth);
  ::Options::get("-holes", holes);
  ::Options::get("-water", water);
  ::Options::get("-rot", rot);
  ::Options::get("-tree1", tree1);
  ::Options::get("-tree2", tree2);
  
  ::Options::get("-island", island);
  ::Options::get("-cliff", cliff);
  ::Options::get("-convex", convex);
}


// defined to simplify the CTF map for debugging - less walls, etc.
//#define CTF_SIMPLE
#define CTF_NO_WALLS

// defines an extra wall - block out the center passageway
//#define EXTRA_WALL

// defined to debug winning condition - only places 1 unit per side
//#define CTF_EASY_WIN

//#define OMIT_FLAG_AND_POST

// for debugging - flip top/bottom sides
//#define CTF_FLIP_SIDES

bool CREATE_WATER = false;

//===================================================================

/** Places a unit into the world centered on the given tile coordinates. */

void MapTool::place_unit(stringstream &world, 
                         const string &unit_name, 
                         sint4 player, 
                         sint4 x, 
                         sint4 y,
                         const string &extra)
{
  const sint4 tile_points = GameConst::TILE_POINTS; // fixme: should depend on game instance
  
  world << unit_name << endl;
  world << "<INT_ATTR>" << endl;
  world << "owner " << player << endl;
  world << "x " << x * tile_points + (tile_points / 2) << endl;
  world << "y " << y * tile_points + (tile_points / 2) << endl;
  world << extra;
  world << "</INT_ATTR>" << endl;
}

//===================================================================

static string strip_white(const string &s)
{
  string foo;
  FORU (i, s.size()) {
    if (!iswspace(s[i])) foo += s[i];
  }
  return foo;
}

//===================================================================

void MapTool::parse_map_include(stringstream &world, string &fname)
{
  ifstream file(fname.c_str());
  if (!file) ERR2("file not found:", fname);

  string base;
  size_t p = fname.rfind(PATH_SEP, fname.size());
  if (p != string::npos) {
    base = fname.substr(0, p+1);
  }

  while (file.good()) {
    
    string t, u;
    getline(file, t);
    if (!file) break;
    u = strip_white(t);

    if (u == "<INCLUDE>") {
      FOREVER {
        string i;
        getline(file, i);
        i = strip_white(i);
        if (i == "</INCLUDE>") break;
        if (i == "") continue;
        i = i.substr(1, i.length()-2);
        i = sanitize_path(i);
        i = base + i;
        parse_map_include(world, i);
      }
    } else {
      world << t << endl;
    }
  }
}

//===================================================================

// Initialize the world with a random map.
// Note: this code is originally from the orts.C file.

void MapTool::generate_random_map(stringstream &world)
{
  opt.get();

  Random rand;
  sint4 seed = opt.seed;
  rand.srand(seed);
  
  // note: if loading a map, -nplayers values must match
  sint4 num_players = opt.nplayers;
  
  sint4 tiles_x, tiles_y;
  sint4 gtiles_x = opt.width;
  sint4 gtiles_y = opt.height;
  sint4 z_sight = 0;

  bool rotate = opt.rot;

  CREATE_WATER = opt.water;

  // if the world is rotated 45 degrees, impose somewhat artificial 45
  // degree bariers to define the playing area.  The actual area of
  // the playing field is the same as it would have been, but the
  // world is still an axis aligned rectangle, so the actual world may
  // be much larger

  /*
    +-----+
    |  /\ |
    | /  \|
    |/   /|
    |\  / |
    | \/  |
    +-----+
   */

  // tiles_x is the actual width of the tile array stored
  // gtiles_x is the width of the playfield and is constant under rotation

  if (rotate) {
    gtiles_x = sint4(gtiles_x * 0.70710678); // side lengths are now fractions of 45 degree lines
    gtiles_y = sint4(gtiles_y * 0.70710678);
    tiles_x = gtiles_x + gtiles_y;
    tiles_y = gtiles_x + gtiles_y;
  } else {
    tiles_x = gtiles_x;
    tiles_y = gtiles_y;
  }

  world << num_players << endl;
  world << gtiles_x << endl;
  world << gtiles_y << endl;
  world << z_sight << endl;
  if (rotate) world << 1 << endl;
  else        world << 0 << endl;
  sint4 tile_n = tiles_x * tiles_y;

  Map<Tile> map_data;
  map_data.allocate(tiles_x, tiles_y);

  sint2 *height = new sint2[tile_n];
  sint2 *ramp_d = new sint2[tile_n];
  sint2 *o_safe = new sint2[tile_n]; // valid square if 0
  if (!height || !ramp_d || !o_safe) ERR("out of memory");
  memset(height, 0, tile_n*sizeof(*height));
  memset(ramp_d, 0, tile_n*sizeof(*ramp_d));
  memset(o_safe, 0, tile_n*sizeof(*o_safe));

  real8 fplat = opt.fplat;
  bool plug_holes = opt.holes;
  sint4 clump = opt.clump;
  bool smooth = opt.smooth;

  if (smooth) plug_holes = false; // smooth > plug_holes

  if (clump < 1) clump = 1;

  if (clump <= 1) {

    FORS (i, sint4(tile_n * fplat)) {
      sint4 r;
      do { r = rand.rand_uint4() % tile_n; } while (height[r] == 1);
      height[r] = 1;
    }

  } else {

    FORS (i, sint4(tile_n * fplat)) {
      sint4 r;
      do { r = rand.rand_uint4() % tile_n; } while (height[r] >= 3);
    
      sint4 next_to = 0;
      sint4 out_of = 4;
    
      sint4 x = r % tiles_x;
      sint4 y = r / tiles_x;
  
      if (x == 0 || x == tiles_x-1) out_of--;
      if (y == 0 || y == tiles_y-1) out_of--;
    
      sint4 h = height[r];

      if (x > 0 && height[r-1] == h+1) next_to++;
      if (x < tiles_x-1 && height[r+1] == h+1) next_to++;
      if (y > 0 && height[r-tiles_x] == h+1) next_to++;
      if (y < tiles_y-1 && height[r+tiles_x] == h+1) next_to++;

      const sint4 offset = 4;

      if (out_of == next_to) {
	if (rand.rand_uint4() % 3)
	  height[r] += 1;
	else if (h <= 1)
	  height[r] += 2;
        continue;
      }
      if (next_to * 2 >= out_of) {
        next_to += offset;
        out_of += offset;
      }
      if (next_to > 0) {
        if ((sint4)(rand.rand_uint4() % out_of) < next_to) {
	  //	  if (rand.rand_uint4() % 6)
	    height[r] += 1;
	    //	  else if (h <= 1)
	    //	    height[r] += 2;
	  continue;
	}
      }
      if (rand.rand_uint4() % clump < 1) {
        height[r]++;// = 1;
        continue;
      }
      --i;
    }
  }

  if (plug_holes) {

    FORS (i, tile_n) {
      sint4 r = i;

      sint4 next_to = 0;
      sint4 out_of = 4;

      sint4 rx = r % tiles_x;
      sint4 ry = r / tiles_x;

      sint4 h = height[r];
    
      if (rx == 0 || rx == tiles_x-1) out_of--;
      if (ry == 0 || ry == tiles_y-1) out_of--;
 
      if (rx > 0 && height[r-1] > h) next_to++;
      if (rx < tiles_x-1 && height[r+1] > h) next_to++;
      if (ry > 0 && height[r-tiles_x] > h) next_to++;
      if (ry < tiles_y-1 && height[r+tiles_x] > h) next_to++;

      if (out_of == next_to) {// && height[r] == 0) {
        //	cout << i << " " << next_to << " " << out_of << " plug" << endl;
        height[r]++;// = 1;
      }
    }
  }

  // mark tiles that aren't part of the board

  if (rotate) {

    FORS (i, tile_n) {
      sint4 x = i % tiles_x;
      sint4 y = i / tiles_x;

      if ((x + y < gtiles_y-1) ||
          (tiles_x - x + tiles_y - y < gtiles_y+1) ||
          (tiles_x - x + y < gtiles_x) ||
          (x + tiles_y - y < gtiles_x)) {
	height[i] = -2; o_safe[i] = 1; continue;
      }
    }
  }

  // smooth plateaus:
  //  continue filling tiles iff it has three or four filled HV neighbors

  bool changed = smooth;

  while (changed) {

    sint4 dx[] = { 1,0,-1,0 };
    sint4 dy[] = { 0,-1,0,1 };

    changed = false;
    cout << "." << flush;

    FORS (i, tile_n) {

      //      if (!height[i]) {
      sint4 h = height[i];

      sint4 ix = i % tiles_x;
      sint4 iy = i / tiles_x;

      sint4 next_to = 0;
      
      FORS (j, 4) {
	sint4 jx = ix + dx[j];
	sint4 jy = iy + dy[j];	  
	if (jx < 0 || jx >= tiles_x ||
	    jy < 0 || jy >= tiles_y) next_to++;
	else {
	  if (height[jy*tiles_x + jx] > h)
	    next_to++;
	}
      }
      
      if (next_to >= 3) { height[i]++; changed = true; }
      //      }
    }
  }

  // ramps:
  //  change some ground level tiles to ramps

  for (sint4 y = 1; y < tiles_y-2; y++) {
    for (sint4 x = 1; x < tiles_x-2; x++) {
      sint4 i = y * tiles_x + x;
      sint4 h = height[i];
      //      if (height[i] != 0) continue;
      if (rand.rand_uint4() % 5 == 0) {
        if (height[i-1] == h+1)       { ramp_d[i] = 7; continue; }
        if (height[i+1] == h+1)       { ramp_d[i] = 3; continue; }
        if (height[i-tiles_x] == h+1) { ramp_d[i] = 1; continue; }
        if (height[i+tiles_x] == h+1) { ramp_d[i] = 5; continue; }
      }
    }
  }

  // corner ramps

  for (sint4 y = 1; y < tiles_y-2; y++) {
    for (sint4 x = 1; x < tiles_x-2; x++) {
      sint4 i = y * tiles_x + x;
      sint4 h = height[i];

      //      if (rand.rand_uint4() % 8 == 0) {
      if (height[i-1] == h && height[i-tiles_x] == h && ramp_d[i-1] == 1 && ramp_d[i-tiles_x] == 7) {
	ramp_d[i] = 8; continue;
      }
      if (height[i-1] == h && height[i+tiles_x] == h && ramp_d[i-1] == 5 && ramp_d[i+tiles_x] == 7) {
	ramp_d[i] = 6; continue;
      }
      if (height[i+1] == h && height[i-tiles_x] == h && ramp_d[i+1] == 1 && ramp_d[i-tiles_x] == 3) {
	ramp_d[i] = 2; continue;
      }
      if (height[i+1] == h && height[i+tiles_x] == h && ramp_d[i+1] == 5 && ramp_d[i-tiles_x] == 3) {
	ramp_d[i] = 4; continue;
      }

      if (height[i-1] == h && height[i-tiles_x] == h && ramp_d[i-1] == 5 && ramp_d[i-tiles_x] == 3) {
	ramp_d[i] = 14; continue;
      }
      if (height[i-1] == h && height[i+tiles_x] == h && ramp_d[i-1] == 1 && ramp_d[i+tiles_x] == 3) {
	ramp_d[i] = 12; continue;
      }
      if (height[i+1] == h && height[i-tiles_x] == h && ramp_d[i+1] == 5 && ramp_d[i-tiles_x] == 7) {
	ramp_d[i] = 16; continue;
      }
      if (height[i+1] == h && height[i+tiles_x] == h && ramp_d[i+1] == 1 && ramp_d[i-tiles_x] == 7) {
	ramp_d[i] = 18; continue;
      }
      //      }
    }
  }

  // mark tiles that aren't part of the board (again)

  if (rotate) {

    FORS (i, tile_n) {
      sint4 x = i % tiles_x;
      sint4 y = i / tiles_x;

      if ((x + y < gtiles_y-1) ||
          (tiles_x - x + tiles_y - y < gtiles_y+1) ||
          (tiles_x - x + y < gtiles_x) ||
          (x + tiles_y - y < gtiles_x)) {
	height[i] = -2; o_safe[i] = 1; continue;
      }
    }
  }

  FORS (i, tile_n) {

    if (height[i] >= 0) {
      map_data(i).set_type(GROUND);

      if (ramp_d[i]) {
	if (ramp_d[i] <= 8) {
	  map_data(i).set_ramp(height[i], height[i], height[i]+1,
			       static_cast<Tile::Compass>(ramp_d[i]));
	} else {
	  map_data(i).set_ramp(height[i], height[i]+1, height[i]+1,
			       static_cast<Tile::Compass>(ramp_d[i]-10));
	}
      } else {
	map_data(i).set_flat(height[i]);
      }

    } else {
      map_data(i).set_type(Tile::UNKNOWN);
      map_data(i).set_flat(Tile::MAX_HEIGHT);
    }
  }

  // half tiles

  for (sint4 y = 1; y < tiles_y-2; y++) {
    for (sint4 x = 1; x < tiles_x-2; x++) {
      sint4 i = y * tiles_x + x;
      sint4 h = height[i];

      //      if (rand.rand_uint4() % 8 == 0) {
      if (height[i-1] == h+1 && height[i-tiles_x] == h+1 &&
	  ramp_d[i-1] == 0 && ramp_d[i-tiles_x] == 0 && ramp_d[i] == 0) {
	map_data(x,y).set_half_tile(h,h+1, Tile::NW); o_safe[i] = 1; continue;
      }

      if (height[i-1] == h+1 && height[i+tiles_x] == h+1 &&
	  ramp_d[i-1] == 0 && ramp_d[i+tiles_x] == 0 && ramp_d[i] == 0) {
	map_data(x,y).set_half_tile(h,h+1, Tile::SW); o_safe[i] = 1; continue;
      }

      if (height[i+1] == h+1 && height[i-tiles_x] == h+1 &&
	  ramp_d[i+1] == 0 && ramp_d[i-tiles_x] == 0 && ramp_d[i] == 0) {
	map_data(x,y).set_half_tile(h,h+1, Tile::NE); o_safe[i] = 1; continue;
      }

      if (height[i+1] == h+1 && height[i+tiles_x] == h+1 &&
	  ramp_d[i+1] == 0 && ramp_d[i-tiles_x] == 0 && ramp_d[i] == 0) {
	map_data(x,y).set_half_tile(h,h+1, Tile::SE); o_safe[i] = 1; continue;
      }
      //      }
    }
  }


  if (CREATE_WATER) {

    FORS (i, 20) {

      const sint4 D = 5;
      sint4 x, y;
      Tile::Type tw, te;
      do {
	x = rand.rand_uint4() % (tiles_x-D);
	y = rand.rand_uint4() % (tiles_y-D);
	map_data(x,y).get_type(tw, te);
      } while (!map_data(x,y).is_flat() || map_data(x,y).get_min_h() != 0 || tw == Tile::UNKNOWN);

      FORS (dx, D) {
        FORS (dy, D) {
          if (!map_data(x+dx, y+dy).is_flat()) break;
          if (map_data(x+dx, y+dy).get_min_h() != map_data(x,y).get_min_h()) break;
          map_data(x+dx, y+dy).set_type(WATER);
        }
      }

    }

    for (sint4 y = 1; y < tiles_y-2; y++) {
      for (sint4 x = 1; x < tiles_x-2; x++) {
        
        Tile::Type a, b;
        map_data(x,y).get_type(a, b);

        if (a == b && b == WATER) {

          if (map_data(x, y-1).get_type_at_corner(5) != WATER &&
              map_data(x-1, y).get_type_at_corner(2) != WATER &&
              map_data(x, y+1).get_type_at_corner(0) == WATER &&
              map_data(x+1, y).get_type_at_corner(7) == WATER)
              {
            map_data(x,y).set_half_tile(0,0, Tile::SE);
            map_data(x,y).set_type(GROUND,WATER);
          }
          if (map_data(x, y+1).get_type_at_corner(1) != WATER &&
              map_data(x+1, y).get_type_at_corner(6) != WATER &&
              map_data(x, y-1).get_type_at_corner(4) == WATER &&
              map_data(x-1, y).get_type_at_corner(3) == WATER)
              {
            map_data(x,y).set_half_tile(0,0, Tile::SE);
            map_data(x,y).set_type(WATER,GROUND);
          }
          if (map_data(x, y+1).get_type_at_corner(1) != WATER &&
              map_data(x-1, y).get_type_at_corner(2) != WATER &&
              map_data(x, y-1).get_type_at_corner(4) == WATER &&
              map_data(x+1, y).get_type_at_corner(7) == WATER)
              {
            map_data(x,y).set_half_tile(0,0, Tile::NE);
            map_data(x,y).set_type(GROUND,WATER);
          }
          if (map_data(x, y-1).get_type_at_corner(5) != WATER &&
              map_data(x+1, y).get_type_at_corner(6) != WATER &&
              map_data(x, y+1).get_type_at_corner(0) == WATER &&
              map_data(x-1, y).get_type_at_corner(3) == WATER)
              {
            map_data(x,y).set_half_tile(0,0, Tile::NE);
            map_data(x,y).set_type(WATER,GROUND);
          }
        }
      }
    }
  }

  
  world << "<TILES>" << endl;
  map_data.write(world);
  world << "</TILES>" << endl;
  
  // mark the tiles on the boundary, stop object creation there

  if (rotate) {
    FORS (i, tile_n) {
      sint4 x = i % tiles_x;
      sint4 y = i / tiles_x;

      if ((x + y <= gtiles_y-1) ||
          (tiles_x - x + tiles_y - y <= gtiles_y+1) ||
          (tiles_x - x + y <= gtiles_x) ||
          (x + tiles_y - y <= gtiles_x))
        { height[i] = -1; o_safe[i] = 1; continue; }
    }
  }

  string fname = opt.bp_file;
  fname = sanitize_path(fname);
  parse_map_include(world, fname);

  world << "<OBJECTS>\n";

  sint4 nobjs = opt.nobjs;

  const bool AIR_UNITS = true;
  const bool LAND_UNITS = true;
  
  if (AIR_UNITS && LAND_UNITS) nobjs /= 2;

  FORS (p, num_players) {

    if (AIR_UNITS) {

      FORS (i, nobjs) {
        sint4 r;
        do { r = rand.rand_uint4() % tile_n; } while (o_safe[r]); //(height[r] > 1 || height[r] < 0);
	o_safe[r] = 1;

        place_unit(world, "destroyer", p, r % tiles_x, r / tiles_x);
      }
    }

    if (LAND_UNITS) { 

      FORS (i, nobjs) {
        sint4 r;
        do { r = rand.rand_uint4() % tile_n; } while (o_safe[r]); //(height[r]);
	o_safe[r] = 1;

        place_unit(world, "marine", p, r % tiles_x, r / tiles_x);
      }
    }
  }
  
  world << "</OBJECTS>\n";
  delete [] height;
  delete [] ramp_d;
  delete [] o_safe;
}

void MapTool::generate_frano_map(stringstream &world)
{
  opt.get();

  Random rand;
  sint4 seed = opt.seed;
  rand.srand(seed);
  
  // note: if loading a map, -nplayers values must match
  sint4 num_players = opt.nplayers;
  
  sint4 tiles_x, tiles_y;
  sint4 gtiles_x, gtiles_y;

  gtiles_x = 50;
  gtiles_y = 38;
  sint4 z_sight = 0;

  tiles_x = gtiles_x;
  tiles_y = gtiles_y;

  world << num_players << endl;
  world << gtiles_x << endl;
  world << gtiles_y << endl;
  world << z_sight << endl;
  world << 0 << endl;

  sint4 tile_n = tiles_x * tiles_y;

  Map<Tile> map_data;
  map_data.allocate(tiles_x, tiles_y);

  sint2 *height = new sint2[tile_n];
  sint2 *ramp_d = new sint2[tile_n];
  sint2 *o_safe = new sint2[tile_n]; // valid square if 0
  if (!height || !ramp_d || !o_safe) ERR("out of memory");
  memset(height, 0, tile_n*sizeof(*height));
  memset(ramp_d, 0, tile_n*sizeof(*ramp_d));
  memset(o_safe, 0, tile_n*sizeof(*o_safe));


  FORS (i, tile_n) {
    //map_data(i).set_type(GROUND);
    map_data(i).set_type(WATER);
    //map_data(i).set_flat(height[i]);
    map_data(i).set_flat(1);
  }

  /*
  map_data.get_tile(15,12).set_topo(1, 0, 1, 0, Tile::TB_SPLIT);
  map_data.get_tile(15,28).set_topo(0, 1, 0, 1, Tile::BT_SPLIT);
  map_data.get_tile(35,12).set_topo(1, 0, 1, 0, Tile::TB_SPLIT);
  map_data.get_tile(35,28).set_topo(0, 1, 0, 1, Tile::BT_SPLIT);
  */

/*
  map_data(15,12).set_topo(0, 1, 0, 1, Tile::TB_SPLIT);
  map_data(17,12).set_topo(0, 1, 0, 1, Tile::TB_SPLIT);
  map_data(19,12).set_topo(0, 1, 0, 1, Tile::BT_SPLIT);

  map_data(21,12).set_half_tile(1,2, Tile::NW);

  */
  world << "<TILES>" << endl;
  map_data.write(world);
  world << "</TILES>" << endl;


  string fname = opt.bp_file;
  fname = sanitize_path(fname);

  parse_map_include(world, fname);

  world << "<OBJECTS>\n";

  sint4 nobjs = opt.nobjs;

  const bool AIR_UNITS = false;
  const bool LAND_UNITS = true;
  
  if (AIR_UNITS && LAND_UNITS) nobjs /= 2;

  FORS (p, num_players) {

    if (AIR_UNITS) {

      FORS (i, nobjs) {
        sint4 r;
        do { r = rand.rand_uint4() % tile_n; } while (o_safe[r]);
	o_safe[r] = 1;

        place_unit(world, "destroyer", p, r % tiles_x, r / tiles_x);
      }
    }

    if (LAND_UNITS) { 

      FORS (i, nobjs) {
        sint4 r;
        do { r = rand.rand_uint4() % tile_n; } while (o_safe[r]);
	o_safe[r] = 1;

        place_unit(world, "worker", p, r % tiles_x, r / tiles_x);
      }
    }
  }

  world << "</OBJECTS>\n";

  delete [] height;
  delete [] ramp_d;
  delete [] o_safe;
}

void MapTool::generate_debug_map(stringstream &world)
{
  opt.get();

  Random rand;
  sint4 seed = opt.seed;
  rand.srand(seed);
  
  // note: if loading a map, -nplayers values must match
  sint4 num_players = opt.nplayers;
  
  sint4 tiles_x, tiles_y;
  sint4 gtiles_x, gtiles_y;

  gtiles_x = 50;
  gtiles_y = 38;
  sint4 z_sight = 0;

  tiles_x = gtiles_x;
  tiles_y = gtiles_y;

  world << num_players << endl;
  world << gtiles_x << endl;
  world << gtiles_y << endl;
  world << z_sight << endl;
  world << 0 << endl;

  sint4 tile_n = tiles_x * tiles_y;

  Map<Tile> map_data;
  map_data.allocate(tiles_x, tiles_y);

  sint2 *height = new sint2[tile_n];
  sint2 *ramp_d = new sint2[tile_n];
  sint2 *o_safe = new sint2[tile_n]; // valid square if 0
  if (!height || !ramp_d || !o_safe) ERR("out of memory");
  memset(height, 0, tile_n*sizeof(*height));
  memset(ramp_d, 0, tile_n*sizeof(*ramp_d));
  memset(o_safe, 0, tile_n*sizeof(*o_safe));


  FORS (i, tile_n) {
    map_data(i).set_type(GROUND);
    map_data(i).set_flat(height[i]);
  }

  /*
  map_data.get_tile(15,12).set_topo(1, 0, 1, 0, Tile::TB_SPLIT);
  map_data.get_tile(15,28).set_topo(0, 1, 0, 1, Tile::BT_SPLIT);
  map_data.get_tile(35,12).set_topo(1, 0, 1, 0, Tile::TB_SPLIT);
  map_data.get_tile(35,28).set_topo(0, 1, 0, 1, Tile::BT_SPLIT);
  */


  map_data(15,12).set_topo(0, 1, 0, 1, Tile::TB_SPLIT);
  map_data(17,12).set_topo(0, 1, 0, 1, Tile::TB_SPLIT);
  map_data(19,12).set_topo(0, 1, 0, 1, Tile::BT_SPLIT);

  map_data(21,12).set_half_tile(1,2, Tile::NW);

  
  world << "<TILES>" << endl;
  map_data.write(world);
  world << "</TILES>" << endl;


  string fname = opt.bp_file;
  fname = sanitize_path(fname);
  parse_map_include(world, fname);

  world << "<OBJECTS>\n";

  FORS (p, num_players) {
    
    sint4 r;
    do { r = rand.rand_uint4() % tile_n; } while (o_safe[r]);
    o_safe[r] = 1;
    place_unit(world, "destroyer", p, r % tiles_x, r / tiles_x);

    do { r = rand.rand_uint4() % tile_n; } while (o_safe[r]);
    o_safe[r] = 1;
    place_unit(world, "worker", p, r % tiles_x, r / tiles_x);
  }

  world << "</OBJECTS>\n";

  delete [] height;
  delete [] ramp_d;
  delete [] o_safe;
}


// generate random map with CLIFF tiles

void MapTool::generate_random_cliff_map(stringstream &world)
{
  opt.get();

  Random rand;
  sint4 seed = opt.seed;
  rand.srand(seed);

  // note: if loading a map, -nplayers values must match
  sint4 num_players = opt.nplayers;
  
  sint4 tiles_x, tiles_y;
  sint4 gtiles_x = opt.width;
  sint4 gtiles_y = opt.height;
  sint4 z_sight = 0;

  bool rotate = opt.rot;
  CREATE_WATER = opt.water;

  // if the world is rotated 45 degrees, impose somewhat artificial 45
  // degree bariers to define the playing area.  The area of
  // the playing field is the same as it would have been, but the
  // world is still an axis aligned rectangle, so the actual world may
  // be much larger

  /*
    +-----+
    |  /\ |
    | /  \|
    |/   /|
    |\  / |
    | \/  |
    +-----+
  */

  // tiles_x is the actual width of the tile array stored
  // gtiles_x is the width of the playfield and is constant under rotation

  if (rotate) {
    gtiles_x = sint4(gtiles_x * 0.70710678); // side lengths are now fractions of 45 degree lines
    gtiles_y = sint4(gtiles_y * 0.70710678);
    tiles_x = gtiles_x + gtiles_y;
    tiles_y = gtiles_x + gtiles_y;
  } else {
    tiles_x = gtiles_x;
    tiles_y = gtiles_y;
  }

  world << num_players << endl;
  world << gtiles_x << endl;
  world << gtiles_y << endl;
  world << z_sight << endl;
  if (rotate) world << 1 << endl;
  else        world << 0 << endl;

  sint4 tile_n = tiles_x * tiles_y;

  Map<Tile> map_data;
  map_data.allocate(tiles_x, tiles_y);

  sint4 h_x = tiles_x + 1;
  sint4 h_y = tiles_y + 1;  
  sint4 h_n = h_x * h_y;

  Vector<sint2> height(h_n); // height field (index is not a tile index!)
  Vector<sint2> bad(tile_n); // do not put stuff on tile if true, works on tiles!

  // first heights heights represent layers (will be multiplied by GameConst::HEIGHT_MULT later

  real8 fplat = opt.fplat;
  bool plug_holes = opt.holes;
  sint4 clump = opt.clump;
  bool smooth = opt.smooth;

  if (smooth) plug_holes = false; // smooth > plug_holes

  if (clump < 1) clump = 1;

  if (clump <= 1) {

    FORS (i, sint4(tile_n * fplat)) {
      sint4 r;
      do { r = rand.rand_uint4() % h_n; } while (height[r] == 1);
      height[r] = 1;
    }

  } else {

    FORS (i, sint4(tile_n * fplat)) {
      sint4 r;
      do { r = rand.rand_uint4() % h_n; } while (height[r] >= 3);
    
      sint4 next_to = 0;
      sint4 out_of = 4;
    
      sint4 x = r % h_x;
      sint4 y = r / h_x;
  
      if (x == 0 || x == h_x-1) out_of--;
      if (y == 0 || y == h_y-1) out_of--;
    
      sint4 h = height[r];

      if (x > 0     && height[r-1]   == h+1) next_to++;
      if (x < h_x-1 && height[r+1]   == h+1) next_to++;
      if (y > 0     && height[r-h_x] == h+1) next_to++;
      if (y < h_y-1 && height[r+h_x] == h+1) next_to++;

      const sint4 offset = 4;

      if (out_of == next_to) {
	if (rand.rand_uint4() % 3) height[r] += 1;
	else if (h <= 1) height[r] += 2;
        continue;
      }
      if (next_to * 2 >= out_of) {
        next_to += offset;
        out_of += offset;
      }
      if (next_to > 0) {
        if ((sint4)(rand.rand_uint4() % out_of) < next_to) {
	  height[r] += 1;
	  continue;
	}
      }
      if (rand.rand_uint4() % clump < 1) {
        height[r]++; // = 1;
        continue;
      }
      --i;
    }
  }

  if (plug_holes) {

    FORS (i, h_n) {
      sint4 r = i;

      sint4 next_to = 0;
      sint4 out_of = 4;

      sint4 rx = r % h_x;
      sint4 ry = r / h_x;

      sint4 h = height[r];
    
      if (rx == 0 || rx == h_x-1) out_of--;
      if (ry == 0 || ry == h_y-1) out_of--;

      if (rx > 0     && height[r-1]   > h) next_to++;
      if (rx < h_x-1 && height[r+1]   > h) next_to++;
      if (ry > 0     && height[r-h_x] > h) next_to++;
      if (ry < h_y-1 && height[r+h_x] > h) next_to++;

      if (out_of == next_to) {// && height[r] == 0) {
        //	cout << i << " " << next_to << " " << out_of << " plug" << endl;
        height[r]++;// = 1;
      }
    }
  }

  // the world is now a height field with no ramps
  // flatten the corner and add demo buildings

  if (tiles_x > 40 && tiles_y > 60) {
    FORU (i, 40) {
      FORU (j, 60) {
        height[i + j*h_x] = 0;
      }
    }
  }

  // smooth plateaus:
  //  continue filling tiles iff it has three or four filled HV neighbors

  bool changed = smooth;

  while (changed) {

    changed = false;
    
    sint4 dx[] = { 1,  0, -1, 0 };
    sint4 dy[] = { 0, -1,  0, 1 };

    FORS (i, h_n) {

      sint4 h = height[i];

      sint4 ix = i % h_x;
      sint4 iy = i / h_x;

      sint4 next_to = 0;
      
      FORS (j, 4) {
	sint4 jx = ix + dx[j];
	sint4 jy = iy + dy[j];	  
	if (jx < 0 || jx >= h_x || jy < 0 || jy >= h_y) next_to++;
	else if (height[jy*h_x + jx] > h) next_to++;
      }
      
      if (next_to >= 3) { height[i]++; changed = true; }
    }
  }


  FORS (i, h_n) height[i] *= GameConst::HEIGHT_MULT;

  // from here on: heights are multiples of GameConst::HEIGHT_MULT
  // or at half-heights (ramps)
  
  FORS (i, h_n) {

    sint4 x = i % h_x;
    sint4 y = i / h_x;

    if (x == h_x-1 || y == h_y-1) continue;

    //    cout << height[i] << " ";
    
    sint4 h[4];
    
    h[0] = height[i];
    h[1] = height[i+1];
    h[2] = height[i+h_x+1];
    h[3] = height[i+h_x];

    Tile &tile = map_data.get_tile(x, y);

    tile.set_topo(h[0], h[1], h[2], h[3], Tile::NO_SPLIT);
    tile.set_type(GROUND);

    assert(x >= 0 && x < tiles_x);	
    assert(y >= 0 && y < tiles_y);
    
    // equal heights -> regular ground tile
    
    if (h[0] == h[1] && h[0] == h[2] && h[0] == h[3]) {

      tile.set_topo(h[0], h[1], h[2], h[3], Tile::NO_SPLIT);
      tile.set_type(GROUND);
      continue;
    }

    // 3 equal heights -> split
    //  - make equal height GROUND triangle and the other CLIFF or
    //  - create ridge
    
    {
      if ((h[0] == h[1] && h[1] == h[2]) ||
          (h[0] == h[1] && h[1] == h[3]) ||
          (h[0] == h[2] && h[2] == h[3]) ||
          (h[1] == h[2] && h[2] == h[3])) {

        sint4 i;
        FOR (i, 4) {
          if (h[i] != h[(i+1)&3] && h[i] != h[(i+2)&3]) break;
        }

	if (1 || (rand.rand_uint4() & 8)) {

	  // equal height triangle
	  
	  if (i == 0 || i == 2) {  // |/|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::BT_SPLIT);
	  } else { // |\|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::TB_SPLIT);
	  }
      
	  if (i == 0 || i == 3) tile.set_type(CLIFF, GROUND); // west CLIFF
	  else                  tile.set_type(GROUND, CLIFF); // east CLIFF

	} else {

	  // ridge

	  if (i == 0 || i == 2) {  // |\|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::TB_SPLIT);
	  } else { // |/|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::BT_SPLIT);
	  }

	  tile.set_type(CLIFF, CLIFF);
	}
	assert(x >= 0 && x < tiles_x);	
	assert(y >= 0 && y < tiles_y);
	bad[y*tiles_x+x] = 1;
	continue;
      }
    }

    
    // 2 and 2 equal heights -> full CLIFF tile
    
    {
      sint4 i;
      FOR (i, 4) {
	if (h[i] == h[(i+1) & 3] && h[(i+2) & 3] == h[(i+3) & 3]) break;
      }
    
      if (i < 4) {

	tile.set_topo(h[0], h[1], h[2], h[3], Tile::NO_SPLIT);

	// small chance of ramp
	if (rand.rand_uint4() % 100 < 15) tile.set_type(GROUND);
	else                              tile.set_type(CLIFF);

	assert(x >= 0 && x < tiles_x);	
	assert(y >= 0 && y < tiles_y);
	bad[y*tiles_x+x] = 1;
	continue;
      }
    }

    // otherwise: random CLIFF/CLIFF split

    {
      tile.set_type(CLIFF, CLIFF);
   
      if (rand.rand_uint4() & 8) {
	tile.set_topo(h[0], h[1], h[2], h[3], Tile::BT_SPLIT);
      } else {
	tile.set_topo(h[0], h[1], h[2], h[3], Tile::TB_SPLIT);
      }

      assert(x >= 0 && x < tiles_x);	
      assert(y >= 0 && y < tiles_y);
      bad[y*tiles_x+x] = 1;
    }
  }

  if (rotate) {

    FORS (i, tile_n) {
      sint4 x = i % tiles_x;
      sint4 y = i / tiles_x;

      if ((x + y < gtiles_y-1) ||
          (tiles_x - x + tiles_y - y < gtiles_y+1) ||
          (tiles_x - x + y < gtiles_x) ||
          (x + tiles_y - y < gtiles_x)) {
        map_data(x,y).set_type(Tile::UNKNOWN);
      }
    }
  }

  FORS (i, tile_n) {

    sint4 x = i % tiles_x;
    sint4 y = i / tiles_x;

    Tile &t = map_data.get_tile(i);
    Tile::Type tw, te;
    Tile::Split split;
    t.get_type(tw, te);
    sint4 h0, h1, h2, h3;
    t.get_topo(h0, h1, h2, h3, split);

    if (split == Tile::NO_SPLIT && tw != CLIFF && (h0 != h3 || h0 != h1)) {

      // ramp

      if (h0 != h3) {

	// | ramp

	if (x > 0) {  // extend left
	  Tile &t = map_data.get_tile(x-1, y);
	  t.get_type(tw, te);
	  t.get_topo(h0, h1, h2, h3, split);

	  if (rand.rand_uint4() % 100 < 50 && te == CLIFF) {
	    if (split) t.set_type(tw, GROUND);
	    else       t.set_type(GROUND, GROUND);
	  }
	}
	
	if (x < tiles_x-1) {  // extend right
	  Tile &t = map_data.get_tile(x+1, y);
	  t.get_type(tw, te);
	  t.get_topo(h0, h1, h2, h3, split);

	  if (rand.rand_uint4() % 100 < 50 && tw == CLIFF) {
	    if (split) t.set_type(GROUND, te);
	    else       t.set_type(GROUND, GROUND);
	  }
	}

      } else {

	// - ramp

 	if (y > 0) {  // extend up
	  Tile &t = map_data.get_tile(x, y-1);
	  t.get_type(tw, te);
	  t.get_topo(h0, h1, h2, h3, split);

	  if (rand.rand_uint4() % 100 < 50 &&
	      ((!split && tw == CLIFF) ||                    // | |
	       (split == Tile::TB_SPLIT && tw == CLIFF) ||   // |\|
	       (split == Tile::BT_SPLIT && te == CLIFF))) {  // |/|

	    if   (!split)                     t.set_type(GROUND, GROUND);
	    else if (split == Tile::TB_SPLIT) t.set_type(GROUND, te);
	    else                              t.set_type(tw, GROUND);
	  }
	}
	
	if (y < tiles_y-1) {  // extend down
	  Tile &t = map_data.get_tile(x, y+1);
	  t.get_type(tw, te);
	  t.get_topo(h0, h1, h2, h3, split);

	  if (rand.rand_uint4() % 100 < 50 &&
	      ((!split && tw == CLIFF) ||                    // | |
	       (split == Tile::TB_SPLIT && te == CLIFF) ||   // |\|
	       (split == Tile::BT_SPLIT && tw == CLIFF))) {  // |/|

	    if   (!split   )                  t.set_type(GROUND, GROUND);
	    else if (split == Tile::TB_SPLIT) t.set_type(tw, GROUND);
	    else                              t.set_type(GROUND, te);
	  }
	}
      }
    }
  }
  
  if (CREATE_WATER) {

    // create lakes
    FORS (i, 20) {

      const sint4 D = 5;
      sint4 x, y;
      Tile::Type tw, te;
      do {
	x = rand.rand_uint4() % (tiles_x-D);
	y = rand.rand_uint4() % (tiles_y-D);
	map_data(x,y).get_type(tw, te);
      } while (!map_data(x,y).is_flat() || map_data(x,y).get_min_h() != 0 || tw == Tile::UNKNOWN);

      FORS (dx, D) {
        FORS (dy, D) {
          if (!map_data(x+dx, y+dy).is_flat()) break;
          if (map_data(x+dx, y+dy).get_min_h() != map_data(x,y).get_min_h()) break;
          map_data(x+dx, y+dy).set_type(WATER);
        }
      }
    }

    // smooth corners
    for (sint4 y = 1; y < tiles_y-2; y++) {
      for (sint4 x = 1; x < tiles_x-2; x++) {
        
        Tile::Type a, b;
        map_data(x,y).get_type(a, b);

        if (a == b && b == WATER) {
	  // sint4 h[4];
	  // Tile::Split split;

          if (map_data(x, y-1).get_typeS() != WATER &&
              map_data(x-1, y).get_typeE() != WATER &&
              map_data(x, y+1).get_typeN() == WATER &&
              map_data(x+1, y).get_typeW() == WATER)
            {
              map_data(x,y).set_type(GROUND, WATER, Tile::BT_SPLIT);
            }
          if (map_data(x, y+1).get_typeN() != WATER &&
              map_data(x+1, y).get_typeW() != WATER &&
              map_data(x, y-1).get_typeS() == WATER &&
              map_data(x-1, y).get_typeE() == WATER)
            {
              map_data(x,y).set_type(WATER, GROUND, Tile::BT_SPLIT);
            }
          if (map_data(x, y+1).get_typeN() != WATER &&
              map_data(x-1, y).get_typeE() != WATER &&
              map_data(x, y-1).get_typeS() == WATER &&
              map_data(x+1, y).get_typeW() == WATER)
            {
              map_data(x,y).set_type(GROUND, WATER, Tile::TB_SPLIT);
            }
          if (map_data(x, y-1).get_typeS() != WATER &&
              map_data(x+1, y).get_typeW() != WATER &&
              map_data(x, y+1).get_typeN() == WATER &&
              map_data(x-1, y).get_typeE() == WATER)
            {
              map_data(x,y).set_type(WATER, GROUND, Tile::TB_SPLIT);
            }
        }
      }
    }
  }

  world << "<TILES>" << endl;
  map_data.write(world);
  world << "</TILES>" << endl;

  // mark the tiles on the boundary, stop object creation there

  if (rotate) {

    FORS (i, tile_n) {
      sint4 x = i % tiles_x;
      sint4 y = i / tiles_x;

      if ((x + y <= gtiles_y-1) ||
          (tiles_x - x + tiles_y - y <= gtiles_y+1) ||
          (tiles_x - x + y <= gtiles_x) ||
          (x + tiles_y - y <= gtiles_x))
        { bad[i] = 1; continue; }
    }
  }

  string fname = opt.bp_file;
  fname = sanitize_path(fname);
  parse_map_include(world, fname);

  world << "<OBJECTS>\n";

  sint4 tile_points = GameConst::TILE_POINTS;  // fixme: should depend on game instance

  sint4 nobjs = opt.nobjs;

  const bool AIR_UNITS = true;
  const bool LAND_UNITS = true;
  
  if (AIR_UNITS && LAND_UNITS) nobjs /= 2;

  if (tiles_x > 40 && tiles_y > 60) {

    Vector<string> units;
    units.push_back("bunker");
    units.push_back("barracks");
    units.push_back("mechBay");
    units.push_back("airBase");
    units.push_back("airTower");
    units.push_back("extractor");
    units.push_back("academy");
    units.push_back("supplyCache");
    units.push_back("comsat");
    units.push_back("engineering");
    units.push_back("factory_shop");
    units.push_back("theoretical_lab");
    units.push_back("phys_ext");
    units.push_back("inf_ext");
    units.push_back("armory");
    units.push_back("nukeSilo");
    units.push_back("turret");
    units.push_back("controlCenter");

    units.push_back("marine");
    units.push_back("spy");
    units.push_back("medic");
    units.push_back("toaster");
    units.push_back("mech");
    units.push_back("hoverbike");
    units.push_back("tank");

    units.push_back("flyingLab");
    units.push_back("dropship");
    units.push_back("striker");


    units.push_back("xenopsyl_nest");
    units.push_back("wyrmhole");
    units.push_back("mutator");
    units.push_back("reservoir");
    units.push_back("hive");
    units.push_back("nursery");
    units.push_back("mound");
    units.push_back("tor");
    units.push_back("citadel");

    units.push_back("vector");
    units.push_back("invicor");
    units.push_back("crenator");
    units.push_back("chalcix");
    units.push_back("xenopsyl");
    units.push_back("hercantus");

    units.push_back("hylecoete");
    units.push_back("acherops");
    units.push_back("myrmec");

    sint4 sx = 3;
    sint4 sy = 3;
    FORU (i, units.size()) {
      string t = units[i];
      place_unit(world, t, 0, sx, sy);
      FORU (j, 6) {
        FORU (k, 6) {
          bad[(sx-2+j) + tiles_x*(sy-2+k)] = 16;
        }
      }
      sx += 6;
      if (sx > 37) {
        sy += 6;
        sx = 3;
      }
    }
  }

#if 0
  // create a mineral patch close to the control center
  if (tiles_x > 40 && tiles_y > 60) {
    sint4 x0,y0;
    x0 = 24;
    y0 = 24;

    FORS (j, 10) {
      sint4 x = x0 - 2 + rand.rand_uint4() % 5;
      sint4 y = y0 - 2 + rand.rand_uint4() % 5;
      if (x < 1 || x >= tiles_x-1) continue;
      if (y < 1 || y >= tiles_y-1) continue;

      FORU (u, 3) {
        FORU (v, 3) {
          bad[(x-1+u) + tiles_x*(y-1+v)] = 2;
        }
      }

      world << "mineral" << endl;
      world << "<INT_ATTR>" << endl;
      world << "owner " << num_players << endl;
      world << "x " << x * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "y " << y * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "</INT_ATTR>" << endl;
    }

    FORU (i, 16) {
      FORU (j, 16) {
        bad[i + tiles_x*j] = 32; // freespace
      }
    }
  }
#endif


  // create random clusters of minerals

#if 1
  FORS (i, 7) {
    sint4 x0, y0;
    sint4 t = rand.rand_uint4() % tile_n;
    x0 = t % tiles_x;
    y0 = t / tiles_y;
    if (bad[t] & ~2) { --i; continue; }

    FORS (j, 10) {
      sint4 x = x0 - 2 + rand.rand_uint4() % 5;
      sint4 y = y0 - 2 + rand.rand_uint4() % 5;
      if (x < 1 || x >= tiles_x-1) continue;
      if (y < 1 || y >= tiles_y-1) continue;
      bool can_use = true;
      FORU (u, 3) {
        FORU (v, 3) {
          if (bad[(x-1+u) + tiles_x*(y-1+v)] & ~2) can_use = false;
        }
      }

      if (!can_use) continue;

      FORU (u, 3) {
        FORU (v, 3) {
          bad[(x-1+u) + tiles_x*(y-1+v)] = 2;
        }
      }

      world << "mineral" << endl;
      world << "<INT_ATTR>" << endl;
      world << "owner " << num_players << endl;
      world << "x " << x * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "y " << y * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "</INT_ATTR>" << endl;
    }
  }
#endif

  // create random geysers
#if 1
  FORS (i, 10) {
    sint4 x0, y0;
    sint4 t = rand.rand_uint4() % tile_n;
    x0 = t % tiles_x;
    y0 = t / tiles_y;
    if (bad[t] & ~4) continue;

    FORS (j, 10) {
      sint4 x = x0 - 2 + rand.rand_uint4() % 5;
      sint4 y = y0 - 2 + rand.rand_uint4() % 5;
      if (x < 1 || x >= tiles_x-1) continue;
      if (y < 1 || y >= tiles_y-1) continue;
      bool can_use = true;
      FORU (u, 3) {
        FORU (v, 3) {
          if (bad[(x-1+u) + tiles_x*(y-1+v)]) can_use = false;
        }
      }
      if (!can_use) continue;
      FORU (u, 3) {
        FORU (v, 3) {
          bad[(x-1+u) + tiles_x*(y-1+v)] = 4;
        }
      }
      world << "geyser" << endl;
      world << "<INT_ATTR>" << endl;
      world << "owner " << num_players << endl;
      world << "x " << x * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "y " << y * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "</INT_ATTR>" << endl;
    }
  }
#endif

  FORU (p, num_players) {

    if (AIR_UNITS) {

      FORS (i, nobjs) {
        sint4 r;
        do { r = rand.rand_uint4() % tile_n; } while (bad[r]);
	bad[r] = 1;

        place_unit(world, "destroyer", p, r % tiles_x, r / tiles_x);
      }
    }

    if (LAND_UNITS) { 

      FORS (i, nobjs) {
        sint4 r;
        do { r = rand.rand_uint4() % tile_n; } while (bad[r]);
	bad[r] = 1;

        place_unit(world, "worker", p, r % tiles_x, r / tiles_x);
      }
    }
  }

  // create random clusters of trees
#if 1
  sint4 tree1 = opt.tree1;
  sint4 tree2 = opt.tree2;

  FORS (i, tree1) {
    sint4 x0,y0;
    sint4 t = rand.rand_uint4() % tile_n;
    x0 = t % tiles_x;
    y0 = t / tiles_y;
    if (bad[t] & ~(1 | 8)) { --i; continue; }

    FORS (j, tree2) {
      sint4 x = x0 - 3 + rand.rand_uint4() % 7;
      sint4 y = y0 - 3 + rand.rand_uint4() % 7;
      if (x < 1 || x >= tiles_x-1) continue;
      if (y < 1 || y >= tiles_y-1) continue;
      bool can_use = true;

      FORU (u, 3) {
        FORU (v, 3) {
          if (bad[(x-1+u) + tiles_x*(y-1+v)] & ~(1 | 8)) can_use = false;
        }
      }
      if (!can_use) continue;

      FORU (u, 3) {
        FORU (v, 3) {
          bad[(x-1+u) + tiles_x*(y-1+v)] = 8;
        }
      }
      world << "tree" << endl;
      world << "<INT_ATTR>" << endl;
      world << "owner " << num_players << endl;
      world << "x " << x * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "y " << y * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "</INT_ATTR>" << endl;
    }
  }
#endif

  world << "</OBJECTS>\n";
}

//==============================================================================

/* hfile format:

  tiles_x (=tiles_y) mult (what height corresponds to HEIGHT_MULT (must divide HEIGHT_MULT)

  tiles_x+1 height values (height y = 0)
  tiles_x ramp flags      (tile y = 0)       -:cliff *:ground

  tiles_x+1 height values (y = 1)
  tiles_x ramp flags      (tile y = 1)  

  ...
  
  tiles_x+1 height values (y = tiles_y+1)  
*/

void MapTool::read_height_map(stringstream &world)
{
  opt.get();

  Random rand;
  sint4 seed = opt.seed;
  rand.srand(seed);
  
  // note: if loading a map, -nplayers values must match
  sint4 num_players = opt.nplayers;

  ifstream ifs;

  string hfile = opt.hfile;
  hfile = sanitize_path(hfile);
  ifs.open(hfile.c_str());

  if (!ifs.good()) ERR2("can't find file ", hfile);

  sint4 gtiles_x, gtiles_y;  
  ifs >> gtiles_x;
  if (!ifs.good()) ERR("expected width/height");

  sint4 mult;
  ifs >> mult;  // what height corresponds to HEIGHT_MULT (must divide HEIGHT_MULT)
  if (!ifs.good() || mult <= 0) ERR("expected multiplier > 0");

  if ((GameConst::HEIGHT_MULT) % mult != 0) ERR("HEIGHT_MULT is not a multiplier of mult"); 
  
  gtiles_y = gtiles_x;
  sint4 tiles_x, tiles_y;

  opt.width = gtiles_x;
  opt.height = gtiles_y;
  ::Options::set("-x", gtiles_x);
  ::Options::set("-y", gtiles_y);

  sint4 z_sight = 0;

  bool rotate = 0; // no rotation
  opt.rot = rotate;
  ::Options::set("-rot", rotate);

  //  sint4 no = 10;   // only a few objects
  //opt.put("-nobjs", no);
  
  tiles_x = gtiles_x;
  tiles_y = gtiles_y;

  world << num_players << endl;
  world << gtiles_x << endl;
  world << gtiles_y << endl;
  world << z_sight << endl;
  if (rotate) world << 1 << endl;
  else        world << 0 << endl;

  sint4 tile_n = tiles_x * tiles_y;

  Map<Tile> map_data;
  map_data.allocate(tiles_x, tiles_y);

  sint4 h_x = tiles_x + 1;
  sint4 h_y = tiles_y + 1;  
  sint4 h_n = h_x * h_y;

  Vector<sint2> height(h_n);   // height field (index is not a tile index!)
  Vector<sint2> ramps(tile_n); // ramp flag for each tile
  Vector<sint2> bad(tile_n);   // do not put stuff on tile if true, works on tiles!

  FORS (y, h_y) {

    // read height values
    
    FORS (x, h_x) {
      sint4 h;
      ifs >> h;
      if (!ifs.good() || h < 0) {
	stringstream t; t << "y=" << y << " x=" << x;
	ERR2("expected height >= 0 at ", t.str());
      }
      height[y * h_x + x] = h * GameConst::HEIGHT_MULT / mult;
    }

    if (y != h_y-1) {

      // read ramp flags
      
      FORS (x, tiles_x) {
	char r;
	ifs >> r;
	if (!ifs.good() || (r != '-' && r != '*')) {
	  stringstream t; t << "y=" << y << " x=" << x;
	  ERR2("expected ramp flag at ", t.str());
	}
	ramps[y * tiles_x + x] = (r != '-');
      }
    }
  }

  char c; ifs >> c; if (!ifs.eof()) ERR("end of file expected");
  
  // set tile types
  
  FORS (i, h_n) {

    sint4 x = i % h_x;
    sint4 y = i / h_x;

    if (x == h_x-1 || y == h_y-1) continue;

    //    cout << height[i] << " ";
    
    sint4 h[4];
    
    h[0] = height[i];
    h[1] = height[i+1];
    h[2] = height[i+h_x+1];
    h[3] = height[i+h_x];

    Tile &tile = map_data.get_tile(x, y);

    tile.set_topo(h[0], h[1], h[2], h[3], Tile::NO_SPLIT);
    tile.set_type(GROUND);

    assert(x >= 0 && x < tiles_x);	
    assert(y >= 0 && y < tiles_y);
    
    bool ramp = ramps[y * tiles_x + x];
    Tile::Type tt;
    if (ramp) tt = GROUND; else tt = CLIFF;

    // equal heights -> regular ground tile
    
    if (h[0] == h[1] && h[0] == h[2] && h[0] == h[3]) {

      tile.set_topo(h[0], h[1], h[2], h[3], Tile::NO_SPLIT);
      tile.set_type(GROUND);
      continue;
    }

    // 3 equal heights -> split
    //  - make equal height GROUND triangle and the other CLIFF or
    //  - create ridge
    
    {
      if ((h[0] == h[1] && h[1] == h[2]) ||
          (h[0] == h[1] && h[1] == h[3]) ||
          (h[0] == h[2] && h[2] == h[3]) ||
          (h[1] == h[2] && h[2] == h[3])) {

        sint4 i;
        FOR (i, 4) {
          if (h[i] != h[(i+1)&3] && h[i] != h[(i+2)&3]) break;
        }

	if (1 || (rand.rand_uint4() & 8)) { // no ridge for now

	  // equal height triangle
	  
	  if (i == 0 || i == 2) {  // |/|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::BT_SPLIT);
	  } else { // |\|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::TB_SPLIT);
	  }

	  if (i == 0 || i == 3) tile.set_type(tt, GROUND); // west CLIFF
	  else                  tile.set_type(GROUND, tt); // east CLIFF

	} else {

	  // ridge

	  if (i == 0 || i == 2) {  // |\|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::TB_SPLIT);
	  } else { // |/|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::BT_SPLIT);
	  }

	  tile.set_type(CLIFF, CLIFF);
	}
	assert(x >= 0 && x < tiles_x);	
	assert(y >= 0 && y < tiles_y);
	bad[y*tiles_x+x] = 1;
	continue;
      }
    }

    
    // 2 and 2 equal heights -> full CLIFF tile
    
    {
      sint4 i;
      FOR (i, 4) {
	if (h[i] == h[(i+1) & 3] && h[(i+2) & 3] == h[(i+3) & 3]) break;
      }
    
      if (i < 4) {

	tile.set_topo(h[0], h[1], h[2], h[3], Tile::NO_SPLIT);

	tile.set_type(tt);

	assert(x >= 0 && x < tiles_x);	
	assert(y >= 0 && y < tiles_y);
	bad[y*tiles_x+x] = 1;
	continue;
      }
    }

    // otherwise: random CLIFF/CLIFF (or GROUND/GROUND) split

    {
      tile.set_type(tt, tt);
   
      if (rand.rand_uint4() & 8) {
	tile.set_topo(h[0], h[1], h[2], h[3], Tile::BT_SPLIT);
      } else {
	tile.set_topo(h[0], h[1], h[2], h[3], Tile::TB_SPLIT);
      }

      assert(x >= 0 && x < tiles_x);	
      assert(y >= 0 && y < tiles_y);
      bad[y*tiles_x+x] = 1;
    }
  }

  world << "<TILES>" << endl;
  map_data.write(world);
  world << "</TILES>" << endl;

  // mark the tiles on the boundary, stop object creation there

  string fname = opt.bp_file;
  fname = sanitize_path(fname);
  parse_map_include(world, fname);

  world << "<OBJECTS>\n";

  sint4 nobjs = opt.nobjs;

  const bool AIR_UNITS = true;
  const bool LAND_UNITS = true;
  
  if (AIR_UNITS && LAND_UNITS) nobjs /= 2;

  FORS (p, num_players) {

    if (AIR_UNITS) {

      FORS (i, nobjs) {
        sint4 r;
        do { r = rand.rand_uint4() % tile_n; } while (bad[r]);
	bad[r] = 1;

        place_unit(world, "destroyer", p, r % tiles_x, r / tiles_x);
      }
    }

    if (LAND_UNITS) { 

      FORS (i, nobjs) {
        sint4 r;
        do { r = rand.rand_uint4() % tile_n; } while (bad[r]);
	bad[r] = 1;

        place_unit(world, "worker", p, r % tiles_x, r / tiles_x);
      }
    }
  }

  world << "</OBJECTS>\n";
}

//==============================================================================

// Initialize the world with a maze map.

void MapTool::generate_maze_map(stringstream &world)
{
  opt.get();

  Random rand;
  sint4 seed = opt.seed;
  rand.srand(seed);

  // note: if loading a map, -nplayers values must match
  sint4 num_players = opt.nplayers;
  
  sint4 tiles_x, tiles_y;
  sint4 gtiles_x = opt.width;
  sint4 gtiles_y = opt.height;

  sint4 z_sight = 0;

  tiles_x = gtiles_x;
  tiles_y = gtiles_y;

  world << num_players << endl;
  world << gtiles_x << endl;
  world << gtiles_y << endl;
  world << z_sight << endl;
  world << 0 << endl;

  sint4 tile_n = tiles_x * tiles_y;

  Map<Tile> map_data;
  map_data.allocate(tiles_x, tiles_y);

  sint4 **hmap = new sint4*[tiles_x];
  FORS (x, tiles_x) {
    hmap[x] = new sint4[tiles_y];
    FORS (y, tiles_y) {
      hmap[x][y] = 0;
    }
  }
  
  
  FORS (i, 6*tiles_x*tiles_y) {

    sint4 x = (rand.rand_uint4()%tiles_x)&(~0x1); // only path on even tiles
    sint4 y = (rand.rand_uint4()%tiles_y)&(~0x1);
    
    if (hmap[x][y] <= 1)
    {
      switch(rand.rand_uint4()%4)
      {
      case 0: // NORTH
        if ((x > 1) && (hmap[x-2][y]+hmap[x][y] < 2))
        { hmap[x-2][y]++; hmap[x-1][y]++; hmap[x][y]++; }
        break;
      case 1: // SOUTH
        if ((x < tiles_x-2) && (hmap[x+2][y]+hmap[x][y] < 2))
        { hmap[x+2][y]++; hmap[x+1][y]++; hmap[x][y]++; }
        break;
      case 2: // WEST
        if ((y > 1) && (hmap[x][y-2]+hmap[x][y] < 2))
        { hmap[x][y-2]++; hmap[x][y-1]++; hmap[x][y]++; }
        break;
      case 3: // EAST
        if ((y < tiles_y-2) && (hmap[x][y+2]+hmap[x][y] < 2))
        { hmap[x][y+2]++; hmap[x][y+1]++; hmap[x][y]++; }
        break;
      }
    }
  }

  FORS (x, tiles_x) {
    FORS (y, tiles_y) {
      map_data(x,y).set_flat(hmap[x][y] ? 0:2);
      map_data(x,y).set_type(GROUND);
    }
  }

  world << "<TILES>" << endl;
  map_data.write(world);
  world << "</TILES>" << endl;

  string fname = opt.bp_file;
  fname = sanitize_path(fname);
  parse_map_include(world, fname);

  world << "<OBJECTS>\n";

  sint4 nobjs = opt.nobjs;

  nobjs = 2;

  const bool AIR_UNITS = true;
  const bool LAND_UNITS = true;

  if (AIR_UNITS && LAND_UNITS) nobjs /= 2;

  FORS (p, num_players) {
    if (AIR_UNITS) {
      FORS (i, nobjs) {
        sint4 r;
        r = rand.rand_uint4() % tile_n;
        
        place_unit(world, "destroyer", p, r % tiles_x, r / tiles_x);
      }
    }
    if (LAND_UNITS) { 
      FORS (i, nobjs) {
        sint4 r;
        r = rand.rand_uint4() % tile_n;

        place_unit(world, "worker", p, r % tiles_x, r / tiles_x);
      }
    }
  }

  world << "</OBJECTS>\n";

  FORS (x, tiles_x) delete [] hmap[x];
  delete [] hmap;
}

//==============================================================================

// Initialize the world with a capture the flag map.
void MapTool::generate_ctf_map(
  stringstream &world, 
  bool test_mode)
{
  // Create a fixed, simple capture-the-flag world

  const sint4 num_players = 2;  // always 2 players
  
  sint4 tiles_x, tiles_y;
  sint4 gtiles_x, gtiles_y;

  // use a fixed width and height
  gtiles_x = 20;
  gtiles_y = 20;

  sint4 z_sight = 0;

  const bool rotate = true;

  if (rotate) {
    gtiles_x = sint4(gtiles_x * 0.70710678);
    gtiles_y = sint4(gtiles_y * 0.70710678);
    tiles_x = gtiles_x + gtiles_y;
    tiles_y = gtiles_x + gtiles_y;
  } else {
    tiles_x = gtiles_x;
    tiles_y = gtiles_y;
  }

  world << num_players << endl;
  world << gtiles_x << endl;
  world << gtiles_y << endl;
  world << z_sight << endl;
  if (rotate) world << 1 << endl;
  else        world << 0 << endl;

  sint4 tile_n = tiles_x * tiles_y;

  Map<Tile> map_data;
  map_data.allocate(tiles_x, tiles_y);

  sint2 *height = new sint2[tile_n];
  sint2 *ramp_d = new sint2[tile_n];
  if (!height || !ramp_d) ERR("out of memory");
  memset(height, 0, tile_n*sizeof(*height));
  memset(ramp_d, 0, tile_n*sizeof(*ramp_d));

#ifndef CTF_NO_WALLS
  
  // set plateaus to be used as walls, since there are no ramps
  FORS (i, tile_n) {
    sint4 x = i % tiles_x;
    sint4 y = i / tiles_x;

#ifndef CTF_SIMPLE
    // define horizontal walls
    if ((y == 1) && (x >= 12) && (x < 16)) height[i] = 1;
    if ((y == 7) && (x >= 10) && (x < 17)) height[i] = 1;
    if ((y == 10) && (x >= 10) && (x < 17)) height[i] = 1;
      
    if ((y == 12) && (x >= 7) && (x < 12)) height[i] = 1;
      
#ifdef EXTRA_WALL
    if ((y == 13) && (x >= 12) && (x < 15)) height[i] = 1;
    if ((y == 14) && (x >= 12) && (x < 15)) height[i] = 1;
#endif
    
    if ((y == 15) && (x >= 15) && (x < 21)) height[i] = 1;
      
    if ((y == 17) && (x >= 10) && (x < 17)) height[i] = 1;
    if ((y == 20) && (x >= 10) && (x < 17)) height[i] = 1;
    if ((y == 26) && (x >= 12) && (x < 16)) height[i] = 1;
	
    // define vertical walls
    
    if ((x == 1) && (y >= 12) && (y < 16)) height[i] = 1;
    if ((x == 6) && (y >= 11) && (y < 17)) height[i] = 1;
      
    if ((x == 11) && (y >= 10) && (y < 15)) height[i] = 1;
    if ((x == 15) && (y >= 13) && (y < 18)) height[i] = 1;
      
    if ((x == 21) && (y >= 11) && (y < 17)) height[i] = 1;
    if ((x == 26) && (y >= 12) && (y < 16)) height[i] = 1;

#else
    // minimal walls
    if ((y == 1) && (x >= 12) && (x < 16)) height[i] = 1;
    if ((y == 10) && (x >= 12) && (x < 15)) height[i] = 1;
    if ((y == 17) && (x >= 12) && (x < 15)) height[i] = 1;
    if ((y == 26) && (x >= 12) && (x < 16)) height[i] = 1;
  
    if ((x == 1) && (y >= 12) && (y < 16)) height[i] = 1;
    if ((x == 26) && (y >= 12) && (y < 16)) height[i] = 1;
#endif

  }

#endif //CTF_NO_WALLS
  
  // mark tiles that aren't part of the board
  if (rotate) {
    FORS (i, tile_n) {
      sint4 x = i % tiles_x;
      sint4 y = i / tiles_x;

      if ((x + y < gtiles_y-1) ||
          (tiles_x - x + tiles_y - y < gtiles_y+1) ||
          (tiles_x - x + y < gtiles_x) ||
          (x + tiles_y - y < gtiles_x))
        { height[i] = -2; continue; }
    }
  }

  FORS (i, tile_n) {

    if (height[i] >= 0) {
      map_data(i).set_type(GROUND);

      if (ramp_d[i]) {
	if (ramp_d[i] <= 8) {
	  map_data(i).set_ramp(height[i], height[i]+1,
			       static_cast<Tile::Compass>(ramp_d[i]));
	} else {
	  map_data(i).set_ramp(height[i], height[i]+1, height[i]+1,
			       static_cast<Tile::Compass>(ramp_d[i]-10));
	}
      } else {
        map_data(i).set_flat(height[i]);
      }

    } else {
      map_data(i).set_type(Tile::UNKNOWN);
    }
  }

  world << "<TILES>" << endl;
  map_data.write(world);
  world << "</TILES>" << endl;

  string fname = "data/ctf_units.bp";  // only supported blueprint file
  //fname = opt.bp_file;
  fname = sanitize_path(fname);
  parse_map_include(world, fname);

  
  world << "<OBJECTS>\n";

  // place player 0 units

#ifdef CTF_FLIP_SIDES
  const sint4 side0 = 1;
  const sint4 side1 = 0;
#else
  const sint4 side0 = 0;
  const sint4 side1 = 1;
#endif //CTF_FLIP_SIDES
  
#ifndef OMIT_FLAG_AND_POST
  place_unit(world, "ctf_post", side0, 13, 3);
  place_unit(world, "ctf_flag", side0, 14, 3);
#endif
  
#ifndef CTF_EASY_WIN
  FORS (i, 5) {
    place_unit(world, "ctf_unit", side0, 11 + i, 5);
  }
#else
  // one unit, easy win!
  place_unit(world, "ctf_unit", side0, 11, 5);
#endif //CTF_EASY_WIN
  
  // place player 1 units

#ifndef OMIT_FLAG_AND_POST
  place_unit(world, "ctf_post", side1, 13, 24);
  place_unit(world, "ctf_flag", side1, 14, 24);
#endif

  // if in test mode, place no units (pure CTF time test)
  if (!test_mode) {  
#ifndef CTF_EASY_WIN
    FORS (i, 5) {
      place_unit(world, "ctf_unit", side1, 11 + i, 22);
    }
#else
    // one unit, easy win!
    place_unit(world, "ctf_unit", side1, 11, 22);
#endif //CTF_EASY_WIN
  }
  
  world << "</OBJECTS>\n";

  delete [] height;
  delete [] ramp_d;
}


/** Writes a map file, given a full description of the world.
    The filename is taken from the command-line options, and if
    left blank, stdout is used.
*/

void MapTool::write_map_file(const stringstream &world, const string &out_fname)
{
  string f = out_fname;
  f = sanitize_path(f);

  if (f.empty()) {

    // for debugging, print out world.str()
    cout << world.str() << endl;

  } else {
    // commit output to file
    ofstream out_file(f.c_str());
    if (!out_file.is_open()) ERR("error opening output file");
    out_file << world.str() << endl;
    out_file.close();
  }
}



//===================================================================

// generate random map with CLIFF tiles
// copied generate_random_cliff_map

void MapTool::generate_marine_map(stringstream &world)
{
  opt.get();

  Random rand;
  sint4 seed = opt.seed;
  rand.srand(seed);

  // note: if loading a map, -nplayers values must match
  sint4 num_players = opt.nplayers;
  
  sint4 tiles_x, tiles_y;
  sint4 gtiles_x = opt.width;
  sint4 gtiles_y = opt.height;
  sint4 z_sight = 0;

  bool rotate = opt.rot;
  CREATE_WATER = opt.water;

  // if the world is rotated 45 degrees, impose somewhat artificial 45
  // degree bariers to define the playing area.  The area of
  // the playing field is the same as it would have been, but the
  // world is still an axis aligned rectangle, so the actual world may
  // be much larger

  /*
    +-----+
    |  /\ |
    | /  \|
    |/   /|
    |\  / |
    | \/  |
    +-----+
  */

  // tiles_x is the actual width of the tile array stored
  // gtiles_x is the width of the playfield and is constant under rotation

  if (rotate) {
    gtiles_x = sint4(gtiles_x * 0.70710678); // side lengths are now fractions of 45 degree lines
    gtiles_y = sint4(gtiles_y * 0.70710678);
    tiles_x = gtiles_x + gtiles_y;
    tiles_y = gtiles_x + gtiles_y;
  } else {
    tiles_x = gtiles_x;
    tiles_y = gtiles_y;
  }

  world << num_players << endl;
  world << gtiles_x << endl;
  world << gtiles_y << endl;
  world << z_sight << endl;
  if (rotate) world << 1 << endl;
  else        world << 0 << endl;

  sint4 tile_n = tiles_x * tiles_y;

  Map<Tile> map_data;
  map_data.allocate(tiles_x, tiles_y);

  sint4 h_x = tiles_x + 1;
  sint4 h_y = tiles_y + 1;  
  sint4 h_n = h_x * h_y;

  Vector<sint2> height(h_n); // height field (index is not a tile index!)
  Vector<sint2> bad(tile_n); // do not put stuff on tile if true, works on tiles!

  // first heights heights represent layers (will be multiplied by GameConst::HEIGHT_MULT later

  real8 fplat = opt.fplat;
  bool plug_holes = opt.holes;
  sint4 clump = opt.clump;
  bool smooth = opt.smooth;

  if (smooth) plug_holes = false; // smooth > plug_holes

  if (clump < 1) clump = 1;

  if (clump <= 1) {

    FORS (i, sint4(tile_n * fplat)) {
      sint4 r;
      do { r = rand.rand_uint4() % h_n; } while (height[r] == 1);
      height[r] = 1;
    }

  } else {

    FORS (i, sint4(tile_n * fplat)) {
      sint4 r;
      do { r = rand.rand_uint4() % h_n; } while (height[r] >= 3);
    
      sint4 next_to = 0;
      sint4 out_of = 4;
    
      sint4 x = r % h_x;
      sint4 y = r / h_x;
  
      if (x == 0 || x == h_x-1) out_of--;
      if (y == 0 || y == h_y-1) out_of--;
    
      sint4 h = height[r];

      if (x > 0     && height[r-1]   == h+1) next_to++;
      if (x < h_x-1 && height[r+1]   == h+1) next_to++;
      if (y > 0     && height[r-h_x] == h+1) next_to++;
      if (y < h_y-1 && height[r+h_x] == h+1) next_to++;

      const sint4 offset = 4;

      if (out_of == next_to) {
	if (rand.rand_uint4() % 3) height[r] += 1;
	else if (h <= 1) height[r] += 2;
        continue;
      }
      if (next_to * 2 >= out_of) {
        next_to += offset;
        out_of += offset;
      }
      if (next_to > 0) {
        if ((sint4)(rand.rand_uint4() % out_of) < next_to) {
	  height[r] += 1;
	  continue;
	}
      }
      if (rand.rand_uint4() % clump < 1) {
        height[r]++; // = 1;
        continue;
      }
      --i;
    }
  }

  if (plug_holes) {

    FORS (i, h_n) {
      sint4 r = i;

      sint4 next_to = 0;
      sint4 out_of = 4;

      sint4 rx = r % h_x;
      sint4 ry = r / h_x;

      sint4 h = height[r];
    
      if (rx == 0 || rx == h_x-1) out_of--;
      if (ry == 0 || ry == h_y-1) out_of--;

      if (rx > 0     && height[r-1]   > h) next_to++;
      if (rx < h_x-1 && height[r+1]   > h) next_to++;
      if (ry > 0     && height[r-h_x] > h) next_to++;
      if (ry < h_y-1 && height[r+h_x] > h) next_to++;

      if (out_of == next_to) {// && height[r] == 0) {
        //	cout << i << " " << next_to << " " << out_of << " plug" << endl;
        height[r]++;// = 1;
      }
    }
  }

  // the world is now a height field with no ramps
  // flatten the corner and add demo buildings

  if (tiles_x > 40 && tiles_y > 60) {
    FORU (i, 40) {
      FORU (j, 60) {
        height[i + j*h_x] = 0;
      }
    }
  }

  // smooth plateaus:
  //  continue filling tiles iff it has three or four filled HV neighbors

  bool changed = smooth;

  while (changed) {

    changed = false;
    
    sint4 dx[] = { 1,  0, -1, 0 };
    sint4 dy[] = { 0, -1,  0, 1 };

    FORS (i, h_n) {

      sint4 h = height[i];

      sint4 ix = i % h_x;
      sint4 iy = i / h_x;

      sint4 next_to = 0;
      
      FORS (j, 4) {
	sint4 jx = ix + dx[j];
	sint4 jy = iy + dy[j];	  
	if (jx < 0 || jx >= h_x || jy < 0 || jy >= h_y) next_to++;
	else if (height[jy*h_x + jx] > h) next_to++;
      }
      
      if (next_to >= 3) { height[i]++; changed = true; }
    }
  }


  FORS (i, h_n) height[i] *= GameConst::HEIGHT_MULT;

  // from here on: heights are multiples of GameConst::HEIGHT_MULT
  // or at half-heights (ramps)
  
  FORS (i, h_n) {

    sint4 x = i % h_x;
    sint4 y = i / h_x;

    if (x == h_x-1 || y == h_y-1) continue;

    //    cout << height[i] << " ";
    
    sint4 h[4];
    
    h[0] = height[i];
    h[1] = height[i+1];
    h[2] = height[i+h_x+1];
    h[3] = height[i+h_x];

    Tile &tile = map_data.get_tile(x, y);

    tile.set_topo(h[0], h[1], h[2], h[3], Tile::NO_SPLIT);
    tile.set_type(GROUND);

    assert(x >= 0 && x < tiles_x);	
    assert(y >= 0 && y < tiles_y);
    
    // equal heights -> regular ground tile
    
    if (h[0] == h[1] && h[0] == h[2] && h[0] == h[3]) {

      tile.set_topo(h[0], h[1], h[2], h[3], Tile::NO_SPLIT);
      tile.set_type(GROUND);
      continue;
    }

    // 3 equal heights -> split
    //  - make equal height GROUND triangle and the other CLIFF or
    //  - create ridge
    
    {
      if ((h[0] == h[1] && h[1] == h[2]) ||
          (h[0] == h[1] && h[1] == h[3]) ||
          (h[0] == h[2] && h[2] == h[3]) ||
          (h[1] == h[2] && h[2] == h[3])) {

        sint4 i;
        FOR (i, 4) {
          if (h[i] != h[(i+1)&3] && h[i] != h[(i+2)&3]) break;
        }

	if (1 || (rand.rand_uint4() & 8)) {

	  // equal height triangle
	  
	  if (i == 0 || i == 2) {  // |/|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::BT_SPLIT);
	  } else { // |\|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::TB_SPLIT);
	  }
      
	  if (i == 0 || i == 3) tile.set_type(CLIFF, GROUND); // west CLIFF
	  else                  tile.set_type(GROUND, CLIFF); // east CLIFF

	} else {

	  // ridge

	  if (i == 0 || i == 2) {  // |\|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::TB_SPLIT);
	  } else { // |/|
	    tile.set_topo(h[0], h[1], h[2], h[3], Tile::BT_SPLIT);
	  }

	  tile.set_type(CLIFF, CLIFF);
	}
	assert(x >= 0 && x < tiles_x);	
	assert(y >= 0 && y < tiles_y);
	bad[y*tiles_x+x] = 1;
	continue;
      }
    }

    
    // 2 and 2 equal heights -> full CLIFF tile
    
    {
      sint4 i;
      FOR (i, 4) {
	if (h[i] == h[(i+1) & 3] && h[(i+2) & 3] == h[(i+3) & 3]) break;
      }
    
      if (i < 4) {

	tile.set_topo(h[0], h[1], h[2], h[3], Tile::NO_SPLIT);

	// small chance of ramp
	if (rand.rand_uint4() % 100 < 15) tile.set_type(GROUND);
	else                              tile.set_type(CLIFF);

	assert(x >= 0 && x < tiles_x);	
	assert(y >= 0 && y < tiles_y);
	bad[y*tiles_x+x] = 1;
	continue;
      }
    }

    // otherwise: random CLIFF/CLIFF split

    {
      tile.set_type(CLIFF, CLIFF);
   
      if (rand.rand_uint4() & 8) {
	tile.set_topo(h[0], h[1], h[2], h[3], Tile::BT_SPLIT);
      } else {
	tile.set_topo(h[0], h[1], h[2], h[3], Tile::TB_SPLIT);
      }

      assert(x >= 0 && x < tiles_x);	
      assert(y >= 0 && y < tiles_y);
      bad[y*tiles_x+x] = 1;
    }
  }

  if (rotate) {

    FORS (i, tile_n) {
      sint4 x = i % tiles_x;
      sint4 y = i / tiles_x;

      if ((x + y < gtiles_y-1) ||
          (tiles_x - x + tiles_y - y < gtiles_y+1) ||
          (tiles_x - x + y < gtiles_x) ||
          (x + tiles_y - y < gtiles_x)) {
        map_data(x,y).set_type(Tile::UNKNOWN);
      }
    }
  }

  FORS (i, tile_n) {

    sint4 x = i % tiles_x;
    sint4 y = i / tiles_x;

    Tile &t = map_data.get_tile(i);
    Tile::Type tw, te;
    Tile::Split split;
    t.get_type(tw, te);
    sint4 h0, h1, h2, h3;
    t.get_topo(h0, h1, h2, h3, split);

    if (split == Tile::NO_SPLIT && tw != CLIFF && (h0 != h3 || h0 != h1)) {

      // ramp

      if (h0 != h3) {

	// | ramp

	if (x > 0) {  // extend left
	  Tile &t = map_data.get_tile(x-1, y);
	  t.get_type(tw, te);
	  t.get_topo(h0, h1, h2, h3, split);

	  if (rand.rand_uint4() % 100 < 50 && te == CLIFF) {
	    if (split) t.set_type(tw, GROUND);
	    else       t.set_type(GROUND, GROUND);
	  }
	}
	
	if (x < tiles_x-1) {  // extend right
	  Tile &t = map_data.get_tile(x+1, y);
	  t.get_type(tw, te);
	  t.get_topo(h0, h1, h2, h3, split);

	  if (rand.rand_uint4() % 100 < 50 && tw == CLIFF) {
	    if (split) t.set_type(GROUND, te);
	    else       t.set_type(GROUND, GROUND);
	  }
	}

      } else {

	// - ramp

 	if (y > 0) {  // extend up
	  Tile &t = map_data.get_tile(x, y-1);
	  t.get_type(tw, te);
	  t.get_topo(h0, h1, h2, h3, split);

	  if (rand.rand_uint4() % 100 < 50 &&
	      ((!split && tw == CLIFF) ||                    // | |
	       (split == Tile::TB_SPLIT && tw == CLIFF) ||   // |\|
	       (split == Tile::BT_SPLIT && te == CLIFF))) {  // |/|

	    if   (!split)                     t.set_type(GROUND, GROUND);
	    else if (split == Tile::TB_SPLIT) t.set_type(GROUND, te);
	    else                              t.set_type(tw, GROUND);
	  }
	}
	
	if (y < tiles_y-1) {  // extend down
	  Tile &t = map_data.get_tile(x, y+1);
	  t.get_type(tw, te);
	  t.get_topo(h0, h1, h2, h3, split);

	  if (rand.rand_uint4() % 100 < 50 &&
	      ((!split && tw == CLIFF) ||                    // | |
	       (split == Tile::TB_SPLIT && te == CLIFF) ||   // |\|
	       (split == Tile::BT_SPLIT && tw == CLIFF))) {  // |/|

	    if   (!split   )                  t.set_type(GROUND, GROUND);
	    else if (split == Tile::TB_SPLIT) t.set_type(tw, GROUND);
	    else                              t.set_type(GROUND, te);
	  }
	}
      }
    }
  }
  
  if (CREATE_WATER) {

    // create lakes
    FORS (i, 20) {

      const sint4 D = 5;
      sint4 x, y;
      Tile::Type tw, te;
      do {
	x = rand.rand_uint4() % (tiles_x-D);
	y = rand.rand_uint4() % (tiles_y-D);
	map_data(x,y).get_type(tw, te);
      } while (!map_data(x,y).is_flat() || map_data(x,y).get_min_h() != 0 || tw == Tile::UNKNOWN);

      FORS (dx, D) {
        FORS (dy, D) {
          if (!map_data(x+dx, y+dy).is_flat()) break;
          if (map_data(x+dx, y+dy).get_min_h() != map_data(x,y).get_min_h()) break;
          map_data(x+dx, y+dy).set_type(WATER);
        }
      }
    }

    // smooth corners
    for (sint4 y = 1; y < tiles_y-2; y++) {
      for (sint4 x = 1; x < tiles_x-2; x++) {
        
        Tile::Type a, b;
        map_data(x,y).get_type(a, b);

        if (a == b && b == WATER) {
	  // sint4 h[4];
	  // Tile::Split split;

          if (map_data(x, y-1).get_typeS() != WATER &&
              map_data(x-1, y).get_typeE() != WATER &&
              map_data(x, y+1).get_typeN() == WATER &&
              map_data(x+1, y).get_typeW() == WATER)
            {
              map_data(x,y).set_type(GROUND, WATER, Tile::BT_SPLIT);
            }
          if (map_data(x, y+1).get_typeN() != WATER &&
              map_data(x+1, y).get_typeW() != WATER &&
              map_data(x, y-1).get_typeS() == WATER &&
              map_data(x-1, y).get_typeE() == WATER)
            {
              map_data(x,y).set_type(WATER, GROUND, Tile::BT_SPLIT);
            }
          if (map_data(x, y+1).get_typeN() != WATER &&
              map_data(x-1, y).get_typeE() != WATER &&
              map_data(x, y-1).get_typeS() == WATER &&
              map_data(x+1, y).get_typeW() == WATER)
            {
              map_data(x,y).set_type(GROUND, WATER, Tile::TB_SPLIT);
            }
          if (map_data(x, y-1).get_typeS() != WATER &&
              map_data(x+1, y).get_typeW() != WATER &&
              map_data(x, y+1).get_typeN() == WATER &&
              map_data(x-1, y).get_typeE() == WATER)
            {
              map_data(x,y).set_type(WATER, GROUND, Tile::TB_SPLIT);
            }
        }
      }
    }
  }

  world << "<TILES>" << endl;
  map_data.write(world);
  world << "</TILES>" << endl;

  // mark the tiles on the boundary, stop object creation there

  if (rotate) {

    FORS (i, tile_n) {
      sint4 x = i % tiles_x;
      sint4 y = i / tiles_x;

      if ((x + y <= gtiles_y-1) ||
          (tiles_x - x + tiles_y - y <= gtiles_y+1) ||
          (tiles_x - x + y <= gtiles_x) ||
          (x + tiles_y - y <= gtiles_x))
        { bad[i] = 1; continue; }
    }
  }

  string fname = opt.bp_file;
  fname = sanitize_path(fname);
  parse_map_include(world, fname);

  world << "<OBJECTS>\n";

  sint4 tile_points = GameConst::TILE_POINTS;  // fixme: should depend on game instance

  sint4 nobjs = opt.nobjs;

  const bool AIR_UNITS = false;
  const bool LAND_UNITS = true;
  
  if (AIR_UNITS && LAND_UNITS) nobjs /= 2;

  // create random clusters of minerals

#if 1
  FORS (i, 7) {
    sint4 x0, y0;
    sint4 t = rand.rand_uint4() % tile_n;
    x0 = t % tiles_x;
    y0 = t / tiles_y;
    if (bad[t] & ~2) { --i; continue; }

    FORS (j, 10) {
      sint4 x = x0 - 2 + rand.rand_uint4() % 5;
      sint4 y = y0 - 2 + rand.rand_uint4() % 5;
      if (x < 1 || x >= tiles_x-1) continue;
      if (y < 1 || y >= tiles_y-1) continue;
      bool can_use = true;
      FORU (u, 3) {
        FORU (v, 3) {
          if (bad[(x-1+u) + tiles_x*(y-1+v)] & ~2) can_use = false;
        }
      }

      if (!can_use) continue;

      FORU (u, 3) {
        FORU (v, 3) {
          bad[(x-1+u) + tiles_x*(y-1+v)] = 2;
        }
      }

      world << "mineral" << endl;
      world << "<INT_ATTR>" << endl;
      world << "owner " << num_players << endl;
      world << "x " << x * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "y " << y * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "</INT_ATTR>" << endl;
    }
  }
#endif

  // create random geysers
#if 1
  FORS (i, 10) {
    sint4 x0, y0;
    sint4 t = rand.rand_uint4() % tile_n;
    x0 = t % tiles_x;
    y0 = t / tiles_y;
    if (bad[t] & ~4) continue;

    FORS (j, 10) {
      sint4 x = x0 - 2 + rand.rand_uint4() % 5;
      sint4 y = y0 - 2 + rand.rand_uint4() % 5;
      if (x < 1 || x >= tiles_x-1) continue;
      if (y < 1 || y >= tiles_y-1) continue;
      bool can_use = true;
      FORU (u, 3) {
        FORU (v, 3) {
          if (bad[(x-1+u) + tiles_x*(y-1+v)]) can_use = false;
        }
      }
      if (!can_use) continue;
      FORU (u, 3) {
        FORU (v, 3) {
          bad[(x-1+u) + tiles_x*(y-1+v)] = 4;
        }
      }
      world << "geyser" << endl;
      world << "<INT_ATTR>" << endl;
      world << "owner " << num_players << endl;
      world << "x " << x * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "y " << y * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "</INT_ATTR>" << endl;
    }
  }
#endif

  FORU (p, num_players) {

    if (AIR_UNITS) {

      FORS (i, nobjs) {
        sint4 r;
        do { r = rand.rand_uint4() % tile_n; } while (bad[r]);
	bad[r] = 1;

        place_unit(world, "destroyer", p, r % tiles_x, r / tiles_x);
      }
    }

    if (LAND_UNITS) { 

      FORS (i, nobjs) {
        sint4 r;
        do { r = rand.rand_uint4() % tile_n; } while (bad[r]);
	bad[r] = 1;

        place_unit(world, "marine", p, r % tiles_x, r / tiles_x);
      }
    }
  }

  // create random clusters of trees
#if 1
  sint4 tree1 = opt.tree1;
  sint4 tree2 = opt.tree2;

  FORS (i, tree1) {
    sint4 x0,y0;
    sint4 t = rand.rand_uint4() % tile_n;
    x0 = t % tiles_x;
    y0 = t / tiles_y;
    if (bad[t] & ~(1 | 8)) { --i; continue; }

    FORS (j, tree2) {
      sint4 x = x0 - 3 + rand.rand_uint4() % 7;
      sint4 y = y0 - 3 + rand.rand_uint4() % 7;
      if (x < 1 || x >= tiles_x-1) continue;
      if (y < 1 || y >= tiles_y-1) continue;
      bool can_use = true;

      FORU (u, 3) {
        FORU (v, 3) {
          if (bad[(x-1+u) + tiles_x*(y-1+v)] & ~(1 | 8)) can_use = false;
        }
      }
      if (!can_use) continue;

      FORU (u, 3) {
        FORU (v, 3) {
          bad[(x-1+u) + tiles_x*(y-1+v)] = 8;
        }
      }
      world << "tree" << endl;
      world << "<INT_ATTR>" << endl;
      world << "owner " << num_players << endl;
      world << "x " << x * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "y " << y * tile_points - 6 + rand.rand_uint4() % 13 << endl;
      world << "</INT_ATTR>" << endl;
    }
  }
#endif

  world << "</OBJECTS>\n";
}

// dump the soar map function here
#include "SoarMap.function"
