#ifndef MapTool_H
#define MapTool_H

/** @file MapTool.H
    Builds game maps.

    $Id: MapTool.H,v 1.30 2006/06/04 07:19:29 dtom Exp $
    $Source: /usr/bodo1/cvs/cvs/orts3/libs/serverclient/src/MapTool.H,v $
*/

// This is an ORTS file
// (c) Michael Chung
// (c) Tim Furtak
// licensed under the GPL

#include "Global.H"
#include "Tile.H"
#include "Map.H"
#include "Options.H"
#include <math.h>  
#include <stack>
#include "Random.H"

// fixme: should be part of a class

const uint4 WATER = 1;
const uint4 GROUND = 2;
const uint4 CLIFF = 3;

class Options;

struct Point
{
  sint4 x;
  sint4 y;
};


// simplifies passing of map data

struct MapData
{
  sint4 tiles_x, tiles_y, tile_n, num_players, gtiles_x, gtiles_y, z_sight;
  Random rand;
  Vector<sint2> bad; // true <=> occupied (tile)
};


class MapTool
{
public:

  struct Options {

    std::string bp_file;
    std::string hfile;

    sint4 seed;
    sint4 nobjs;
    sint4 nplayers;
    sint4 width;
    sint4 height;
    real8 fplat;
    sint4 clump;
    bool smooth;
    bool holes;
    bool water;
    bool rot;
    sint4 tree1;
    sint4 tree2;

    sint4 island;
    bool cliff;
    real8 convex;

    bool split;
    bool symmetric;
    sint4 n_tanks;
    sint4 n_marines;

    static void add();
    void get();
  };
  static Options opt;

  static void parse_map_include(std::stringstream &world, std::string &fname);
  static void place_unit(std::stringstream &world, const std::string &unit_name, 
                         sint4 player, sint4 x, sint4 y,
                         const std::string &extra = "");
  
  static void read_height_map(std::stringstream &world);

  static void generate_random_map(std::stringstream &world);

  static void generate_random_cliff_map(std::stringstream &world);
  static void generate_soar_map(std::stringstream &world, std::string map_settings_file);
  static void generate_ctf_map(std::stringstream &world, bool test_mode);
  static void generate_maze_map(std::stringstream &world);
  static void generate_debug_map(std::stringstream &world);
  static void generate_island_map(std::stringstream &world);
  static void generate_frano_map(std::stringstream &world);
  static void generate_marine_map(std::stringstream &world);
  //---------------------------here----------------------------//
  static void generate_simple_map(std::stringstream &world);

  static void init_map_data(MapData &md);
  static void generate_game_tiles(MapData &md, std::stringstream& world, bool bad_init);
  static void generate_game1_map(std::stringstream &world);
  static void generate_soarbp_map(std::stringstream &world);
  static void generate_game2_map(std::stringstream &world);
  static void generate_game3_map(std::stringstream &world);
  
  static void write_map_file(const std::stringstream &world, const std::string &out_fname);
  
  // inscribes players on a disk marked by the specified radius
  static void place_players(sint4 rad_i, sint4 *x_pos, sint4 *y_pos);
  
  //Grows islands or plateaus
  static void add_tile(sint4 seed, Vector<Point> &edges, Map<Tile> &map,
                       bool *flags, sint4 x_tiles, sint4 y_tiles, sint4 diff);
  static void raise_island(Map<Tile>& map_data, sint4 level_2, sint4 num_islands, sint4 size, sint4 seed);
  
  /**
   * raises a single island of the given size at the given position using the given seed.
   * */
  static void raise_island(Map<Tile>& map_data, sint4 level, Point pos, sint4 size, sint4 seed);

  typedef bool (RampFunc)(sint4 x, sint4 y, Map<Tile>& map_data, sint4 left_edge, sint4 lower_edge, sint4 right_edge,
                          sint4 level_1, sint4 level_2, sint4 width, sint4 height);

  static RampFunc n_ramp;
  static RampFunc e_ramp;
  static RampFunc s_ramp;
  static RampFunc w_ramp;
  static RampFunc ne_ramp;
  static RampFunc nw_ramp;
  static RampFunc se_ramp;
  static RampFunc sw_ramp;

  static bool place_ramp(Map<Tile>& map_data, sint4 x, sint4 y, sint4 box_width, sint4 dir, sint4 level_1, sint4 level_2, sint4 width, sint4 height);
  static void cliff_tiles(Map<Tile>& map_data, sint4 width, sint4 height);
  static void sync_corner(Map<Tile>& map_data, sint4 width, sint4 height);
  static bool inside_polygon(std::vector<Point>& polygon,Point p);
  static void lower_edge(std::stack<Point> all, std::stack<Point>& polygon, real8 min_angle);
  static void upper_edge(std::stack<Point> all, std::stack<Point>& polygon, real8 min_angle);
  static void smooth_edge(sint4 x, sint4 y, Map<Tile> &map_data, sint4 diff);
  static Point place_object(sint4 x, sint4 y, sint4 min_dist, sint4 max_dist, Random r);
  static void add_options();
 };

#endif
