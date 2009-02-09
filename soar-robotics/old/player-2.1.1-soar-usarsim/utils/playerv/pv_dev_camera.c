/* 
 *  PlayerViewer
 *  Copyright (C) Andrew Howard 2002
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
/***************************************************************************
 * Desc: Camera interface
 * Author: Kevin Barry (Based off Andrew Howard's blobfinder)
 * Date: 19 Nov 2007
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"


// Update the blobfinder configuration
//void blobfinder_update_config(blobfinder_t *blobfinder);


void flip_image(unsigned char *dst, unsigned char *src, int width, int height, int bpp, int usize);

// Draw the camera scan
void camera_draw(camera_t *camera);


// Create a camera device
camera_t *camera_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                                int index, const char *drivername, int subscribe)
{
  char section[64];
  char label[64];
  camera_t *camera;
  
  camera = malloc(sizeof(camera_t));
  camera->datatime = 0;
  camera->drivername = strdup(drivername);
  camera->proxy = playerc_camera_create(client, index);
  
  // Construct the menu
  snprintf(label, sizeof(label), "camera:%d (%s)", index, camera->drivername);
  camera->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  camera->subscribe_item = rtk_menuitem_create(camera->menu, "Subscribe", 1);
  camera->stats_item = rtk_menuitem_create(camera->menu, "Show stats", 1);
  camera->scale_item = rtk_menuitem_create(camera->menu, "Scale Image", 1);

  snprintf(section, sizeof(section), "camera:%d", index);

  // Set the initial menu state
  rtk_menuitem_check(camera->subscribe_item, subscribe);
  rtk_menuitem_check(camera->stats_item, 0);
  
  // Default scale for drawing the image (m/pixel)
  camera->scale = 0.01;
    
  // Construct figures
  camera->image_init = 0;
  camera->allocated_size = 0;
  camera->image_fig = rtk_fig_create(mainwnd->canvas, NULL, 99);
  rtk_fig_movemask(camera->image_fig, RTK_MOVE_TRANS);

  return camera;
}


// Destroy a camera device
void camera_destroy(camera_t *camera)
{
  // Destroy figures
  rtk_fig_destroy(camera->image_fig);

  // Destroy menu items
  rtk_menuitem_destroy(camera->stats_item);
  rtk_menuitem_destroy(camera->subscribe_item);
  rtk_menu_destroy(camera->menu);

  // Unsubscribe/destroy the proxy
  if (camera->proxy->info.subscribed)
    playerc_camera_unsubscribe(camera->proxy);
  playerc_camera_destroy(camera->proxy);

  free(camera->drivername);
  free(camera);
}


// Update a camera device
void camera_update(camera_t *camera)
{
  // Update the device subscription
  if (rtk_menuitem_ischecked(camera->subscribe_item))
  {
    if (!camera->proxy->info.subscribed)
      if (playerc_camera_subscribe(camera->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  else
  {
    if (camera->proxy->info.subscribed)
      if (playerc_camera_unsubscribe(camera->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(camera->subscribe_item, camera->proxy->info.subscribed);

  // Draw in the camera scan if it has been changed.
  if (camera->proxy->info.subscribed)
  {
    if (camera->proxy->info.datatime != camera->datatime) {
      playerc_camera_decompress(camera->proxy);
      if(camera->allocated_size != camera->proxy->image_count) {
	camera->img_buffer = realloc(camera->img_buffer, camera->proxy->image_count);
	camera->allocated_size = camera->proxy->image_count;
      }
	
      flip_image(camera->img_buffer, camera->proxy->image, camera->proxy->width, 
		camera->proxy->height, camera->proxy->bpp, camera->proxy->image_count);
      camera_draw(camera);
    }
    camera->datatime = camera->proxy->info.datatime;
  }
  else
  {
    rtk_fig_show(camera->image_fig, 0);
    camera->datatime = 0;
  }
}

#define PX(ix) ((ix - camera->proxy->width/2) * camera->scale)
#define PY(iy) ((camera->proxy->height/2 - iy) * camera->scale)
#define DX(ix) ((ix) * camera->scale)
#define DY(iy) ((iy) * camera->scale)

// Draw the camera scan
void camera_draw(camera_t *camera)
{
  int i;
  char text[64];
  double ox, oy, dx, dy;
  int sizex, sizey;
  double scalex, scaley;


  rtk_fig_show(camera->image_fig, 1);
  rtk_fig_clear(camera->image_fig);

  // Set the initial pose of the image if it hasnt already been set.
  if (camera->image_init == 0)
  {
    rtk_canvas_get_size(camera->image_fig->canvas, &sizex, &sizey);
    rtk_canvas_get_scale(camera->image_fig->canvas, &scalex, &scaley);
    rtk_fig_origin(camera->image_fig, 0, -sizey * scaley / 4, 0 );
    camera->image_init = 1;
  }

  if (rtk_menuitem_ischecked(camera->scale_item))
	camera->scale = .01;
  else {
  	rtk_canvas_get_scale(camera->image_fig->canvas, &scalex, &scaley);
	camera->scale = scalex;
  }

  // Draw an opaque rectangle on which to render the image.
  rtk_fig_color_rgb32(camera->image_fig, 0x000000);
  rtk_fig_rectangle(camera->image_fig, 0, 0, 0,
                    DX(camera->proxy->width), DY(camera->proxy->height), 0);

  rtk_fig_image(camera->image_fig, 0, 0, 0, camera->scale, camera->proxy->width, camera->proxy->height, camera->proxy->bpp, camera->img_buffer, NULL);


  if (rtk_menuitem_ischecked(camera->stats_item))
  {
     snprintf(text, sizeof(text), "%dx%d %dbpp", camera->proxy->width, camera->proxy->height,
				 camera->proxy->bpp);
     rtk_fig_text(camera->image_fig, PX(10), PY(camera->proxy->height + 10), 0, text);
  }
    
}

void flip_image(unsigned char *dst, unsigned char *src, int width, int height, int bpp, int usize)
{
	/* ERROR CHECK BPP!! */
	/* if i need to flip just the rows */
	int r, row_width;

	if(bpp % 8 != 0) {
		/* Something is wrong. I don't know how to flip this. Sucks to be you */
		memcpy(dst, src, usize);
		return;
	}

	row_width = width * bpp/8;
	for(r = height; r >= 0; r--) {
		memcpy(dst, src + r*row_width, row_width);
		dst = dst + row_width;
	}
}
