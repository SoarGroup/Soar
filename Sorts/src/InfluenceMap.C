// $Id: InfluenceMap.C,v 1.2 2007/05/23 23:32:18 orts_furtak Exp $
// This is an ORTS file (c) Tim Furtak licensed under the GPL

#include "Global.H"
#include "InfluenceMap.H"
#include "DrawOnTerrain.H"
#include "Game.H"
#include "Profiler.H"
#include "GameStateModule.H"
#include "GameTile.H"
#include "GfxModule.H"

using namespace std;

enum {MAP_WALKABLE = 0, 
      MAP_UNWALKABLE = 99999999, 
      MAP_BASE_UNWALKABLE = 9999,
      MAP_DECAY = 20,
      MAP_RANGE = 3, 

      MAP_MYBASE = 1, 
      MAP_ENEMYBASE = -1, 
      MAP_BASE_DECAY = 1,
      MAP_BASE_RADIUS = 100000,

      MAP_MYWORKER = 2, 
      MAP_ENEMYWORKER = -2, 

      MAP_MYMARINE = 3, 
      MAP_ENEMYMARINE = -3, 

      MAP_MYTANK = 4, 
      MAP_ENEMYTANK = -4};

enum {WALK_DECAY = 20, 
      WALK_RANGE = 50, 
      WALK_VALUE = -4};


/********************************************************************
 * InfluenceMap::InfluenceMap
 ********************************************************************/ 
InfluenceMap::InfluenceMap(sint4 px, sint4 py, sint4 cell_size) {
  init(px, py, cell_size);
}

InfluenceMap::InfluenceMap(){}

/********************************************************************
 * InfluenceMap::init
 ********************************************************************/
void InfluenceMap::init(sint4 px, sint4 py, sint4 cs) {
  assert(px >= 0 && py >= 0);
  assert(cs >= 1);

  points_x = px;
  points_y = py;
  cell_size = cs;

  cells_x = (px + cell_size-1) / cell_size;
  cells_y = (py + cell_size-1) / cell_size;
  cell_num = cells_x * cells_y;
  
  data.resize(cell_num);
  clear_all();
}

/********************************************************************
 * InfluenceMap::init_game
 ********************************************************************/
void InfluenceMap::init_game(const Game *game, sint4 cid){
  assert (game != NULL);
  assert(cid >= 0);

  const Map<GameTile> &map = game->get_map();

  // Topography
  FORS(x, cells_x) {
    FORS(y, cells_y){
      const GameTile &tile = map(x, y);
      sint4 hNW, hNE, hSE, hSW;
      Tile::Split split;
      
      // Get the topography of current tile
      tile.get_topo(hNW, hNE, hSE, hSW, split);
      
      // Define tiles at height 0 to be walkable
      if ( hNW == 0 && hNE == 0 && hSE == 0 && hSW == 0 ) 
	get_cell(x, y) = MAP_WALKABLE; 
      else // (height > 0)
	get_cell(x, y) = MAP_UNWALKABLE;
    }
  }
  
  refresh_units(game, cid);
}

/********************************************************************
 * InfluenceMap::init_map
 ********************************************************************/
void InfluenceMap::init_map(const Game *game){
  assert (game != NULL);

  const Map<GameTile> &map = game->get_map();

  // Topography
  FORS(x, cells_x) {
    FORS(y, cells_y){
      const GameTile &tile = map(x, y);
      sint4 hNW, hNE, hSE, hSW;
      Tile::Split split;
      
      // Get the topography of current tile
      tile.get_topo(hNW, hNE, hSE, hSW, split);
      
      // Define tiles at height 0 to be walkable
      if ( hNW == 0 && hNE == 0 && hSE == 0 && hSW == 0 ) 
	continue; // do nothing for these 
      else // (height > 0)
	add_gaussian(x * cell_size, y * cell_size, WALK_DECAY, WALK_RANGE, WALK_VALUE);
    }
  }
}


/********************************************************************
 * InfluenceMap::refresh_units ** control centers are unwalkable
 ********************************************************************/
void InfluenceMap::refresh_units(const Game *game, sint4 cid) {
  assert(game != NULL);
  assert(cid >= 0);
  
  // Clear current units
  clear_units();
  
  // Add new units
  FORS(id, game->get_player_num()){    
    FORALL (game->get_objs(id), it) {
      GameObj *gob = (*it)->get_GameObj();
      ScriptObj *weapon = gob->component("weapon");
      sintptr *prange = 0;
      
      // Only get the range for units who have a weapon
      if (weapon){
	prange = weapon->get_int_ptr("max_ground_range");
      }

      // Our Units
      if (id == cid){
	// WORKER
	if (gob->bp_name() == "worker"){
	  add_gaussian(*gob->sod.x, *gob->sod.y, MAP_DECAY, *prange*MAP_RANGE, MAP_MYWORKER);
	}
	// MARINE
	else if (gob->bp_name() == "marine"){
	  add_gaussian(*gob->sod.x, *gob->sod.y, MAP_DECAY, *prange*MAP_RANGE, MAP_MYMARINE);	  
	}
	// TANK
	else if (gob->bp_name() == "tank"){
	  add_gaussian(*gob->sod.x, *gob->sod.y, MAP_DECAY, *prange*MAP_RANGE, MAP_MYTANK);
	}
	// CONTROL CENTER
	else if (gob->bp_name() == "controlCenter" 
		 && *gob->sod.x1 > 0 
		 && gob->sod.in_game) {
	  add_gaussian(*gob->sod.x, *gob->sod.y, MAP_BASE_DECAY, MAP_BASE_RADIUS, MAP_MYBASE);
	  make_unwalkable(*gob->sod.x1, *gob->sod.x2, *gob->sod.y1, *gob->sod.y2);
	}
      }
      
      // Enemy Units
      else {
	// WORKER
	if (gob->bp_name() == "worker"){
	  add_gaussian(*gob->sod.x, *gob->sod.y, MAP_DECAY, *prange*MAP_RANGE, MAP_ENEMYWORKER);
	}
	// MARINE
	else if (gob->bp_name() == "marine"){
	  add_gaussian(*gob->sod.x, *gob->sod.y, MAP_DECAY, *prange*MAP_RANGE, MAP_ENEMYMARINE);	  
	}
	// TANK
	else if (gob->bp_name() == "tank"){
	  add_gaussian(*gob->sod.x, *gob->sod.y, MAP_DECAY, *prange*MAP_RANGE, MAP_ENEMYTANK);
	}
	// CONTROL CENTER -- Unwalkable 
	else if (gob->bp_name() == "controlCenter" 
		 && *gob->sod.x1 > 0 
		 && gob->sod.in_game) {
	  add_gaussian(*gob->sod.x, *gob->sod.y, MAP_BASE_DECAY, MAP_BASE_RADIUS, MAP_ENEMYBASE);
	  make_unwalkable(*gob->sod.x1, *gob->sod.x2, *gob->sod.y1, *gob->sod.y2);
	}
      }
    }
  }
}


/********************************************************************
 * InfluenceMap::add_gaussian - Drop a gaussian centered on (cx, cy) 
 * with decay s, and max radius r.
 ********************************************************************/
void InfluenceMap::add_gaussian(real4 cx, real4 cy, real4 s, real4 r, real4 val) {
  sint4 min_x = ::max(sint4(floor(cx - r)), 0);
  sint4 max_x = ::min(sint4( ceil(cx + r)), points_x-1);
  sint4 min_y = ::max(sint4(floor(cy - r)), 0);
  sint4 max_y = ::min(sint4( ceil(cy + r)), points_y-1);

  min_x /= cell_size;
  max_x /= cell_size;
  min_y /= cell_size;
  max_y /= cell_size;

  cx /= cell_size;
  cy /= cell_size;
  r /= cell_size;

  real4 s2 = s*s;
  const real4 r2pi = 2.506628274;

  for (sint4 y = min_y; y <= max_y; ++y) {
    real4 ty = y+0.5;
    for (sint4 x = min_x; x <= max_x; ++x) {
      real4 tx = x+0.5;
      real4 d2 = square(cx - tx) + square(cy - ty);
      real4 d = sqrt(d2);

      real4 v = val / (s*r2pi) * expf(-d2 / (2*s2));

      d -= r;
      if (d <= 0.0) {
        get_cell(x,y) += v;
      } else if (d <= 1.0) {
        get_cell(x,y) += v * (1.0-d);
      }
    }
  }
}
/********************************************************************
 * InfluenceMap::add_circle - add a circle centered on cx, cy, with 
 * radius r and value val
 ********************************************************************/
void InfluenceMap::add_circle(real4 cx, real4 cy, real4 r, real4 val) {
  sint4 min_x = ::max(sint4(floor(cx - r)), 0);
  sint4 max_x = ::min(sint4( ceil(cx + r)), points_x-1);
  sint4 min_y = ::max(sint4(floor(cy - r)), 0);
  sint4 max_y = ::min(sint4( ceil(cy + r)), points_y-1);

  min_x /= cell_size;
  max_x /= cell_size;
  min_y /= cell_size;
  max_y /= cell_size;

  cx /= cell_size;
  cy /= cell_size;
  r /= cell_size;

  real4 r2 = r*r;

  for (sint4 y = min_y; y <= max_y; ++y) {
    for (sint4 x = min_x; x <= max_x; ++x) {
      real4 d2 = square(cx - x) + square(cy - y);
      if (d2 <= r2) get_cell(x,y) += val; //data[xy2ind(x, y)] += val;
    }
  }
}


/********************************************************************
 * InfluenceMap::add_aacircle - add a circle centered on cx, cy, with 
 * radius r and value val
 ********************************************************************/
void InfluenceMap::add_aacircle(real4 cx, real4 cy, real4 r, real4 val) {
  sint4 min_x = ::max(sint4(floor(cx - r)), 0);
  sint4 max_x = ::min(sint4( ceil(cx + r)), points_x-1);
  sint4 min_y = ::max(sint4(floor(cy - r)), 0);
  sint4 max_y = ::min(sint4( ceil(cy + r)), points_y-1);

  min_x /= cell_size;
  max_x /= cell_size;
  min_y /= cell_size;
  max_y /= cell_size;

  cx /= cell_size;
  cy /= cell_size;
  r /= cell_size;

  for (sint4 y = min_y; y <= max_y; ++y) {
    real4 ty = (y+0.5);
    for (sint4 x = min_x; x <= max_x; ++x) {
      real4 tx = (x+0.5);
      real4 d = sqrt(square(cx - tx) + square(cy - ty));
      d -= r;
      //      real4 d = d2 - r2;
      if (d <= 0.0) {
        get_cell(x,y) += val;
      } else if (d <= 1.0) {
        get_cell(x,y) += val * (1.0-d);
      }
    }
  }
}

/********************************************************************
 * InfluenceMap::clear_all
 ********************************************************************/
void InfluenceMap::clear_all() {
  FORU (i, cell_num) data[i] = 0.0;
}

/********************************************************************
 * InfluenceMap::clear_units
 ********************************************************************/
void InfluenceMap::clear_units() {
  FORU (i, cell_num) {
    if (data[i] < MAP_UNWALKABLE/2) data[i] = 0.0;
  }
}

/********************************************************************
 * InfluenceMap::make_unwalkable
 ********************************************************************/
void InfluenceMap::make_unwalkable(sint4 px, sint4 py){
  get_cell(px / cell_size, py / cell_size) = MAP_BASE_UNWALKABLE;
}

void InfluenceMap::make_unwalkable(sint4 px1, sint4 px2, sint4 py1, sint4 py2){
  for (sint4 i = (px1 + cell_size) / cell_size; i < px2 / cell_size; i++)
    for (sint4 j = (py1 + cell_size) / cell_size; j < py2 / cell_size; j++)
      get_cell(i, j) = MAP_BASE_UNWALKABLE;
}


/********************************************************************
 * InfluenceMap::invert 
 ********************************************************************/
void InfluenceMap::invert() {
  FORU (i, cell_num) if (data[i] != 0.0) data[i] = 1.0 / data[i];
}

/********************************************************************
 * InfluenceMap::get_walkable
 ********************************************************************/
sint4 InfluenceMap::get_walkable(){
  return MAP_BASE_UNWALKABLE/2;
}

/********************************************************************
 * InfluenceMap::get_highest
 ********************************************************************/
ScalarPoint InfluenceMap::get_highest(){
  ScalarPoint loc;
  real4 high;
  loc.x = loc.y = 0;
  high = -9999;
  
  FORS (y, cells_y){
    FORS (x, cells_x){
      if (get_cell(x, y) > high && 
	  get_cell(x, y) < MAP_BASE_UNWALKABLE/2){
	high = get_cell(x,y);
	loc.x = x * cell_size;
	loc.y = y * cell_size;
      }
    }
  }  
  
  return loc;
}

/********************************************************************
 * InfluenceMap::print
 ********************************************************************/
void InfluenceMap::print() {
  cout << "%%%%%%%%%%% Print %%%%%%%%%%%%" << endl;

  /*
    FORS (y, cells_y){
    FORS(x, cells_x){
    if      (get_cell(x,y) > MAP_UNWALKABLE)
    cout << "X";
    else if (get_cell(x,y) > MAP_UNWALKABLE/2)
    cout << "E";
    else if (get_cell(x,y) > 20)
    cout << "5";
    else if (get_cell(x,y) > 15)
    cout << "4";
    else if (get_cell(x,y) > 10)
    cout << "3";
    else if (get_cell(x,y) > 5.0)
    cout << "2";
    else if (get_cell(x,y) > 1.0)
    cout << "1";
    else if (get_cell(x,y) > 0.0)
    cout << "0";
    else 
    cout << " ";
    }
    cout << endl;
    }
  */

  FORS(y, cells_y){
    FORS(x, cells_x){
      cout << "[" << x << ", " << y << "] :: " << get_cell(x,y) << endl;
    }
  }  
}


/********************************************************************
 * InfluenceMap::draw
 ********************************************************************/
void InfluenceMap::draw(DrawOnTerrain2D *dot) {
  sint4 half_cell = cell_size/2;
  assert(dot != NULL);
  
  // Draw Things
  FORS(y, cells_y){
    FORS(x, cells_x){
      if ((get_cell(x,y) < .25 && get_cell(x,y) > -.25) || get_cell(x,y) > MAP_BASE_UNWALKABLE/2){
	continue;
      }
      // Draw us in Blue - Positive Influence
      else if (get_cell(x,y) > 0) {
	Vec3<real4> color = Vec3<real4>(0.0, 0.5, 1.0);
	color *= get_cell(x, y);
	dot->draw_circle(x * cell_size + half_cell, 
			y * cell_size + half_cell, 
			8,  color);
      }
      // Draw enemy in Red - Negative Influence
      else {
	Vec3<real4> color = Vec3<real4>(1.0, .5, 0.0);
	color *= -get_cell(x, y);
	dot->draw_circle(x * cell_size + half_cell, 
			y * cell_size + half_cell,
			8, color);
      }
    }
  }
}
