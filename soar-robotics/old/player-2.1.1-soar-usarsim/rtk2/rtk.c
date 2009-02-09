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
 * Desc: Rtk application functions
 * Author: Andrew Howard
 * CVS: $Id: rtk.c 2686 2005-02-03 23:44:17Z gerkey $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <gdk/gdkkeysyms.h>

#if ENABLE_AVCODEC
#include "avcodec.h"
#endif

//#define DEBUG
#include "rtk.h"
#include "rtkprivate.h"

// Dummy symbol so that autoconf can verify the library version number
// using AC_CHECK_LIB.
int LIBRTK_VERSION_2_2(void) { return(0); }

// Declare some local functions
static int rtk_app_on_timer(rtk_app_t *app);

// Initialise the library.
// Pass in the program arguments through argc, argv
int rtk_init(int *argc, char ***argv)
{
  // Initialise the gtk lib
  gtk_init(argc, argv);

  // Allow rgb image handling
  gdk_rgb_init();

#ifdef ENABLE_AVCODEC
  // Allow movie capture
  avcodec_init();
  avcodec_register_all();
#endif

  return 0;
}


// Create an app
rtk_app_t *rtk_app_create()
{
  rtk_app_t *app;

  app = malloc(sizeof(rtk_app_t));
  app->must_quit = FALSE;
  app->has_quit = FALSE;
  app->canvas = NULL;
  app->table = NULL;

  return app;
}


// Destroy the app
void rtk_app_destroy(rtk_app_t *app)
{
  int count;

  // Get rid of any canvases we still have
  count = 0;
  while (app->canvas)
  {
    rtk_canvas_destroy(app->canvas);
    count++;
  }
  if (count > 0)
    PRINT_WARN1("garbage collected %d canvases", count);

  // Get rid of any tables  we still have
  count = 0;
  while (app->table)
  {
    rtk_table_destroy(app->table);
    count++;
  }
  if (count > 0)
    PRINT_WARN1("garbage collected %d tables", count);

  // Free ourself
  free(app);
}


// Check to see if its time to quit
int rtk_app_quit(rtk_app_t *app)
{
  return (app->has_quit);
}

// Handle timer events
int rtk_app_on_timer(rtk_app_t *app)
{
  /* REMOVE
  rtk_canvas_t *canvas;
  rtk_table_t *table;

  // Update the display
  for (canvas = app->canvas; canvas != NULL; canvas = canvas->next)
    rtk_canvas_update(canvas);
  
  // Quit the app if we have been told we should
  // We first destroy in windows that are still open.
  if (app->must_quit)
  {
    for (canvas = app->canvas; canvas != NULL; canvas = canvas->next)
      if (!canvas->destroyed)
        gtk_widget_destroy(canvas->frame);
    for (table = app->table; table != NULL; table = table->next)
      if (!table->destroyed)
        gtk_widget_destroy(table->frame);
    gtk_main_quit();
  }
  */
  return TRUE;
}

// Main loop -- run in own thread
int rtk_app_main(rtk_app_t *app)
{
  rtk_app_main_init(app);
	gtk_main();
  rtk_app_main_term(app);
  
  return 0;
}


// Do the initial main loop stuff
void rtk_app_main_init(rtk_app_t *app)
{
  rtk_canvas_t *canvas;
  rtk_table_t *table;
  
  // Display everything
  for (canvas = app->canvas; canvas != NULL; canvas = canvas->next)
    gtk_widget_show_all(canvas->frame);
  for (table = app->table; table != NULL; table = table->next)
    gtk_widget_show_all(table->frame);

  return;
}


// Do the final main loop stuff
void rtk_app_main_term(rtk_app_t *app)
{
  // Process remaining events
  while (gtk_events_pending())
    gtk_main_iteration();

  // Note that the app has quit
  app->has_quit = TRUE;

  return;
}


// Event processing function.  Returns non-zero if the app should quit.
int rtk_app_main_loop(rtk_app_t *app)
{
  int ret;
  rtk_canvas_t *canvas;
  rtk_table_t *table;
  
  while (gtk_events_pending())
    ret = gtk_main_iteration();

  // Quit the app if we have been told we should
  // We first destroy in windows that are still open.
  if (app->must_quit)
  {
    for (canvas = app->canvas; canvas != NULL; canvas = canvas->next)
      if (!canvas->destroyed)
        gtk_widget_destroy(canvas->frame);
    for (table = app->table; table != NULL; table = table->next)
      if (!table->destroyed)
        gtk_widget_destroy(table->frame);
    gtk_main_quit();
  }

  return ret;
}

