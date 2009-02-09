#ifndef PLAYERNAV_GUI_H
#define PLAYERNAV_GUI_H

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgnomecanvas/libgnomecanvas.h>

#include <libplayerc/playerc.h>

#define MIN_DISPLAY_WIDTH 10

#define MAX_HOSTNAME_LEN 256

#define DATA_FREQ 2
#define MAX_NUM_ROBOTS 32

#define ROBOT_RADIUS 0.3
#define ROBOT_V_ANGLE DTOR(30.0)

#define COLOR_BLACK GNOME_CANVAS_COLOR_A(0,0,0,255)

typedef struct
{
  double px,py,pa;
} pose_t;

typedef struct
{
  GtkWindow* main_window;
  GtkBox* vbox;
  GtkBox* hbox;
  GtkScrolledWindow* map_window;
  GnomeCanvas* map_canvas;
  // proxy for map
  playerc_map_t* mapdev;
  // scrollbar for zooming
  GtkVScrollbar* zoom_scrollbar;
  GtkAdjustment* zoom_adjustment;

  size_t num_robots;
  char* hostnames[MAX_NUM_ROBOTS];
  int ports[MAX_NUM_ROBOTS];
  double initial_zoom;
  int aa;
  pose_t robot_poses[MAX_NUM_ROBOTS];
  GnomeCanvasItem* imageitem;
  GnomeCanvasItem* robot_items[MAX_NUM_ROBOTS];
  GnomeCanvasItem* robot_labels[MAX_NUM_ROBOTS];
  GnomeCanvasItem* robot_goals[MAX_NUM_ROBOTS];
  GnomeCanvasItem* robot_paths[MAX_NUM_ROBOTS];
  GnomeCanvasItem* robot_particles[MAX_NUM_ROBOTS];
  playerc_mclient_t* mclient;
  playerc_client_t* clients[MAX_NUM_ROBOTS];
  playerc_map_t* maps[MAX_NUM_ROBOTS];
  playerc_localize_t* localizes[MAX_NUM_ROBOTS];
  playerc_planner_t* planners[MAX_NUM_ROBOTS];
  double goals[MAX_NUM_ROBOTS][3];
  int robot_enable_states[MAX_NUM_ROBOTS];
} gui_data_t;

void create_map_image(gui_data_t* gui_data);
void init_gui(gui_data_t* gui_data, 
              int argc, char** argv);
void update_map(gui_data_t* gui_data);
void fini_gui(gui_data_t* gui_data);
void create_robot(gui_data_t* gui_data, int idx, pose_t pose);
void move_item(GnomeCanvasItem* item, pose_t pose, int raise);
void canvas_to_meters(gui_data_t* gui_data, double* dx, double* dy, 
                      int cx, int cy);
void meters_to_canvas(gui_data_t* gui_data, int* cx, int* cy, 
                      double dx, double dy);
void item_to_meters(GnomeCanvasItem* item,
                    double* dx, double* dy, 
                    double ix, double iy);
void draw_waypoints(gui_data_t* gui_data, int idx);
void draw_goal(gui_data_t* gui_data, int idx);
void draw_particles(gui_data_t* gui_data, int idx);
void dump_screenshot(gui_data_t* gui_data);

playerc_mclient_t* init_player(playerc_client_t** clients,
                               playerc_map_t** maps,
                               playerc_localize_t** localizes,
                               playerc_planner_t** planners,
                               int num_bots,
                               char** hostnames,
                               int* ports,
                               int data_freq,
                               int map_idx,
                               int planner_idx);
void fini_player(playerc_mclient_t* mclient,
                 playerc_client_t** clients,
                 playerc_map_t** maps,
                 playerc_localize_t** localizes,
                 playerc_planner_t** planners,
                 int num_bots);

/* Parse command line arguments, of the form host:port */
int parse_args(int argc, char** argv,
               size_t* num_bots, char** hostnames, int* ports, double*
               zoom, int* aa, int* map_idx, int* planner_idx);

#endif
