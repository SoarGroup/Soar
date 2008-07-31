/***************************************************************************
 * Desc: Tests for the position3d device
 * Author: Andrew Howard
 * Date: 23 May 2002
 # CVS: $Id: test_position3d.c 3022 2005-09-01 06:23:32Z inspectorg $
**************************************************************************/

#include <math.h>

#include "test.h"
#include "playerc.h"


// Basic test for position3d device.
int test_position3d(playerc_client_t *client, int index)
{
  int t;
  void *rdevice;
  playerc_position3d_t *device;

  printf("device [position3d] index [%d]\n", index);

  device = playerc_position3d_create(client, index);

  TEST("subscribing (read/write)");
  if (playerc_position3d_subscribe(device, PLAYER_OPEN_MODE) < 0)
  {
    FAIL();
    return -1;
  }
  PASS();

#if 0 // TODO  
  TEST("getting geometry");
  if (playerc_position3d_get_geom(device) == 0)
    PASS();
  else
    FAIL();
  printf("position3d geom: [%6.3f %6.3f %6.3f] [%6.3f %6.3f]\n",
         device->pose[0], device->pose[1], device->pose[2], device->size[0], device->size[1]);

  TEST("enabling motors");
  if (playerc_position3d_enable(device, 1) == 0)
    PASS();
  else
    FAIL();
#endif
  
  for (t = 0; t < 300; t++)
  {
    TEST1("reading data (attempt %d)", t);

    do
      rdevice = playerc_client_read(client);
    while (rdevice == client);

    if (rdevice == device)
    {
      PASS();
      printf("position3d: [%14.3f] [%+7.3f %+7.3f %+7.3f] [%+7.3f %+7.3f %+7.3f]\n",
             device->info.datatime,
             device->pos_x, device->pos_y, device->pos_z,
             device->pos_roll * 180/M_PI,
             device->pos_pitch * 180/M_PI,
             device->pos_yaw * 180/M_PI);
    }
    else
    {
      FAIL();
      break;
    }

#if 0 // TODO
    TEST1("writing data (attempt %d)", t);
    if (playerc_position3d_set_velocity(device, 0.10, 0, 0, 0, 0, 0.2, 1) != 0)
    {
      FAIL();
      break;
    }
    PASS();
#endif
  }

#if 0 // TODO
  TEST1("writing data (attempt %d)", t);
  if (playerc_position3d_set_velocity(device, 0, 0, 0, 0, 0, 0, 1) != 0)
    FAIL();
  else
    PASS();

  TEST("disabling motors");
  if (playerc_position3d_enable(device, 0) == 0)
    PASS();
  else
    FAIL();

  TEST("unsubscribing");
  if (playerc_position3d_unsubscribe(device) != 0)
    FAIL();
  else
    PASS();
#endif
  
  playerc_position3d_destroy(device);
  
  return 0;
}

