/*
 *  RTK2 : A GUI toolkit for robotics
 *  Copyright (C) 2001  Andrew Howard  ahoward@usc.edu
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * Desc: Combined Rtk functions
 * Author: Andrew Howard
 * Contributors: Richard Vaughan
 * CVS: $Id: rtk.h 2686 2005-02-03 23:44:17Z gerkey $
 */

#ifndef RTK_H
#define RTK_H

#include <stdio.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTK_CANVAS_LAYERS 100

// Movement mask for canvases
#define RTK_MOVE_PAN   (1 << 0)
#define RTK_MOVE_ZOOM  (1 << 1)

// Movement masks for figures
#define RTK_MOVE_TRANS (1 << 0)
#define RTK_MOVE_ROT   (1 << 1)
#define RTK_MOVE_SCALE (1 << 2)

// Event codes for figures
#define RTK_EVENT_PRESS     1
#define RTK_EVENT_MOTION    2
#define RTK_EVENT_RELEASE   3
#define RTK_EVENT_MOUSE_OVER 4
#define RTK_EVENT_MOUSE_NOT_OVER 5

// Export image formats
#define RTK_IMAGE_FORMAT_JPEG 0
#define RTK_IMAGE_FORMAT_PPM  1  

// Color space conversion macros
#define RTK_RGB16(r, g, b) (((b) >> 3) | (((g) & 0xFC) << 3) | (((r) & 0xF8) << 8))
#define RTK_R_RGB16(x) (((x) >> 8) & 0xF8)
#define RTK_G_RGB16(x) (((x) >> 3) & 0xFC)
#define RTK_B_RGB16(x) (((x) << 3) & 0xF8)

  
// Useful forward declarations
struct _rtk_canvas_t;
struct _rtk_table_t;
struct _rtk_fig_t;
struct _rtk_stroke_t;
struct _rtk_region_t;
struct _rtk_menuitem_t;

struct _rtk_flasher_t; // rtv experimental
  
struct AVCodec;
struct AVCodecContext;


/***************************************************************************
 * Library functions
 ***************************************************************************/

// Initialise the library.
// Pass in the program arguments through argc, argv
int rtk_init(int *argc, char ***argv);

  
/***************************************************************************
 * Application functions
 ***************************************************************************/

// Structure describing and application
typedef struct _rtk_app_t
{
  int must_quit;
  int has_quit;

  // Linked list of canvases
  struct _rtk_canvas_t *canvas;

  // Linked list of tables
  struct _rtk_table_t *table;
  
} rtk_app_t;
  
// Create the application
rtk_app_t *rtk_app_create(void);

// Destroy the application
void rtk_app_destroy(rtk_app_t *app);
  
// Main loop for the app (blocking).  Will return only when the gui is closed by
// the user.
int rtk_app_main(rtk_app_t *app);

// Do the initial main loop stuff
void rtk_app_main_init(rtk_app_t *app);

// Do the final main loop stuff
void rtk_app_main_term(rtk_app_t *app);

// Process pending events (non-blocking). Returns non-zero if the app
// should quit.
int rtk_app_main_loop(rtk_app_t *app);


/***************************************************************************
 * Canvas functions
 ***************************************************************************/
  
// Structure describing a canvas
typedef struct _rtk_canvas_t
{
  // Linked list of canvases
  struct _rtk_canvas_t *next, *prev;

  // Link to our parent app
  struct _rtk_app_t *app;

  // Gtk stuff
  GtkWidget *frame;
  GtkWidget *layout;
  GtkWidget *canvas;

  // GDK stuff
  GdkPixmap *bg_pixmap, *fg_pixmap;
  GdkGC *gc;
  GdkColormap *colormap;
  GdkFont *font;

  // Default font name
  char *fontname;

  // Canvas background color
  GdkColor bgcolor;
  
  // Default line width
  int linewidth;

  // The menu bar widget
  GtkWidget *menu_bar;

  // File in which to render xfig figures.
  FILE *file;

  // Physical size of window
  int sizex, sizey;
    
  // Coordinate transform
  // Logical coords of middle of canvas
  // and logical/device scale (eg m/pixel)
  double ox, oy;
  double sx, sy;

  // Flag set if canvas has been destroyed
  int destroyed;
  
  // Flags controlling background re-rendering
  int bg_dirty;

  // Flags controlling foreground re-rendering
  int fg_dirty;
  struct _rtk_region_t *fg_dirty_region;

  // Non-zero if there are deferred calculations to be done.
  int calc_deferred;

  // Movement mask for the canvas
  int movemask;

  // Movie capture stuff
  FILE *movie_file;
  double movie_fps, movie_speed;
  int movie_frame;
  double movie_time, mpeg_time;
  unsigned char movie_lut[0x10000][3];
  struct AVCodec *movie_codec;
  struct AVCodecContext *movie_context;

  // Head of linked-list of top-level (parentless) figures.
  struct _rtk_fig_t *fig;

  // Head of linked-list of figures ordered by layer.
  struct _rtk_fig_t *layer_fig;

  // Head of linked-list of moveble figures.
  struct _rtk_fig_t *moveable_fig;
  
  // Mouse stuff
  int mouse_mode;
  double mouse_start_x, mouse_start_y, mouse_start_a;
  struct _rtk_fig_t *zoom_fig;
  struct _rtk_fig_t *mouse_over_fig;
  struct _rtk_fig_t *mouse_selected_fig;
  
  // linked list of figs that are shown for a short period, then
  // hidden or destroyed - rtv experimental
  struct _rtk_flasher_t *flashers;
  
  // toggle these bytes to individually show and hide layers - rtv
  char layer_show[RTK_CANVAS_LAYERS];

  // arbitrary user data
  void *userdata;

} rtk_canvas_t;


// Create a canvas on which to draw stuff.
rtk_canvas_t *rtk_canvas_create(rtk_app_t *app);

// Destroy the canvas
void rtk_canvas_destroy(rtk_canvas_t *canvas);

// Lock the canvas i.e. get exclusive access.
void rtk_canvas_lock(rtk_canvas_t *canvas);

// Unlock the canvas
void rtk_canvas_unlock(rtk_canvas_t *canvas);

// See if the canvas has been closed
int rtk_canvas_isclosed(rtk_canvas_t *canvas);

// Set the canvas title
void rtk_canvas_title(rtk_canvas_t *canvas, const char *title);

// Set the size of a canvas
// (sizex, sizey) is the width and height of the canvas, in pixels.
void rtk_canvas_size(rtk_canvas_t *canvas, int sizex, int sizey);

// Get the canvas size
// (sizex, sizey) is the width and height of the canvas, in pixels.
void rtk_canvas_get_size(rtk_canvas_t *canvas, int *sizex, int *sizey);

// Set the origin of a canvas
// (ox, oy) specifies the logical point that maps to the center of the
// canvas.
void rtk_canvas_origin(rtk_canvas_t *canvas, double ox, double oy);

// Get the origin of a canvas
// (ox, oy) specifies the logical point that maps to the center of the
// canvas.
void rtk_canvas_get_origin(rtk_canvas_t *canvas, double *ox, double *oy);

// Scale a canvas
// Sets the pixel width and height in logical units
void rtk_canvas_scale(rtk_canvas_t *canvas, double sx, double sy);

// Get the scale of the canvas
// (sx, sy) are the pixel with and height in logical units
void rtk_canvas_get_scale(rtk_canvas_t *canvas, double *sx, double *sy);

// Set the movement mask
// Set the mask to a bitwise combination of RTK_MOVE_TRANS, RTK_MOVE_SCALE.
// to enable user manipulation of the canvas.
void rtk_canvas_movemask(rtk_canvas_t *canvas, int mask);

// Set the default font for text strokes
void rtk_canvas_font(rtk_canvas_t *canvas, const char *fontname);

// Set the canvas backround color
void rtk_canvas_bgcolor(rtk_canvas_t *canvas, double r, double g, double b);

// Set the default line width.
void rtk_canvas_linewidth(rtk_canvas_t *canvas, int width);

// Re-render the canvas
void rtk_canvas_render(rtk_canvas_t *canvas);

// Export an image.
// [filename] is the name of the file to save.
// [format] is the image file format (RTK_IMAGE_FORMAT_JPEG, RTK_IMAGE_FORMAT_PPM).
void rtk_canvas_export_image(rtk_canvas_t *canvas, const char *filename, int format);

/*
// Export canvas to xfig file
int rtk_canvas_export_xfig(rtk_canvas_t *canvas, char *filename);
*/

// Start movie capture.
// [filename] is the name of the file to save.
// [fps] is the rate at which the caller will write frames (e.g. 5fps, 10fps).
// [speed] is the ultimate playback speed of the movie (e.g. 1x, 2x).
int rtk_canvas_movie_start(rtk_canvas_t *canvas,
                           const char *filename, double fps, double speed);

// Start movie capture.
void rtk_canvas_movie_frame(rtk_canvas_t *canvas);

// Stop movie capture.
void rtk_canvas_movie_stop(rtk_canvas_t *canvas);

// rtv experimental
// Maintain a list of figures that are shown for a set time, then hidden.
typedef struct _rtk_flasher_t
{
  struct _rtk_fig_t* fig; // a figure to be shown for a set period
  int duration; // decremented on each call to rtk_canvas_flash_update()
  int kill; // if zero, the fig is hidden on timeout, if non-zero, the
  // figure is destroyed instead

  // list hooks
  struct _rtk_flasher_t* next, *prev;
} rtk_flasher_t;

// Add this figure to the list of flashers. It will be shown until
// rtk_canvas_flash_update() is called [duration] times, then it will
// be destroyed if [kill] is non-zero, or hidden, if [kill] is zero
void rtk_canvas_flash( rtk_canvas_t* canvas, 
		       struct _rtk_fig_t* fig, int duration, int kill );

// Decrement the flasher's counters. Those that time out get hidden
// and removed from the flasher list
void rtk_canvas_flash_update( rtk_canvas_t* canvas );

// show and hide individual layers
void rtk_canvas_layer_show( rtk_canvas_t* canvas, int layer, char show );

// end rtv experimental

/***************************************************************************
 * Figure functions
 ***************************************************************************/
  
// Structure describing a color
typedef GdkColor rtk_color_t;

// Callback function signatures
typedef void (*rtk_mouse_fn_t) (struct _rtk_fig_t *fig, int event, int mode);


// Structure describing a figure
typedef struct _rtk_fig_t
{
  // Pointer to parent canvas
  struct _rtk_canvas_t *canvas;

  // Pointer to our parent figure
  struct _rtk_fig_t *parent;

  // Head of a linked-list of children.
  struct _rtk_fig_t *child;
  
  // Linked-list of siblings
  struct _rtk_fig_t *sibling_next, *sibling_prev;

  // Linked-list of figures ordered by layer  
  struct _rtk_fig_t *layer_next, *layer_prev;

  // Linked-list of figures that are moveable.
  struct _rtk_fig_t *moveable_next, *moveable_prev;

  // Arbitrary user data
  void *userdata;
  
  // Layer this fig belongs to
  int layer;

  // Flag set to true if figure should be displayed
  int show;

  // Movement mask (a bit vector)
  int movemask;
  
  // Origin, scale of figure
  // relative to parent.
  double ox, oy, oa;
  double cos, sin;
  double sx, sy;

  // Origin, scale of figure
  // in global cs.
  double dox, doy, doa;
  double dcos, dsin;
  double dsx, dsy;

  // Bounding box for the figure in local cs; contains all of the
  // strokes for this figure.
  double min_x, min_y, max_x, max_y;

  // Bounding region for the figure in device cs; contains all the
  // strokes for this figure.
  struct _rtk_region_t *region;
  
  // List of strokes
  int stroke_size;
  int stroke_count;
  struct _rtk_stroke_t **strokes;

  // Drawing context information.  Just a list of default args for
  // drawing primitives.
  rtk_color_t dc_color;
  int dc_xfig_color;
  int dc_linewidth;

  // if > 0, the visibility of this figure is toggled with this
  // interval. To stop blinking but remember the interval, make the
  // value negative. (rtv)
  int blink_interval_ms;

  // Event callback functions.
  rtk_mouse_fn_t mouse_fn;
} rtk_fig_t;


// Figure creation/destruction
//rtk_fig_t *rtk_fig_create(rtk_canvas_t *canvas, rtk_fig_t *parent, int layer);
rtk_fig_t *rtk_fig_create(rtk_canvas_t *canvas, rtk_fig_t *parent, int layer );
void rtk_fig_destroy(rtk_fig_t *fig);

// Recursively free a whole tree of figures (rtv)
void rtk_fig_and_descendents_destroy( rtk_fig_t* fig );

// create a figure and set its user data pointer (rtv)
rtk_fig_t *rtk_fig_create_ex(rtk_canvas_t *canvas, rtk_fig_t *parent, 
			     int layer, void* userdata );

// Set the mouse event callback function.
void rtk_fig_add_mouse_handler(rtk_fig_t *fig, rtk_mouse_fn_t callback);

// Unset the mouse event callback function.
void rtk_fig_remove_mouse_handler(rtk_fig_t *fig, rtk_mouse_fn_t callback);

// Figure synchronization.
void rtk_fig_lock(rtk_fig_t *fig);
void rtk_fig_unlock(rtk_fig_t *fig);

// Clear all existing strokes from a figure.
void rtk_fig_clear(rtk_fig_t *fig);

// Show or hide the figure
void rtk_fig_show(rtk_fig_t *fig, int show);

// Set the movement mask
// Set the mask to a bitwise combination of RTK_MOVE_TRANS, RTK_MOVE_ROT, etc,
// to enable user manipulation of the figure.
void rtk_fig_movemask(rtk_fig_t *fig, int mask);

// See if the mouse is over this figure
int rtk_fig_mouse_over(rtk_fig_t *fig);

// See if the figure has been selected
int rtk_fig_mouse_selected(rtk_fig_t *fig);

// Set the figure origin (local coordinates).
void rtk_fig_origin(rtk_fig_t *fig, double ox, double oy, double oa);

// Set the figure origin (global coordinates).
void rtk_fig_origin_global(rtk_fig_t *fig, double ox, double oy, double oa);

// Get the current figure origin (local coordinates).
void rtk_fig_get_origin(rtk_fig_t *fig, double *ox, double *oy, double *oa);

// Set the figure scale
void rtk_fig_scale(rtk_fig_t *fig, double scale);

// Set the color for strokes.  Color is specified as an (r, g, b)
// tuple, with values in range [0, 1].
void rtk_fig_color(rtk_fig_t *fig, double r, double g, double b);

// Set the color for strokes.  Color is specified as an RGB32 value (8
// bits per color).
void rtk_fig_color_rgb32(rtk_fig_t *fig, int color);

// Set the color for strokes.  Color is specified as an xfig color.
void rtk_fig_color_xfig(rtk_fig_t *fig, int color);

// Set the line width.
void rtk_fig_linewidth(rtk_fig_t *fig, int width);

// Draw a single point.
void rtk_fig_point(rtk_fig_t *fig, double ox, double oy);

// Draw a line between two points.
void rtk_fig_line(rtk_fig_t *fig, double ax, double ay, double bx, double by);

// Draw a line centered on the given point.
void rtk_fig_line_ex(rtk_fig_t *fig, double ox, double oy, double oa, double size);

// Draw a rectangle centered on (ox, oy) with orientation oa and size (sx, sy).
void rtk_fig_rectangle(rtk_fig_t *fig, double ox, double oy, double oa,
                       double sx, double sy, int filled);

// Draw an ellipse centered on (ox, oy) with orientation oa and size (sx, sy).
void rtk_fig_ellipse(rtk_fig_t *fig, double ox, double oy, double oa,
                     double sx, double sy, int filled);

// Draw an arc between min_th and max_th on the ellipse as above
void rtk_fig_ellipse_arc( rtk_fig_t *fig, double ox, double oy, double oa,
			  double sx, double sy, double min_th, double max_th);

// Create a polygon
void rtk_fig_polygon(rtk_fig_t *fig, double ox, double oy, double oa,
                     int point_count, double points[][2], int filled);

// Draw an arrow from point (ox, oy) with orientation oa and length len.
void rtk_fig_arrow(rtk_fig_t *fig, double ox, double oy, double oa,
                   double len, double head);

// Draw an arrow between points (ax, ay) and (bx, by).
void rtk_fig_arrow_ex(rtk_fig_t *fig, double ax, double ay,
                      double bx, double by, double head);

// Draw single or multiple lines of text.  Lines are deliminted with
// '\n'.
void rtk_fig_text(rtk_fig_t *fig, double ox, double oy, double oa,
                  const char *text);

// Draw a grid.  Grid is centered on (ox, oy) with size (dx, dy) with
// spacing (sp).
void rtk_fig_grid(rtk_fig_t *fig, double ox, double oy,
                  double dx, double dy, double sp);

// Draw an image.  bpp specifies the number of bits per pixel, and
// must be 16.
void rtk_fig_image(rtk_fig_t *fig, double ox, double oy, double oa,
                   double scale, int width, int height, int bpp, void *image, void *mask);

// start toggling the show flag for the figure at approximately the
// desired millisecond interval - set to < 1 to stop blinking
void rtk_fig_blink( rtk_fig_t* fig, int interval_ms, int flag );


/***************************************************************************
 * Primitive strokes for figures.
 ***************************************************************************/

// Signature for stroke methods.
typedef void (*rtk_stroke_fn_t) (struct _rtk_fig_t *fig, void *stroke);


// Structure describing a stroke (base structure for all strokes).
typedef struct _rtk_stroke_t
{
  // Color
  rtk_color_t color;

  // Xfig color
  int xfig_color;

  // Line width
  int linewidth;

  // Function used to free data associated with stroke
  rtk_stroke_fn_t freefn;

  // Function used to compute onscreen appearance
  rtk_stroke_fn_t calcfn;
  
  // Function used to render stroke
  rtk_stroke_fn_t drawfn;

  // Function used to render stroke to xfig
  rtk_stroke_fn_t xfigfn;
  
} rtk_stroke_t;


// Struct describing a point
typedef struct
{  
  double x, y; 
} rtk_point_t;


// Struct describing a point stroke
typedef struct
{
  rtk_stroke_t stroke;

  // Point in logical local coordinates.
  double ox, oy;

  // Point in absolute physical coordinates.
  GdkPoint point;
  
} rtk_point_stroke_t;


// Polygon/polyline stroke
typedef struct
{
  rtk_stroke_t stroke;

  // Origin of figure in logical local coordinates.
  double ox, oy, oa;

  // Zero if this is a polyline, Non-zero if this is a polygon.
  int closed;
  
  // Non-zero if polygon should be filled.
  int filled;

  // A list of points in the polygon, in both local logical
  // coordinates and absolute physical coordinates.
  int point_count;
  rtk_point_t *lpoints;
  GdkPoint *ppoints;
  
} rtk_polygon_stroke_t;


// Struct describing text stroke
typedef struct
{
  rtk_stroke_t stroke;

  // Origin of the text in logical local coordinates.
  double ox, oy, oa;

  // Origin of the text in absolute physical coordinates.
  GdkPoint point;

  // The text
  char *text;
  
} rtk_text_stroke_t;


// Structure describing an image stroke
typedef struct
{
  rtk_stroke_t stroke;

  // Origin of figure in local logical coordinates.
  double ox, oy, oa;

  // Rectangle containing the image (absolute physical coordinates).
  double points[4][2];

  // Image data
  double scale;
  int width, height, bpp;
  void *image, *mask;

} rtk_image_stroke_t;


/***************************************************************************
 * Region manipulation (used internal for efficient redrawing).
 ***************************************************************************/

// Info about a region
typedef struct _rtk_region_t
{
  // Bounding box
  GdkRectangle rect;
  
} rtk_region_t;


// Create a new region.
rtk_region_t *rtk_region_create(void);

// Destroy a region.
void rtk_region_destroy(rtk_region_t *region);

// Set a region to empty.
void rtk_region_set_empty(rtk_region_t *region);

// Set the region to the union of the two given regions.
void rtk_region_set_union(rtk_region_t *regiona, rtk_region_t *regionb);

// Set the region to the union of the region with a rectangle
void rtk_region_set_union_rect(rtk_region_t *region, int ax, int ay, int bx, int by);

// Get the bounding rectangle for the region.
void rtk_region_get_brect(rtk_region_t *region, GdkRectangle *rect);

// Test to see if a region is empty.
int rtk_region_test_empty(rtk_region_t *region);

// Test for intersection betweenr regions.
int rtk_region_test_intersect(rtk_region_t *regiona, rtk_region_t *regionb);


/***************************************************************************
 * Menus : menus, submenus and menu items.
 ***************************************************************************/

// Callback function signatures
typedef void (*rtk_menuitem_fn_t) (struct _rtk_menuitem_t *menuitem);


// Info about a menu
typedef struct
{
  // Which canvas we are attached to.
  rtk_canvas_t *canvas;

  // GTK widget holding the menu label widget.
  GtkWidget *item;

  // GTK menu item widget.
  GtkWidget *menu;
  
} rtk_menu_t;


// Info about a menu item
typedef struct _rtk_menuitem_t
{
  // Which menu we are attached to.
  rtk_menu_t *menu;

  // GTK menu item widget.
  GtkWidget *item;

  // Flag set if item has been activated.
  int activated;

  // Flag set if this is a check-menu item
  int checkitem;

  // Flag set if item is checked.
  int checked;          

  // User data (mainly for callbacks)
  void *userdata;
  
  // Callback function
  rtk_menuitem_fn_t callback;
  
} rtk_menuitem_t;


// Create a menu
rtk_menu_t *rtk_menu_create(rtk_canvas_t *canvas, const char *label);

// Create a sub menu
rtk_menu_t *rtk_menu_create_sub(rtk_menu_t *menu, const char *label);

// Delete a menu
void rtk_menu_destroy(rtk_menu_t *menu);

// Create a new menu item.  Set check to TRUE if you want a checkbox
// with the menu item
rtk_menuitem_t *rtk_menuitem_create(rtk_menu_t *menu,
                                    const char *label, int check);

// Set the callback for a menu item.  This function will be called
// when the user selects the menu item.
void rtk_menuitem_set_callback(rtk_menuitem_t *item,
                               rtk_menuitem_fn_t callback);

// Delete a menu item.
void rtk_menuitem_destroy(rtk_menuitem_t *item);

// Test to see if the menu item has been activated.  Calling this
// function will reset the flag.
int rtk_menuitem_isactivated(rtk_menuitem_t *item);

// Set the check state of a menu item.
void rtk_menuitem_check(rtk_menuitem_t *item, int check);

// Test to see if the menu item is checked.
int rtk_menuitem_ischecked(rtk_menuitem_t *item);

// Enable/disable the menu item
int rtk_menuitem_enable(rtk_menuitem_t *item, int enable);


/***************************************************************************
 * Tables : provides a list of editable var = value pairs.
 ***************************************************************************/

// Info about a single item in a table
typedef struct _rtk_tableitem_t
{
  struct _rtk_tableitem_t *next, *prev;  // Linked list of items
  struct _rtk_table_t *table;
  GtkWidget *label;  // Label widget
  GtkObject *adj;    // Object for storing spin box results.
  GtkWidget *spin;   // Spin box widget
  double value;      // Last recorded value of the item
} rtk_tableitem_t;


// Info about a table
typedef struct _rtk_table_t
{
  struct _rtk_table_t *next, *prev;  // Linked list of tables
  rtk_app_t *app;         // Link to our parent app
  GtkWidget *frame;       // A top-level window to put the table in.
  GtkWidget *table;       // The table layout
  int destroyed;          // Flag set to true if the GTK frame has been destroyed.
  int item_count;         // Number of items currently in the table
  int row_count;          // Number of rows currently in the table
  rtk_tableitem_t *item;  // Linked list of items that belong in the table
} rtk_table_t;

// Create a new table
rtk_table_t *rtk_table_create(rtk_app_t *app, int width, int height);

// Delete the table
void rtk_table_destroy(rtk_table_t *table);

// Create a new item in the table
rtk_tableitem_t *rtk_tableitem_create_int(rtk_table_t *table,
                                          const char *label, int low, int high);

// Set the value of a table item (as an integer)
void rtk_tableitem_set_int(rtk_tableitem_t *item, int value);

// Get the value of a table item (as an integer)
int rtk_tableitem_get_int(rtk_tableitem_t *item);

#ifdef __cplusplus
}
#endif

#endif

