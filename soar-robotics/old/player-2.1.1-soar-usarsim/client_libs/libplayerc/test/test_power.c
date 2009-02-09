/***************************************************************************
 * Desc: Tests for the power device
 * Author: Andrew Howard
 * Date: 26 May 2002
 # CVS: $Id: test_power.c 3001 2005-08-29 19:55:45Z gerkey $
 **************************************************************************/

#include "test.h"
#include "playerc.h"


// Basic test for power device.
int test_power(playerc_client_t *client, int index)
{
  int t;
  void *rdevice;
  playerc_power_t *device;

  printf("device [power] index [%d]\n", index);

  device = playerc_power_create(client, index);

  TEST("subscribing (read)");
  if (playerc_power_subscribe(device, PLAYER_OPEN_MODE) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();

  for (t = 0; t < 3; t++)
  {
    TEST1("reading data (attempt %d)", t);

    do
      rdevice = playerc_client_read(client);
    while (rdevice == client);

    if (rdevice == device)
    {
      PASS();
      printf("power: [%6.1f] [%6.1f%% full]\n",
             device->charge, device->percent);
    }
    else
      FAIL();
  }
  
  TEST("unsubscribing");
  if (playerc_power_unsubscribe(device) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();
  
  playerc_power_destroy(device);
  
  return 0;
}

