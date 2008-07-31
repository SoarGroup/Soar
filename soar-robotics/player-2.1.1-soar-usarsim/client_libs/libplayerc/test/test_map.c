/***************************************************************************
 * Desc: Tests for the map device
 * Author: Brian Gerkey
 * Date: June 2004
 # CVS: $Id: test_map.c 4121 2007-08-06 06:30:50Z thjc $
 **************************************************************************/

#include <math.h>
#include "test.h"
#include "playerc.h"


// Basic test for map device.
int test_map(playerc_client_t *client, int index)
{
//   int t;
//   void *rdevice;
  playerc_map_t *device;

  printf("device [map] index [%d]\n", index);

  device = playerc_map_create(client, index);

  TEST("subscribing (read)");
  if (playerc_map_subscribe(device, PLAYER_OPEN_MODE) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();

  TEST("reading map");
  if(playerc_map_get_map(device) != 0)
  {
    FAIL();
    return -1;
  }
  printf("read a %d X %d map @ %.3f m/cell\n",
         device->width, device->height, device->resolution);
  PASS();

  TEST("unsubscribing");
  if (playerc_map_unsubscribe(device) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();
  
  playerc_map_destroy(device);
  
  return 0;
}

