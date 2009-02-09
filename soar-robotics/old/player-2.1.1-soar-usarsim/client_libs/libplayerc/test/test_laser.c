/***************************************************************************
 * Desc: Tests for the laser device
 * Author: Andrew Howard
 * Date: 23 May 2002
 # CVS: $Id: test_laser.c 4152 2007-09-17 02:18:59Z thjc $
 **************************************************************************/

#include <math.h>
#include "test.h"
#include "playerc.h"


// Basic laser test
int test_laser(playerc_client_t *client, int index)
{
  int t, i;
  void *rdevice;
  playerc_laser_t *device;

  double min, max, scanning_frequency, resolution, range_res;
  unsigned char intensity;

  printf("device [laser] index [%d]\n", index);

  device = playerc_laser_create(client, index);

  TEST("subscribing (read)");
  if (playerc_laser_subscribe(device, PLAYER_OPEN_MODE) == 0)
    PASS();
  else
  {
    FAIL();
    return(-1);
  }

  TEST("set configuration");
  min = -M_PI/2;
  max = +M_PI/2;
  resolution = 100;
  range_res = 1;
  intensity = 1;
  if (playerc_laser_set_config(device, min, max, resolution, range_res, intensity, scanning_frequency) == 0)
    PASS();
  else
    FAIL();

  TEST("get configuration");
  if (playerc_laser_get_config(device, &min, &max, &resolution, &range_res, &intensity, &scanning_frequency) == 0)
    PASS();
  else
    FAIL();

  TEST("check configuration sanity");
  if (abs(min + M_PI/2) > 0.01 || abs(max - M_PI/2) > 0.01)
    FAIL();
  else if (resolution != 100 || intensity != 1)
    FAIL();
  else
    PASS();
  
  TEST("getting geometry");
  if (playerc_laser_get_geom(device) == 0)
    PASS();
  else
    FAIL();
  printf("laser geom: [%6.3f %6.3f %6.3f] [%6.3f %6.3f]\n",
         device->pose[0], device->pose[1], device->pose[2], device->size[0], device->size[1]);
  for (t = 0; t < 10; t++)
  {
    TEST1("reading data (attempt %d)", t);

    do
      rdevice = playerc_client_read(client);
    while (rdevice == client);
    
    if (rdevice == device)
    {
      PASS();
      printf("laser: [%14.3f] [%d] ", device->info.datatime, device->scan_count);
      for (i = 0; i < 3; i++)
        printf("[%6.3f, %6.3f] ", device->scan[i][0], device->scan[i][1]);
      printf("\n");
    }
    else
    {
      FAIL();
      break;
    }
  }
  
  TEST("unsubscribing");
  if (playerc_laser_unsubscribe(device) == 0)
    PASS();
  else
    FAIL();
  
  playerc_laser_destroy(device);
  
  return 0;
}


