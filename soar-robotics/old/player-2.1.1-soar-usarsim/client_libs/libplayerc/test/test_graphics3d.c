/***************************************************************************
 * Desc: Tests for the graphics3d device
 * Author: Richard Vaughan
 * Date: 15 June 2007
 # CVS: $Id: test_graphics3d.c 6566 2008-06-14 01:00:19Z thjc $
 **************************************************************************/

#include <unistd.h>
#include <math.h>

#include "test.h"
#include "playerc.h"

#define RAYS 64

// Basic test for graphics3d device.
int test_graphics3d(playerc_client_t *client, int index)
{
/*  int t;
  void *rdevice;*/
  playerc_graphics3d_t *device;
  player_color_t col;
  player_point_3d_t pts[RAYS];
  player_point_3d_t pt;
  int p;
  double r;

  printf("device [graphics3d] index [%d]\n", index);

  device = playerc_graphics3d_create(client, index);

  TEST("subscribing (read/write)");
  if (playerc_graphics3d_subscribe(device, PLAYER_OPEN_MODE) < 0)
  {
    FAIL();
    return -1;
  }
  PASS();

  TEST("changing color");
  col.red = 0;
  col.green = 0;
  col.blue = 255;
  col.alpha = 255;

  if(playerc_graphics3d_setcolor(device, col) < 0)
    FAIL();
  else
    PASS();

  for( r=0; r<1.0; r+=0.05 )
    {
      TEST("drawing line loop");

      for( p=0; p<RAYS; p++ )
	{
	  pts[p].px = 5 * r * cos(p * M_PI/(RAYS/2));
	  pts[p].py = 5 * r * sin(p * M_PI/(RAYS/2));
	  pts[p].pz = 5 * r;

	  printf( "vertex [%.2f,%.2f,%.2f]\n",
		  pts[p].px,
		  pts[p].py,
		  pts[p].pz );
	}	
      
      if( playerc_graphics3d_draw(device, 
				  PLAYER_DRAW_LINE_LOOP, 
				  pts, RAYS) < 0)
	FAIL();
      else
	PASS();

      usleep(50000);
    }
  
  TEST("changing color");
  col.alpha = 60;

  if(playerc_graphics3d_setcolor(device, col) < 0)
    FAIL();
  else
    PASS();

  TEST("drawing polygon");
  
  if(playerc_graphics3d_draw(device, 
			     PLAYER_DRAW_POLYGON,
			     pts, RAYS) < 0)
    FAIL();
  else
    PASS();
  
  sleep(1);
  
  TEST("changing color");
  col.red = 0;//random() % 255;
  col.green = 255;//random() % 255;
  col.blue = 0;//random() % 255;
  col.alpha = 255;
  
  if(playerc_graphics3d_setcolor(device, col) < 0)
    FAIL();
  else
    PASS();
  
  for( r=0; r<300; r++ )
    {
      pt.px = fmod( rand(), 100 ) / 50.0 - 1.0;
      pt.py = fmod( rand(), 100 ) / 50.0 - 1.0;
      pt.pz = fmod( rand(), 100 ) / 30;
      
      if( playerc_graphics3d_draw(device, 
				  PLAYER_DRAW_POINTS, 
				  &pt, 1) < 0)
	FAIL();
      else
	PASS();
    }

  sleep(2);
  
  TEST("clearing");
  if(playerc_graphics3d_clear(device) < 0)
    FAIL();
  else
    PASS();
  
  sleep( 1 );
  
  TEST( "translating" );
  if(playerc_graphics3d_translate( device, 1, 1, 1 ) < 0)
    FAIL();
  else
    PASS();
  
  TEST( "rotating" );
  if(playerc_graphics3d_rotate( device, M_PI/2.0, 0, 0, 1 ) < 0)
    FAIL();
  else
    PASS();
    

  /*  TEST("clearing");
  
  if(playerc_graphics3d_clear(device) < 0)
    FAIL();
  else
    PASS();

  sleep( 1 );
  */

  

  TEST("unsubscribing");
  if (playerc_graphics3d_unsubscribe(device) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();
  
  playerc_graphics3d_destroy(device);
  
  return 0;
}

