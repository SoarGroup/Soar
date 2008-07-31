/***************************************************************************
 * Desc: Tests for the PTZ device
 * Author: Andrew Howard
 * Date: 23 May 2002
 # CVS: $Id: test_ptz.c 3041 2005-09-05 14:20:27Z bradkratochvil $
 **************************************************************************/

#include <math.h>
#include "test.h"
#include "playerc.h"


// Basic ptz test
int test_ptz(playerc_client_t *client, int index)
{
  int t;
  void *rdevice;
  playerc_ptz_t *device;
  double period;

  printf("device [ptz] index [%d]\n", index);

  device = playerc_ptz_create(client, index);

  TEST("subscribing (read)");
  if (0 == playerc_ptz_subscribe(device, PLAYER_OPEN_MODE))
    PASS();
  else
    FAIL();

  period = 10 / M_PI * 2;

  for (t = 0; t < 20; t++)
  {
    TEST1("reading data (attempt %d)", t);

    do
      rdevice = playerc_client_read(client);
    while (rdevice == client);

    if (rdevice == device)
    {
      PASS();
      printf("ptz: [%d %d %d]\n",
             (int) (device->pan * 180 / M_PI),
             (int) (device->tilt * 180 / M_PI),
             (int) (device->zoom * 180 / M_PI));
    }
    else
    {
      FAIL();
      break;
    }

    TEST1("writing data (attempt %d)", t);
    if (playerc_ptz_set(device,
                        sin(t / period) * M_PI / 2,
                        sin(t / period) * M_PI / 3,
                        (1 - t / 20.0) * M_PI) != 0)
    {
      FAIL();
      break;
    }
    PASS();
  }

  TEST1("writing data (attempt %d)", t);
  if (playerc_ptz_set(device, 0, 0, M_PI) != 0)
    FAIL();
  else
    PASS();

  TEST("unsubscribing");
  if (playerc_ptz_unsubscribe(device) == 0)
    PASS();
  else
    FAIL();

  playerc_ptz_destroy(device);

  return 0;
}


