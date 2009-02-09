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
 * Desc: Rtk canvas functions
 * Author: Andrew Howard
 * Contributors: Richard Vaughan
 * CVS: $Id: rtk_canvas.c 2686 2005-02-03 23:44:17Z gerkey $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>

#if HAVE_JPEGLIB_H
#include <jpeglib.h>
#endif

#define DEBUG
#include "rtk.h"
#include "rtkprivate.h"

// Declare some local functions
static gboolean rtk_on_destroy(GtkWidget *widget, rtk_canvas_t *canvas);
static void rtk_on_configure(GtkWidget *widget, GdkEventConfigure *event, rtk_canvas_t *canvas);
static void rtk_on_expose(GtkWidget *widget, GdkEventExpose *event, rtk_canvas_t *canvas);
static void rtk_on_press(GtkWidget *widget, GdkEventButton *event, rtk_canvas_t *canvas);
static void rtk_on_motion(GtkWidget *widget, GdkEventMotion *event, rtk_canvas_t *canvas);
static void rtk_on_release(GtkWidget *widget, GdkEventButton *event, rtk_canvas_t *canvas);
static void rtk_on_key_press(GtkWidget *widget, GdkEventKey *event, rtk_canvas_t *canvas);
static void rtk_canvas_mouse(rtk_canvas_t *canvas, int event, int button, int x, int y);

// Export functions
static int rtk_canvas_save_jpeg(rtk_canvas_t *canvas, const char *filename,
                                int sizex, int sizey, uint8_t *image);
static int rtk_canvas_save_ppm(rtk_canvas_t *canvas, const char *filename,
                               int sizex, int sizey, uint8_t *image);


// Mouse modes 
enum {MOUSE_NONE, MOUSE_PAN, MOUSE_ZOOM, MOUSE_TRANS, MOUSE_ROT, MOUSE_SCALE};

// Mouse events
enum {EVENT_PRESS, EVENT_MOTION, EVENT_RELEASE};

// Convert from device to logical coords
#define LX(x) (canvas->ox + (+(x) - canvas->sizex / 2) * canvas->sx)
#define LY(y) (canvas->oy + (-(y) + canvas->sizey / 2) * canvas->sy)

// Convert from logical to device coords
#define DX(x) (canvas->sizex / 2 + ((x) - canvas->ox) / canvas->sx)
#define DY(y) (canvas->sizey / 2 - ((y) - canvas->oy) / canvas->sy)


// Create a canvas
rtk_canvas_t *rtk_canvas_create(rtk_app_t *app)
{
  int l;
  rtk_canvas_t *canvas;

  // Create canvas
  canvas = calloc(1, sizeof(rtk_canvas_t));

  // Append canvas to linked list
  RTK_LIST_APPEND(app->canvas, canvas);
    
  canvas->app = app;
  canvas->sizex = 0;
  canvas->sizey = 0;
  canvas->ox = 0.0;
  canvas->oy = 0.0;
  canvas->sx = 0.01;
  canvas->sy = 0.01;
  canvas->destroyed = FALSE;
  canvas->bg_dirty = TRUE;
  canvas->fg_dirty = TRUE;
  canvas->fg_dirty_region = rtk_region_create();
  canvas->calc_deferred = 0;
  canvas->movemask = RTK_MOVE_PAN | RTK_MOVE_ZOOM;

  canvas->fig = NULL;
  canvas->layer_fig = NULL;

  /* REMOVE
  // Create a mutex and some TLS
  pthread_key_create(&canvas->key, NULL);
  pthread_mutex_init(&canvas->mutex, NULL);
  */
  
  // Initialise mouse handling
  canvas->zoom_fig = NULL;
  canvas->mouse_mode = MOUSE_NONE;
  canvas->mouse_over_fig = NULL;  
  canvas->mouse_selected_fig = NULL;

  // Create a top-level window
  canvas->frame = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  canvas->layout = gtk_vbox_new(FALSE, 0);

  // Create gtk drawing area
  canvas->canvas = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(canvas->canvas), 100, 100);

  // Create menu bar
  canvas->menu_bar = gtk_menu_bar_new();
  
  // Put it all together
  gtk_container_add(GTK_CONTAINER(canvas->frame), canvas->layout);
  gtk_box_pack_start(GTK_BOX(canvas->layout), canvas->menu_bar, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(canvas->layout), canvas->canvas, TRUE, TRUE, 0);

  canvas->bg_pixmap = NULL;
  canvas->fg_pixmap = NULL;
  canvas->gc = NULL;
  canvas->colormap = NULL;

  // Set default drawing attributes
  canvas->fontname = strdup("-adobe-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*");
  canvas->font = gdk_font_load(canvas->fontname);
  canvas->bgcolor.red = 0xFFFF;
  canvas->bgcolor.green = 0xFFFF;
  canvas->bgcolor.blue = 0xFFFF;
  canvas->linewidth = 0;

  // Movie stuff
  canvas->movie_context = NULL;

  // Connect gtk signal handlers
  gtk_signal_connect(GTK_OBJECT(canvas->frame), "destroy",
                     GTK_SIGNAL_FUNC(rtk_on_destroy), canvas);
  gtk_signal_connect(GTK_OBJECT(canvas->canvas), "configure_event", 
                     GTK_SIGNAL_FUNC(rtk_on_configure), canvas);
  gtk_signal_connect(GTK_OBJECT(canvas->canvas), "expose_event", 
                     GTK_SIGNAL_FUNC(rtk_on_expose), canvas);
  gtk_signal_connect(GTK_OBJECT(canvas->canvas), "button_press_event", 
                     GTK_SIGNAL_FUNC(rtk_on_press), canvas);
  gtk_signal_connect(GTK_OBJECT(canvas->canvas), "motion_notify_event", 
                     GTK_SIGNAL_FUNC(rtk_on_motion), canvas);
  gtk_signal_connect(GTK_OBJECT(canvas->canvas), "button_release_event", 
                     GTK_SIGNAL_FUNC(rtk_on_release), canvas);
  gtk_signal_connect(GTK_OBJECT(canvas->frame), "key-press-event", 
                     GTK_SIGNAL_FUNC(rtk_on_key_press), canvas);

  // Set the event mask
  gtk_widget_set_events(canvas->frame,
                        GDK_KEY_PRESS_MASK);
  gtk_widget_set_events(canvas->canvas,
                        GDK_EXPOSURE_MASK |
                        GDK_BUTTON_PRESS_MASK |
                        GDK_BUTTON_RELEASE_MASK |
                        GDK_POINTER_MOTION_MASK );

  // rtv - support for motion hints would be handy - todo?
  //GDK_POINTER_MOTION_HINT_MASK);

  // show all layers by default
  for( l=0; l<RTK_CANVAS_LAYERS; l++ )
    canvas->layer_show[l] = 1;

  return canvas;
}

// Delete the canvas
void rtk_canvas_destroy(rtk_canvas_t *canvas)
{
  int count;

  // Finish any movies
  if (canvas->movie_context)
    rtk_canvas_movie_stop(canvas);
  
  // Get rid of any figures we still have
  count = 0;
  while (canvas->fig)
  {
    rtk_fig_destroy(canvas->fig);
    count++;
  }
  if (count > 0)
    PRINT_WARN1("garbage collected %d figures", count);

  // Clear the dirty regions
  rtk_region_destroy(canvas->fg_dirty_region);
  
  // Remove ourself from the linked list in the app
  RTK_LIST_REMOVE(canvas->app->canvas, canvas);

  // Destroy the frame
  gtk_widget_hide(GTK_WIDGET(canvas->frame));
  gtk_widget_destroy(GTK_WIDGET(canvas->frame));
  
  // fontname was strdup()ed 
  if (canvas->fontname)
    free(canvas->fontname);

  // Free ourself
  free(canvas);

  return;
}


// Lock the canvas
// This implements recursive mutexes using TLD
void rtk_canvas_lock(rtk_canvas_t *canvas)
{
  /* REMOVE
  int count;

  count = (int) pthread_getspecific(canvas->key);
  if (count == 0)
    pthread_mutex_lock(&canvas->mutex);
  count++;
  pthread_setspecific(canvas->key, (void*) count);
  */
}


// Unlock the canvas
void rtk_canvas_unlock(rtk_canvas_t *canvas)
{
  /*REMOVE
  int count;

  count = (int) pthread_getspecific(canvas->key);
  count--;
  pthread_setspecific(canvas->key, (void*) count);
  if (count == 0)
    pthread_mutex_unlock(&canvas->mutex);
*/
}


// See if the canvas has been closed
int rtk_canvas_isclosed(rtk_canvas_t *canvas)
{
  return canvas->destroyed;
}


// Set the canvas title
void rtk_canvas_title(rtk_canvas_t *canvas, const char *title)
{
  gtk_window_set_title(GTK_WINDOW(canvas->frame), title);
}


// Set the size of a canvas
// (sizex, sizey) is the width and height of the canvas, in pixels.
void rtk_canvas_size(rtk_canvas_t *canvas, int sizex, int sizey)
{
  GtkRequisition size;
  
  // Interpret the given size as the size of the drawing area, and use
  // this to compute the default window size.  There must be a less hacky way
  // to do this.
  gtk_widget_size_request(GTK_WIDGET(canvas->menu_bar), &size);
  sizey += size.height + 4;
  gtk_window_set_default_size(GTK_WINDOW(canvas->frame), sizex, sizey);

  // You can do use the line below, but then you cant shrink the window.
  //gtk_drawing_area_size(GTK_DRAWING_AREA(canvas->canvas), sizex, sizey);
  return;
}


// Get the canvas size.
// (sizex, sizey) is the width and height of the canvas, in pixels.
void rtk_canvas_get_size(rtk_canvas_t *canvas, int *sizex, int *sizey)
{
  *sizex = canvas->sizex;
  *sizey = canvas->sizey;
  return;
}


// Set the origin of a canvas
// (ox, oy) specifies the logical point that maps to the center of the
// canvas.
void rtk_canvas_origin(rtk_canvas_t *canvas, double ox, double oy)
{
  canvas->ox = ox;
  canvas->oy = oy;

  // Re-calculate all the figures.
  rtk_canvas_calc(canvas);
}


// Get the origin of a canvas
// (ox, oy) specifies the logical point that maps to the center of the
// canvas.
void rtk_canvas_get_origin(rtk_canvas_t *canvas, double *ox, double *oy)
{
  *ox = canvas->ox;
  *oy = canvas->oy;
}


// Scale a canvas
// Sets the pixel width and height in logical units
void rtk_canvas_scale(rtk_canvas_t *canvas, double sx, double sy)
{
  canvas->sx = sx;
  canvas->sy = sy;
  
  // Re-calculate all the figures.
  rtk_canvas_calc(canvas);
}


// Get the scale of the canvas
// (sx, sy) are the pixel with and height in logical units
void rtk_canvas_get_scale(rtk_canvas_t *canvas, double *sx, double *sy)
{
  *sx = canvas->sx;
  *sy = canvas->sy;
}


// Set the movement mask
// Set the mask to a bitwise combination of RTK_MOVE_TRANS, RTK_MOVE_SCALE.
// to enable user manipulation of the canvas.
void rtk_canvas_movemask(rtk_canvas_t *canvas, int mask)
{
  canvas->movemask = mask;
}


// Set the default font for text strokes
void rtk_canvas_font(rtk_canvas_t *canvas, const char *fontname)
{
  rtk_canvas_lock(canvas);

  if (canvas->font)
  {
    gdk_font_unref(canvas->font);
    canvas->font = NULL;
  }

  if (canvas->fontname)
  {
    free(canvas->fontname);
    canvas->fontname = strdup(fontname);
  }

  // Load the default font
  if (canvas->font == NULL)
    canvas->font = gdk_font_load(canvas->fontname);

  // Text extents will have changed, so recalc everything.
  rtk_canvas_calc(canvas);

  rtk_canvas_unlock(canvas);
}


// Set the canvas backround color
void rtk_canvas_bgcolor(rtk_canvas_t *canvas, double r, double g, double b)
{
  canvas->bgcolor.red = (int) (r * 0xFFFF);
  canvas->bgcolor.green = (int) (g * 0xFFFF);
  canvas->bgcolor.blue = (int) (b * 0xFFFF);
  return;
}


// Set the default line width.
void rtk_canvas_linewidth(rtk_canvas_t *canvas, int width)
{
  canvas->linewidth = width;
}

///
// rtv experimental feature
//

// the figure will be shown until rtk_canvas_flash_update() is called
// [duration] times. if [kill] is non-zero, the fig will
// also be destroyed when its counter expires.
void rtk_canvas_flash( rtk_canvas_t* canvas, rtk_fig_t* fig, int duration, 
		       int kill )
{
  rtk_flasher_t* flasher = malloc( sizeof(rtk_flasher_t) );
  
  // force the fig visible
  rtk_fig_show( fig, 1 );
  
  flasher->fig = fig;
  flasher->duration = duration;
  flasher->kill = kill;
  
  RTK_LIST_APPEND( canvas->flashers, flasher );
}

void rtk_canvas_flash_update( rtk_canvas_t* canvas )
{
  rtk_flasher_t* flasher = canvas->flashers; 
  
  while( flasher != NULL )
    {
      //rtk_fig_t* fig = flasher->fig;      
      flasher->duration--;
      
      // if it's time to flip, flip
      if( flasher->duration < 1 )
	{
	  rtk_flasher_t* doomed = flasher;
	  
	  // force the fig invisible
	  if( doomed->kill )
	    rtk_fig_and_descendents_destroy( doomed->fig );
	  else
	    rtk_fig_show( doomed->fig, 0);
	  
	  flasher = flasher->next;	  
	  RTK_LIST_REMOVE( canvas->flashers, doomed );
	  continue;
	}
      
      flasher = flasher->next;
    }
}

void rtk_canvas_layer_show( rtk_canvas_t* canvas, int layer, char show )
{
  canvas->layer_show[layer] = show;
  
  // invalidate the whole window
  rtk_canvas_calc(canvas);
} 

// end rtv experimental

// Re-calculate all the figures (private).
void rtk_canvas_calc(rtk_canvas_t *canvas)
{
  rtk_fig_t *fig;
  
  rtk_canvas_lock(canvas);

  // The whole window is dirty
  canvas->bg_dirty = TRUE;
  canvas->fg_dirty = TRUE;

  // Add the whole window to the dirty region
  rtk_region_set_union_rect(canvas->fg_dirty_region, 0, 0, canvas->sizex, canvas->sizey);

  // Update all the figures
  for (fig = canvas->fig; fig != NULL; fig = fig->sibling_next)
    rtk_fig_calc(fig);
  
  rtk_canvas_unlock(canvas);
  return;
}


// Render the figures in the canvas
void rtk_canvas_render(rtk_canvas_t *canvas)
{
  int bg_count;
  rtk_fig_t *fig;
  GdkColor color;

  int rcount;
  GdkRectangle clipbox;

  if (canvas->destroyed)
    return;

  // If there where deferred computations, do them now.
  if (canvas->calc_deferred)
  {
    rtk_canvas_calc(canvas);
    canvas->calc_deferred = 0;
  }
  
  rtk_canvas_lock(canvas);

  // Set the canvas color
  gdk_color_alloc(canvas->colormap, &canvas->bgcolor);
  gdk_gc_set_foreground(canvas->gc, &canvas->bgcolor);
  
  // See if there is anything in the background.
  // TODO: optimize
  bg_count = 0;
  for (fig = canvas->layer_fig; fig != NULL; fig = fig->layer_next)
  {
    if (fig->layer < 0 && fig->show)
    {
      bg_count++;
      break;
    }
  }

  // Render the background
  if (canvas->bg_dirty)
  {
    // Clear background pixmap
    gdk_draw_rectangle(canvas->bg_pixmap, canvas->gc, TRUE,
                       0, 0, canvas->sizex, canvas->sizey);

    // Render all figures, in order of layer
    for (fig = canvas->layer_fig; fig != NULL; fig = fig->layer_next)
    {
      if (fig->layer < 0)
        rtk_fig_render(fig);
    }

    // Copy background pixmap to foreground pixmap
    gdk_draw_pixmap(canvas->fg_pixmap, canvas->gc, canvas->bg_pixmap,
                    0, 0, 0, 0, canvas->sizex, canvas->sizey);

    // The entire forground needs redrawing
    rtk_region_set_union_rect(canvas->fg_dirty_region,
                             0, 0, canvas->sizex, canvas->sizey);
  }

  // Render the foreground
  if (canvas->bg_dirty || canvas->fg_dirty)
  {
    // Clip drawing to the dirty region
    rtk_region_get_brect(canvas->fg_dirty_region, &clipbox);
    gdk_gc_set_clip_rectangle(canvas->gc, &clipbox);

    // Copy background pixmap to foreground pixmap
    gdk_draw_pixmap(canvas->fg_pixmap, canvas->gc, canvas->bg_pixmap,
                    clipbox.x, clipbox.y, clipbox.x, clipbox.y,
                    clipbox.width, clipbox.height);
    rcount = 0;
    
    // Render all figures, in order of layer
    for (fig = canvas->layer_fig; fig != NULL; fig = fig->layer_next)
    {
      // Make sure the figure is in the foreground
      //if (fig->layer >= 0)
      if (fig->layer >= 0 && canvas->layer_show[fig->layer] > 0 )
      {
        // Only draw figures in the dirty region
        if (rtk_region_test_intersect(canvas->fg_dirty_region, fig->region))
        {
          rcount++;
          rtk_fig_render(fig);
        }
      }
    }

    // Now copy foreground pixmap to screen
    gdk_draw_pixmap(canvas->canvas->window, canvas->gc, canvas->fg_pixmap,
                    clipbox.x, clipbox.y, clipbox.x, clipbox.y,
                    clipbox.width, clipbox.height);
    gdk_gc_set_clip_rectangle(canvas->gc, NULL);
  }

  // Reset the dirty regions
  canvas->bg_dirty = FALSE;
  canvas->fg_dirty = FALSE;
  rtk_region_set_empty(canvas->fg_dirty_region);

  gdk_colormap_free_colors(canvas->colormap, &canvas->bgcolor, 1);
  
  rtk_canvas_unlock(canvas);
}


// Export an image.
// [filename] is the name of the file to save.
// [format] is the image file format (RTK_IMAGE_FORMAT_JPEG, RTK_IMAGE_FORMAT_PPM).
void rtk_canvas_export_image(rtk_canvas_t *canvas, const char *filename, int format)
{
  uint8_t *image;
  
  // Get an image in canonical 24-bit RGB format
  image = rtk_canvas_get_image_rgb24(canvas);
  if (!image)
    return;
  
  switch (format)
  {
    case RTK_IMAGE_FORMAT_JPEG:
      rtk_canvas_save_jpeg(canvas, filename, canvas->sizex, canvas->sizey, image);
      break;
    case RTK_IMAGE_FORMAT_PPM:
      rtk_canvas_save_ppm(canvas, filename, canvas->sizex, canvas->sizey, image);
      break;
  }
  
  free(image);
  return;
}


// Grab an image in canonical 16-bit form
uint16_t *rtk_canvas_get_image_rgb16(rtk_canvas_t *canvas, int sizex, int sizey)
{
  GdkImage *im;
  int ix, iy;
  guint32 c;
  uint16_t *pixel;
  uint16_t *image;

  // Get the image
  im = gdk_image_get((GdkDrawable*) canvas->fg_pixmap,
                     0, 0, sizex, sizey);
  if (!im)
  {
    PRINT_ERR("image creation failed; export aborted");
    return NULL;
  }

  // We only to 16 bpp
  if (im->depth != 16 && im->depth != 24)
  {
    PRINT_ERR("only 16 and 24 bpp displays are currently supported; export abandoned");
    return NULL;
  }

  // Create an image buffer
  image = malloc(canvas->sizex * canvas->sizey * 2);
    
  // Copy the data to an RGB16 buffer.
  pixel = image;
  for (iy = 0; iy < sizey; iy++)
  {
    for (ix = 0; ix < sizex; ix++)
    {
      c = gdk_image_get_pixel(im, ix, iy);
      if (im->depth == 16)
        *pixel = c;
      else if (im->depth == 24) {
        unsigned char r,g,b;
        r =  c        & 0xf8;  
        g = (c >>  8) & 0xfc;
        b = (c >> 16) & 0xf8;
        *pixel = (r >> 3 | g << 3 | b << 8);
      }
      pixel++;
    }
  }

  gdk_image_destroy(im);
  return image;
}


// Grab an image in canonical 24-bit form
uint8_t *rtk_canvas_get_image_rgb24(rtk_canvas_t *canvas)
{
  GdkImage *im;
  int ix, iy;
  char *pixel;
  guint32 c;
  unsigned char r, g, b;
  uint8_t *image;

  // Get the image
  im = gdk_image_get((GdkDrawable*) canvas->fg_pixmap,
                     0, 0, canvas->sizex, canvas->sizey);
  if (!im)
  {
    PRINT_ERR("image creation failed; export aborted");
    return NULL;
  }

  // We only to 16 and 24 bpp
  if((im->depth != 16) && (im->depth != 24))
  {
    PRINT_ERR("only 16 and 24 bpp displays are currently supported; export abandoned");
    return NULL;
  }

  // Create an image buffer
  image = malloc(canvas->sizex * canvas->sizey * 3);
    
  // Copy the data to an RGB32 buffer.
  pixel = image;
  for (iy = 0; iy < canvas->sizey; iy++)
  {
    for (ix = 0; ix < canvas->sizex; ix++)
    {
      c = gdk_image_get_pixel(im, ix, iy);

      if(im->depth == 16)
      {
        // Assumes 16 bit (565) color; byte order has to be correct,
        // also.
        r = (c >> 8) & 0xF8;
        g = (c >> 3) & 0xFC;
        b = (c << 3) & 0xF8;
      }
      else
      {
        b = (c >> 0) & 0xFF;
        g = (c >> 8) & 0xFF;
        r = (c >> 16) & 0xFF;
      }

      *(pixel++) = r;
      *(pixel++) = g;
      *(pixel++) = b;
    }
  }

  gdk_image_destroy(im);
  return image;
}


// Export the canvas as an xfig image.
int rtk_canvas_export_xfig(rtk_canvas_t *canvas, char *filename)
{
  int wx, wy;
  rtk_fig_t *fig;
    
  canvas->file = fopen(filename, "w+");
  if (canvas->file == NULL)
  {
    PRINT_ERR2("unable to open [%s] for export; [%s]", filename, strerror(errno));
    return -1;
  }

  rtk_canvas_lock(canvas);
    
  // Write header info
  fprintf(canvas->file, "#FIG 3.2\n");
  fprintf(canvas->file, "Portrait\nCenter\nInches\nLetter\n100.00\nSingle\n");
  fprintf(canvas->file, "-2\n 1200 2\n");

  // Create a bounding box
  // Box has line width of zero, so it hopefully wont be visible
  wx = 1200 * 6;
  wy = (int) (1200 * 6 * canvas->sizey / canvas->sizex);
  fprintf(canvas->file, "2 3 0 0 0 7 50 0 -1 0.000 0 0 -1 0 0 5\n");
  fprintf(canvas->file, "0 0 %d %d %d %d %d %d 0 0\n", wx, 0, wx, wy, 0, wy);

  // Render all figures, in order of layer
  // TODO: optimize
  for (fig = canvas->layer_fig; fig != NULL; fig = fig->layer_next)
    rtk_fig_render_xfig(fig);

  // Clean up
  fclose(canvas->file);
  canvas->file = NULL;
    
  rtk_canvas_unlock(canvas);

  return 0;
}

// Save a ppm image.
int rtk_canvas_save_ppm(rtk_canvas_t *canvas, const char *filename,
                        int sizex, int sizey, uint8_t *image)
{
  FILE *file;

  if (!filename)
  {
    PRINT_ERR("export filename is NULL; ignoring");
    return -1;
  }
  
  file = fopen(filename, "w+");
  if (!file)
  {
    PRINT_ERR2("unable to open [%s] for export: [%s]",
               filename, strerror(errno));
    return -1;
  }

  // Save PPM image
  fprintf(file, "P6\n%d %d\n255\n", sizex, sizey);
  fwrite(image, sizex * sizey * 3, 1, file);
  fclose(file);

  return 0;
}


// Save a jpeg image
int rtk_canvas_save_jpeg(rtk_canvas_t *canvas, const char *filename,
                         int sizex, int sizey, uint8_t *image)
{
#if HAVE_JPEGLIB_H

  FILE *file;
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW jrow[1];
  
  // Now save it to a file
  if (!filename)
  {
    PRINT_ERR("export filename is NULL; ignoring");
    return -1;
  }
  
  file = fopen(filename, "w+");
  if (!file)
  {
    PRINT_ERR2("unable to open [%s] for export: [%s]",
               filename, strerror(errno));
    return -1;
  }

  // Initialize jpeg compressor
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  // Point compressor to output file
  jpeg_stdio_dest(&cinfo, file);

  // Set image parameters
  cinfo.image_width = sizex;
  cinfo.image_height = sizey;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&cinfo);

  // Compress image
  jpeg_start_compress(&cinfo, TRUE);
  while (cinfo.next_scanline < cinfo.image_height)
  {
    jrow[0] = image + cinfo.next_scanline * sizex * 3;
    jpeg_write_scanlines(&cinfo, jrow, 1);
  }
  jpeg_finish_compress(&cinfo);

  // Finialize compressor
  jpeg_destroy_compress(&cinfo);

  // Close the output file
  fclose(file);

  return 0;

#else

  PRINT_ERR("jpeg support has not been compiled in; export failed.");
  return -1;

#endif
}


// Pixel tolerances for moving stuff
#define TOL_MOVE 15

// See if there is a moveable figure close to the given device point.
rtk_fig_t *rtk_canvas_pick_fig(rtk_canvas_t *canvas, int x, int y)
{
  int maxlayer;
  rtk_fig_t *fig, *maxfig;

  maxfig = NULL;
  maxlayer = INT_MIN;

  // TODO: optimize
  for (fig = canvas->layer_fig; fig != NULL; fig = fig->layer_next)
  {
    if (!fig->show)
      continue;
    if (fig->movemask == 0)
      continue;
    if (rtk_fig_hittest(fig, x, y))
    {
      if (fig->layer >= maxlayer)
      {
        maxfig = fig;
        maxlayer = fig->layer;
      }
    }
  }
  return maxfig;
}

// Do mouse stuff
void rtk_canvas_mouse(rtk_canvas_t *canvas, int event, int button, int x, int y)
{
  double px, py, pa, rd, rl;
  rtk_fig_t *fig;
    
  if (event == EVENT_PRESS)
  {        
    // See of there are any moveable figures at this point
    fig = rtk_canvas_pick_fig(canvas, x, y);

    // If there are moveable figures...
    if (fig)
    {
      if (button == 1 && (fig->movemask & RTK_MOVE_TRANS))
      {
        canvas->mouse_mode = MOUSE_TRANS;
        canvas->mouse_start_x = LX(x) - fig->dox;
        canvas->mouse_start_y = LY(y) - fig->doy;
        canvas->mouse_selected_fig = fig;
        rtk_fig_dirty(fig);
        rtk_fig_on_mouse(canvas->mouse_selected_fig, RTK_EVENT_PRESS, canvas->mouse_mode);
      }
      else if (button == 3 && (fig->movemask & RTK_MOVE_ROT))
      {
        canvas->mouse_mode = MOUSE_ROT;
        px = LX(x) - fig->dox;
        py = LY(y) - fig->doy;
        canvas->mouse_start_a = atan2(py, px) - fig->doa;
        canvas->mouse_selected_fig = fig;
        rtk_fig_dirty(fig);
        rtk_fig_on_mouse(canvas->mouse_selected_fig, RTK_EVENT_PRESS, canvas->mouse_mode);
      }

      // rtv - fixed and reinstated scroll wheel support 1/7/03
      // rtv - handle the mouse scroll wheel for rotating objects 
      else if( button == 4 && (fig->movemask & RTK_MOVE_ROT))
	{
	  rtk_fig_dirty(fig);
	  rtk_fig_origin_global(fig, fig->dox, fig->doy, fig->doa + 0.2 );
	  rtk_fig_on_mouse(fig, RTK_EVENT_PRESS, canvas->mouse_mode);
	  
	  return;
	}
      else if( button == 5 && (fig->movemask & RTK_MOVE_ROT))
	{
	  rtk_fig_dirty(fig);
	  rtk_fig_origin_global(fig, fig->dox, fig->doy, fig->doa - 0.2 );
	  rtk_fig_on_mouse(fig, RTK_EVENT_PRESS, canvas->mouse_mode);
	  return;
	}
      
    }

    // Else translate and scale the canvas...
    else
    {
      if (button == 1 && (canvas->movemask & RTK_MOVE_PAN))
      {
        // Store the logical coordinate of the start point
        canvas->mouse_mode = MOUSE_PAN;
        canvas->mouse_start_x = LX(x);
        canvas->mouse_start_y = LY(y);
      }
      else if (button == 3 && (canvas->movemask & RTK_MOVE_ZOOM))
      {
        // Store the logical coordinate of the start point
        canvas->mouse_mode = MOUSE_ZOOM;
        canvas->mouse_start_x = LX(x);
        canvas->mouse_start_y = LY(y);

        // Create a figure for showing the zoom
        //assert(canvas->zoom_fig == NULL);
        if( canvas->zoom_fig == NULL )
	  { 
	    canvas->zoom_fig = rtk_fig_create(canvas, NULL, RTK_CANVAS_LAYERS-1);
	    px = LX(canvas->sizex / 2);
	    py = LY(canvas->sizey / 2);
	    rl = 2 * sqrt((LX(x) - px) * (LX(x) - px) + (LY(y) - py) * (LY(y) - py));
	    rtk_fig_ellipse(canvas->zoom_fig, px, py, 0, rl, rl, 0);
	  }
      }

      /* BROKEN
      // handle the mouse scroll wheel for cancas zooming
      else if (button == 4 && (canvas->movemask & RTK_MOVE_ZOOM))
      {
        canvas->mouse_mode = MOUSE_ZOOM;
        canvas->sx *= 0.9;
        canvas->sy *= 0.9;
      }
      else if (button == 5 && (canvas->movemask & RTK_MOVE_ZOOM))
      {
        canvas->mouse_mode = MOUSE_ZOOM;
        canvas->sx *= 1.1;
        canvas->sy *= 1.1;
      }
      */
    }
  }

  if (event == EVENT_MOTION)
  {            
    if (canvas->mouse_mode == MOUSE_TRANS)
    {
      // Translate the selected figure
      fig = canvas->mouse_selected_fig;
      px = LX(x) - canvas->mouse_start_x;
      py = LY(y) - canvas->mouse_start_y;
      rtk_fig_dirty(fig);
      rtk_fig_origin_global(fig, px, py, fig->doa);
      rtk_fig_on_mouse(canvas->mouse_selected_fig, RTK_EVENT_MOTION, canvas->mouse_mode);
    }
    else if (canvas->mouse_mode == MOUSE_ROT)
    {
      // Rotate the selected figure 
      fig = canvas->mouse_selected_fig;
      px = LX(x) - fig->dox;
      py = LY(y) - fig->doy;
      pa = atan2(py, px) - canvas->mouse_start_a;
      rtk_fig_dirty(fig);
      rtk_fig_origin_global(fig, fig->dox, fig->doy, pa);
      rtk_fig_on_mouse(canvas->mouse_selected_fig, RTK_EVENT_MOTION, canvas->mouse_mode);
    }
    else if (canvas->mouse_mode == MOUSE_PAN)
    {
      // Infer the translation that will map the current physical mouse
      // point to the original logical mouse point.        
      canvas->ox = canvas->mouse_start_x -
        (+x - canvas->sizex / 2) * canvas->sx;
      canvas->oy = canvas->mouse_start_y -
        (-y + canvas->sizey / 2) * canvas->sy;
    }
    else if (canvas->mouse_mode == MOUSE_ZOOM)
    {
      // Compute scale change that will map original logical point
      // onto circle intersecting current mouse point.
      px = x - canvas->sizex / 2;
      py = y - canvas->sizey / 2;
      rd = sqrt(px * px + py * py) + 1;
      px = canvas->mouse_start_x - canvas->ox;
      py = canvas->mouse_start_y - canvas->oy;
      rl = sqrt(px * px + py * py);
      canvas->sy = rl / rd * canvas->sy / canvas->sx;        
      canvas->sx = rl / rd;
    }
    else if (canvas->mouse_mode == MOUSE_NONE)
    {
      // See of there are any moveable figures at this point
      fig = rtk_canvas_pick_fig(canvas, x, y);
      if (fig)
        rtk_fig_dirty(fig);
      if (canvas->mouse_over_fig)
        rtk_fig_dirty(canvas->mouse_over_fig);
      if (fig != canvas->mouse_over_fig)
      {
        if (canvas->mouse_over_fig)
          rtk_fig_on_mouse(canvas->mouse_over_fig, RTK_EVENT_MOUSE_NOT_OVER, canvas->mouse_mode);
        if (fig)
          rtk_fig_on_mouse(fig, RTK_EVENT_MOUSE_OVER, canvas->mouse_mode);
      }
      canvas->mouse_over_fig = fig;
    }

    if (canvas->mouse_mode == MOUSE_PAN || canvas->mouse_mode == MOUSE_ZOOM)
    {
      // Re-compute figures (deferred, will be done on next render).
      canvas->calc_deferred++;
    }
  }

  if (event == EVENT_RELEASE)
  {
    if (canvas->mouse_mode == MOUSE_PAN || canvas->mouse_mode == MOUSE_ZOOM)
    {
      // Re-compute figures (deferred, will be done on next render).
      canvas->calc_deferred++;

      // Delete zoom figure
      if (canvas->zoom_fig != NULL)
      {
        rtk_fig_destroy(canvas->zoom_fig);
        canvas->zoom_fig = NULL;
      }
  
      // Reset mouse mode
      canvas->mouse_mode = MOUSE_NONE;
      canvas->mouse_selected_fig = NULL;
    }
    else
    {
      fig = canvas->mouse_selected_fig;
            
      // Reset mouse mode
      canvas->mouse_mode = MOUSE_NONE;
      canvas->mouse_selected_fig = NULL;
      
      // Do callbacks
      if (fig)
      {
        rtk_fig_dirty(fig);
        rtk_fig_on_mouse(fig, RTK_EVENT_RELEASE, canvas->mouse_mode);
      }
    }
  }
}


// Handle destroy events
gboolean rtk_on_destroy(GtkWidget *widget, rtk_canvas_t *canvas)
{
  canvas->destroyed = TRUE;
  return FALSE;
}


// Process configure events
void rtk_on_configure(GtkWidget *widget, GdkEventConfigure *event, rtk_canvas_t *canvas)
{
  GdkColor color;
  
  rtk_canvas_lock(canvas);
  
  canvas->sizex = event->width;
  canvas->sizey = event->height;

  if (canvas->gc == NULL)
    canvas->gc = gdk_gc_new(canvas->canvas->window);
  if (canvas->colormap == NULL)
    canvas->colormap = gdk_colormap_get_system();
  
  // Create offscreen pixmaps
  if (canvas->bg_pixmap != NULL)
    gdk_pixmap_unref(canvas->bg_pixmap);
  if (canvas->fg_pixmap != NULL)
    gdk_pixmap_unref(canvas->fg_pixmap);
  canvas->bg_pixmap = gdk_pixmap_new(canvas->canvas->window,
                                     canvas->sizex, canvas->sizey, -1);
  canvas->fg_pixmap = gdk_pixmap_new(canvas->canvas->window,
                                     canvas->sizex, canvas->sizey, -1);

  // Make sure we redraw with a white background
  gdk_color_white(canvas->colormap, &color);
  gdk_gc_set_foreground(canvas->gc, &color);

  // Clear pixmaps
  gdk_draw_rectangle(canvas->bg_pixmap, canvas->gc, TRUE,
                     0, 0, canvas->sizex, canvas->sizey);
  gdk_draw_rectangle(canvas->fg_pixmap, canvas->gc, TRUE,
                     0, 0, canvas->sizex, canvas->sizey);
    
  // Re-calculate all the figures since the coord transform has
  // changed.
  canvas->calc_deferred++;

  rtk_canvas_unlock(canvas);
}


// Process expose events
void rtk_on_expose(GtkWidget *widget, GdkEventExpose *event, rtk_canvas_t *canvas)
{
  if (canvas->fg_pixmap)
  {
    // Copy foreground pixmap to screen
    gdk_draw_pixmap(canvas->canvas->window, canvas->gc, canvas->fg_pixmap,
                    0, 0, 0, 0, canvas->sizex, canvas->sizey);
  }
}


// Process keyboard events
void rtk_on_key_press(GtkWidget *widget, GdkEventKey *event, rtk_canvas_t *canvas)
{
  double scale, dx, dy;
  //PRINT_DEBUG3("key: %d %d %s", event->keyval, event->state, gdk_keyval_name(event->keyval));

  dx = canvas->sizex * canvas->sx;
  dy = canvas->sizey * canvas->sy;
  scale = 1.5;
  
  switch (event->keyval)
  {
    case GDK_Left:
      if (event->state == 0)
        rtk_canvas_origin(canvas, canvas->ox - 0.05 * dx, canvas->oy);
      else if (event->state == 4)
        rtk_canvas_origin(canvas, canvas->ox - 0.5 * dx, canvas->oy);
      break;
    case GDK_Right:
      if (event->state == 0)
        rtk_canvas_origin(canvas, canvas->ox + 0.05 * dx, canvas->oy);
      else if (event->state == 4)
        rtk_canvas_origin(canvas, canvas->ox + 0.5 * dx, canvas->oy);
      break;
    case GDK_Up:
      if (event->state == 0)
        rtk_canvas_origin(canvas, canvas->ox, canvas->oy + 0.05 * dy);
      else if (event->state == 4)
        rtk_canvas_origin(canvas, canvas->ox, canvas->oy + 0.5 * dy);
      else if (event->state == 1)
        rtk_canvas_scale(canvas, canvas->sx / scale, canvas->sy / scale);
      break;
    case GDK_Down:
      if (event->state == 0)
        rtk_canvas_origin(canvas, canvas->ox, canvas->oy - 0.05 * dy);
      else if (event->state == 4)
        rtk_canvas_origin(canvas, canvas->ox, canvas->oy - 0.5 * dy);
      else if (event->state == 1)
        rtk_canvas_scale(canvas, canvas->sx * scale, canvas->sy * scale);
      break;
  }
}


// Process mouse press events
void rtk_on_press(GtkWidget *widget, GdkEventButton *event, rtk_canvas_t *canvas)
{
  rtk_canvas_mouse(canvas, EVENT_PRESS, event->button, event->x, event->y);
}


// Process mouse motion events
void rtk_on_motion(GtkWidget *widget, GdkEventMotion *event, rtk_canvas_t *canvas)
{
  rtk_canvas_mouse(canvas, EVENT_MOTION, 0, event->x, event->y);
}


// Process mouse release events
void rtk_on_release(GtkWidget *widget, GdkEventButton *event, rtk_canvas_t *canvas)
{
  rtk_canvas_mouse(canvas, EVENT_RELEASE, event->button, event->x, event->y);
}



