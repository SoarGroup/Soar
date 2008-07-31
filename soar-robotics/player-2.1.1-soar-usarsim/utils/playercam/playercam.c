/*
 *  PlayerCam
 *  Copyright (C) Brad Kratochvil 2005
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
/**************************************************************************
 * Desc: PlayerCam
 * Author: Brad Kratochvil
 * Date: 20050902
 * CVS: $Id: playercam.c 4232 2007-11-01 22:16:23Z gerkey $
 *************************************************************************/

/** @ingroup utils */
/** @{ */
/** @defgroup util_playercam playercam
 * @brief Camera visualization GUI

@par Synopsis

Playercam is a gui client that displays images captured from a player
@ref interface_camera and/or @ref interface_blobfinder device.

@par Usage

playercam is installed alongside player in $prefix/bin, so if player is
in your PATH, then @em playercam should also be.  Command-line usage is:
@verbatim
$ playercam [options]
@endverbatim

Where [options] can be:
- -help : print this message
- -h &lt;hostname&gt; : host that is running player
- -p &lt;port&gt; : the port number of the host
- -i &lt;index&gt; : the index of the camera
- -b &lt;index&gt; : the index of the blobfinder
- -r &lt;rate&gt; : the refresh rate of the video

For example, to connect to Player on localhost at the default port
(6665), and subscribe to the 1st camera device:
@verbatim
$ playercam -i=1
@endverbatim


@par Features

playercam can visualize data from devices that support the following
colorspaces:
- @ref PLAYER_CAMERA_FORMAT_MONO8
- @ref PLAYER_CAMERA_FORMAT_MONO16
- @ref PLAYER_CAMERA_FORMAT_RGB888

Any time a user clicks on the image display, the pixel location and color
value at that place will be written to standard out.

@todo
- add additional vision feedback abilities w/ opencv (directional histogram)

@author Brad Kratochvil
*/

/** @} */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include <libplayerc/playerc.h>

#define PLAYERCAM_MAX_BLOBS 256

char g_hostname[255]       = "localhost";
int32_t g_port             = 6665;
int16_t g_camera_index     = 0;
int16_t g_blobfinder_index = 0;
int16_t g_rate             = 30;
int16_t g_transport        = PLAYERC_TRANSPORT_TCP;

playerc_client_t* g_client         = NULL;
playerc_camera_t* g_camera         = NULL;
playerc_blobfinder_t* g_blobfinder = NULL;

player_blobfinder_blob_t g_blobs[PLAYERCAM_MAX_BLOBS];
uint16_t g_blob_count             = 0;

int32_t g_window_width  = 0;
int32_t g_window_height = 0;

uint16_t    g_width = 0;
uint16_t   g_height = 0;
GdkPixbuf* g_pixbuf = NULL;
size_t allocated_size = 0;
guchar *g_img = NULL;

int player_init(int argc, char *argv[]);
int player_update();
int player_quit();

int
get_options(int argc, char **argv)
{
  int ch=0, errflg=0;
  const char* optflags = "i:h:p:r:b:t:";

  while((ch=getopt(argc, argv, optflags))!=-1)
  {
    switch(ch)
    {
      /* case values must match long_options */
      case 'i':
          g_camera_index = atoi(optarg);
          break;
      case 'h':
          strcpy(g_hostname,optarg);
          break;
      case 'p':
          g_port = atoi(optarg);
          break;
      case 'r':
          g_rate = atoi(optarg);
          break;
      case 't':
          if(!strcasecmp(optarg,"tcp"))
            g_transport = PLAYERC_TRANSPORT_TCP;
          else if(!strcasecmp(optarg,"udp"))
            g_transport = PLAYERC_TRANSPORT_UDP;
          else
          {
            printf("unknown transport \"%s\"", optarg);
            return(-1);
          }
          break;
      case 'b':
          g_blobfinder_index = atoi(optarg);
          break;
      case '?':
      case ':':
      default:
        return (-1);
    }
  }

  return (0);
}

void
print_usage()
{
  printf("\n"
         " playercam - camera test utility for a player camera\n\n"
         "USAGE:  playercam [options] \n\n"
         "Where [options] can be:\n"
         "  -help          : print this message.\n"
         "  -h <hostname>  : host that is running player\n"
         "  -p <port>      : the port number of the host\n"
         "  -i <index>     : the index of the camera\n"
         "  -b <index>     : the index of the blobfinder\n"
         "  -r <rate>      : the refresh rate of the video\n"
         "  -t <transport> : transport to use (either \"tcp\" or \"udp\")\n\n"
         "Currently supports RGB888 and 8/16-bit grey scale images.\n\n");
}

gint
render_camera(gpointer data)
{
  GdkPixbuf *pixbuf = NULL;
  GdkPixbuf *blobbuf= NULL;
  GdkGC         *gc = NULL;
  GtkWidget *drawing_area = GTK_WIDGET(data);
  gc = GTK_WIDGET(drawing_area)->style->fg_gc[GTK_WIDGET_STATE(GTK_WIDGET(drawing_area))];
  uint16_t i;

  player_update();

  if (g_blob_count > 0)
  {
    // Draw the blobs
    blobbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, 1, 1);
    for (i=0; i< g_blob_count; ++i)
    {
      // shift the color by 8-bits to account for the alpha channel
      gdk_pixbuf_fill(blobbuf, g_blobs[i].color << 8);

      gdk_pixbuf_composite(blobbuf,
                           g_pixbuf,
                           g_blobs[i].left,
                           g_blobs[i].top,
                           abs(g_blobs[i].right - g_blobs[i].left),
                           abs(g_blobs[i].top   - g_blobs[i].bottom),
                           1, 1, 1, 1, GDK_INTERP_NEAREST, 128);
    }
    gdk_pixbuf_unref(blobbuf);
  }

  // scale everything at the end
  if ((g_width==g_window_width)&&(g_height==g_window_height))
  { // we don't need to worry about scaling
    gdk_draw_pixbuf(GTK_WIDGET(drawing_area)->window, gc,
                    g_pixbuf, 0, 0, 0, 0, g_width, g_height,
                    GDK_RGB_DITHER_NONE, 0, 0);
  }
  else
  {
    pixbuf = gdk_pixbuf_scale_simple(g_pixbuf, g_window_width,
                                     g_window_height, GDK_INTERP_BILINEAR);
    gdk_draw_pixbuf(GTK_WIDGET(drawing_area)->window, gc,
                    pixbuf, 0, 0, 0, 0, g_window_width, g_window_height,
                    GDK_RGB_DITHER_NONE, 0, 0);
    gdk_pixbuf_unref(pixbuf);
  }

  return TRUE;
}

gint
resize(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GtkAllocation *size = (GtkAllocation*)event;
    g_window_width  = size->width;
    g_window_height = size->height;
    return TRUE;
}

gint
click(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  GdkEventButton *bevent = (GdkEventButton *)event;
  int x,y,o;
  int width, height, rowstride, n_channels;
  guchar *pixels, *p;

  switch ((gint)event->type)
  {
    case GDK_BUTTON_PRESS:
      x = (int)rint(bevent->x*g_width/g_window_width);
      y = (int)rint(bevent->y*g_height/g_window_height);
      g_print("[%i, %i] = ", x, y);

      if (NULL != g_camera)
      {
        switch (g_camera->format)
        {
          case PLAYER_CAMERA_FORMAT_MONO8:
            /// @todo
            // I'm not 100% sure this works
            // need to check when I get time
            g_print("[%i]\n", g_camera->image[x + g_camera->width*y]);
            break;
          case PLAYER_CAMERA_FORMAT_RGB888:
            /// @todo
            // I'm not 100% sure this works
            // need to check when I get time
            o = x*3 + 3*g_camera->width*y;
            g_print("[%i %i %i]",
                    g_camera->image[o],
                    g_camera->image[o+1],
                    g_camera->image[o+2]);
            break;
        }
      }
      g_print("\n");
      return TRUE;
  }
  /* Event not handled; try parent item */
  return FALSE;
}

static
gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  gdk_pixbuf_unref(g_pixbuf);
  return FALSE;
}

static
void destroy(GtkWidget *widget, gpointer data)
{
  gtk_main_quit();
}

int
main(int argc, char *argv[])
{
  GtkWidget *window = NULL;
  GtkWidget *vbox;
  GtkWidget *drawing_area = NULL;

  player_init(argc, argv);
  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "PlayerCam");

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add (GTK_CONTAINER(window), vbox);
  gtk_widget_show(vbox);

  drawing_area = gtk_drawing_area_new();
  gtk_widget_set_size_request(GTK_WIDGET (drawing_area), g_width, g_height);
  gtk_box_pack_start(GTK_BOX (vbox), drawing_area, TRUE, TRUE, 0);
  gtk_widget_show(drawing_area);

  gtk_widget_show_all(window);

  gtk_widget_add_events(GTK_WIDGET(drawing_area), GDK_BUTTON_PRESS_MASK);

  g_signal_connect(G_OBJECT (window), "delete_event",
                    G_CALLBACK (delete_event), NULL);
  g_signal_connect(G_OBJECT (window), "destroy",
                    G_CALLBACK (destroy), NULL);
  g_signal_connect(GTK_OBJECT(drawing_area), "size-allocate",
                    G_CALLBACK(resize), NULL);
  g_signal_connect(GTK_OBJECT(drawing_area), "event",
                    G_CALLBACK(click), NULL);

  g_pixbuf = gdk_pixbuf_new_from_data(g_img, GDK_COLORSPACE_RGB,
                                      FALSE, 8, g_width, g_height,
                                      g_width * 3, NULL, NULL);

  gtk_idle_add(render_camera, drawing_area);

  gtk_main();

  player_quit();
  return 0;
}

int
player_init(int argc, char *argv[])
{
  int csize, usize, i;

  if(get_options(argc, argv) < 0)
  {
    print_usage();
    exit(-1);
  }

  // Create a g_client object and connect to the server; the server must
  // be running on "localhost" at port 6665
  g_client = playerc_client_create(NULL, g_hostname, g_port);
  playerc_client_set_transport(g_client, g_transport);
  if (0 != playerc_client_connect(g_client))
  {
    fprintf(stderr, "error: %s\n", playerc_error_str());
    exit(-1);
  }

/*  if (0 != playerc_client_datafreq(g_client, 20))
  {
    fprintf(stderr, "error: %s\n", playerc_error_str());
    return -1;
  }
*/

  // Create a camera proxy (device id "camera:index") and susbscribe
  g_camera = playerc_camera_create(g_client, g_camera_index);
  if (0 != playerc_camera_subscribe(g_camera, PLAYER_OPEN_MODE))
  {
    fprintf(stderr, "camera error: %s\n", playerc_error_str());
    fprintf(stderr, "playercam will attempt to continue without a camera\n");
    playerc_camera_destroy(g_camera);
    g_camera = NULL;
  }

  // Create a blobfinder proxy (device id "blobfinder:index") and susbscribe
  g_blobfinder = playerc_blobfinder_create(g_client, g_blobfinder_index);
  if (0 != playerc_blobfinder_subscribe(g_blobfinder, PLAYER_OPEN_MODE))
  {
    fprintf(stderr, "blobfinder error: %s\n", playerc_error_str());
    fprintf(stderr, "playercam will attempt to continue without a blobfinder\n");
    playerc_blobfinder_destroy(g_blobfinder);
    g_blobfinder = NULL;
  }

  if ((NULL == g_camera) && (NULL == g_blobfinder))
  {
    fprintf(stderr, "we need either a camera or blobfinder! aborting\n");
    exit(-1);
  }

  // Get up to 10 images until we have a valid frame (g_wdith > 0)
  for (i=0, g_width=0; i < 10 && g_width==0 && NULL != playerc_client_read(g_client); ++i)
  {
    if (NULL != g_camera)
    {
      // Decompress the image
      csize = g_camera->image_count;
      playerc_camera_decompress(g_camera);
      usize = g_camera->image_count;

      g_print("camera: [w %d h %d d %d] [%d/%d bytes]\n",
              g_camera->width, g_camera->height, g_camera->bpp, csize, usize);

      g_width  = g_camera->width;
      g_height = g_camera->height;
      if (allocated_size != usize)
      {
    	  g_img = realloc(g_img, usize);
        allocated_size = usize;
      }
    }
    else // try the blobfinder
    {
      g_print("blobfinder: [w %d h %d]\n",
              g_blobfinder->width, g_blobfinder->height);

      g_width  = g_blobfinder->width;
      g_height = g_blobfinder->height;
      usize = g_width * g_height * 3;
      if (allocated_size != usize)
      {
        g_img = realloc(g_img, usize);
        allocated_size = usize;
      }
      // set the image data to 0 since we don't have a camera
      memset(g_img, 128, usize);
    }
  }

  g_window_width  = g_width;
  g_window_height = g_height;

  assert(g_width>0);
  assert(g_height>0);

  playerc_client_datamode(g_client,PLAYER_DATAMODE_PULL); 
  playerc_client_set_replace_rule(g_client,-1,-1,PLAYER_MSGTYPE_DATA,-1,1);
}

int
player_update()
{
  int i;

  if (NULL != playerc_client_read(g_client))
  {
    if (NULL != g_camera)
    {
      // Decompress the image if necessary
      playerc_camera_decompress(g_camera);
      // figure out the colorspace
      switch (g_camera->format)
      {
        assert(allocated_size > g_camera->image_count*3);
        case PLAYER_CAMERA_FORMAT_MONO8:
          // we should try to use the alpha layer,
          // but for now we need to change
          // the image data
          for (i=0;i<g_camera->image_count;++i)
          {
        
            memcpy(g_img+i*3, g_camera->image+i, 3);
          }
          break;
        case PLAYER_CAMERA_FORMAT_MONO16:
    	{
          int j = 0;
          // Transform to MONO8
          for (i = 0; i < g_camera->image_count; i++, j+=2)
          {
            g_img[i*3+1] = g_img[i*3+2] = g_img[i*3+3] = 
          	  ((unsigned char)(g_camera->image[j]) << 8) + 
          	  (unsigned char)(g_camera->image[j+1]);
          }
          break;
        }
        case PLAYER_CAMERA_FORMAT_RGB888:
          // do nothing
          memcpy(g_img, g_camera->image, g_camera->image_count);
          break;
        default:
          g_print("Unknown camera format: %i\n", g_camera->format);
          exit(-1);
      }
      g_width  = g_camera->width;
      g_height = g_camera->height;
    }

    if (NULL != g_blobfinder)
    {
      g_blob_count = PLAYERCAM_MAX_BLOBS < g_blobfinder->blobs_count ? PLAYERCAM_MAX_BLOBS : g_blobfinder->blobs_count;
      memcpy(g_blobs,
             g_blobfinder->blobs,
             g_blob_count*sizeof(player_blobfinder_blob_t));


      if ((g_width  != g_blobfinder->width) ||
          (g_height != g_blobfinder->height))
      {
        g_print("camera and blobfinder height or width do not match %d,%d != %d,%d\n",g_width,g_height,g_blobfinder->width,g_blobfinder->height);
        // should we die here?
        //exit(-1);
      }
    }
  }
  else
  {
    g_print("ERROR reading player g_client\n");
    //exit(-1);
  }
}

int
player_quit()
{
  if (NULL != g_camera)
  {
    playerc_camera_unsubscribe(g_camera);
    playerc_camera_destroy(g_camera);
  }
  if (NULL != g_blobfinder)
  {
    playerc_blobfinder_unsubscribe(g_blobfinder);
    playerc_blobfinder_destroy(g_blobfinder);
  }
  playerc_client_disconnect(g_client);
  playerc_client_destroy(g_client);
}
