/***************************************************************************
 * Desc: Tests for the graphics2d device
 * Author: Richard Vaughan
 * Date: 8 Feb 2006
 # CVS: $Id: test_graphics2d.c 6566 2008-06-14 01:00:19Z thjc $
 **************************************************************************/

#include <unistd.h>
#include <math.h>

#include "test.h"
#include "playerc.h"

#define RAYS 64

// Basic test for graphics2d device.
int test_graphics2d(playerc_client_t *client, int index)
{
/*  int t;
  void *rdevice;*/
  playerc_graphics2d_t *device;
  int p;
  double r;
  player_point_2d_t pts[RAYS];
  player_color_t col;

  printf("device [graphics2d] index [%d]\n", index);

  device = playerc_graphics2d_create(client, index);

  TEST("subscribing (read/write)");
  if (playerc_graphics2d_subscribe(device, PLAYER_OPEN_MODE) < 0)
  {
    FAIL();
    return -1;
  }
  PASS();

  for( r=0; r<1.0; r+=0.05 )
    {
      TEST("drawing points");

      for( p=0; p<RAYS; p++ )
	{
	  pts[p].px = r * cos(p * M_PI/(RAYS/2));
	  pts[p].py = r * sin(p * M_PI/(RAYS/2));
	}	
      
      if(playerc_graphics2d_draw_points(device, pts, RAYS) < 0)
	FAIL();
      else
	PASS();

      usleep(100000);
    }
  
  TEST("changing color");
  col.red = 0;
  col.green = 255;
  col.blue = 0;
  col.alpha = 0;

  if(playerc_graphics2d_setcolor(device, col) < 0)
    FAIL();
  else
    PASS();

  TEST("drawing polyline");
  
  if(playerc_graphics2d_draw_polyline(device, pts, RAYS) < 0)
    FAIL();
  else
    PASS();
  
  usleep(500000);

  TEST("changing color");      
  col.red = 0;
  col.green = 128;
  col.blue = 255;
  col.alpha = 0;

  if(playerc_graphics2d_setcolor(device, col) < 0)
    FAIL();
  else
    PASS();



  for( r=1.0; r>0; r-=0.1 )
    {
      TEST("drawing polygon");
      pts[0].px = -r;
      pts[0].py = -r;
      pts[1].px = r;
      pts[1].py = -r;
      pts[2].px = r;
      pts[2].py = r;
      pts[3].px = -r;
      pts[3].py = r;
      
      if(playerc_graphics2d_draw_polygon(device, pts, 4, 0, col) < 0)
	FAIL();
      else
	PASS();
      
      usleep(100000);
    }

  sleep(2);


  TEST("clearing");
  
  if(playerc_graphics2d_clear(device) < 0)
    FAIL();
  else
    PASS();


  TEST("unsubscribing");
  if (playerc_graphics2d_unsubscribe(device) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();
  
  playerc_graphics2d_destroy(device);
  
  return 0;
}

