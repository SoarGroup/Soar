/***************************************************************************
 * Desc: Tests for the aio device
 * Author: Alexis Maldonado
 * Date: 3 May 2007
 **************************************************************************/

#include <unistd.h>

#include "test.h"
#include "playerc.h"


// Just read from a aio device.
int test_aio(playerc_client_t *client, int index)
{
  int t;
  void *rdevice;
  playerc_aio_t *device;

  printf("device [aio] index [%d]\n", index);

  device = playerc_aio_create(client, index);

  TEST("subscribing (read/write)");
  if (playerc_aio_subscribe(device, PLAYER_OPEN_MODE) < 0)
  {
    FAIL();
    return -1;
  }
  PASS();

  for (t = 0; t < 5; t++)
  {
    TEST1("reading data (attempt %d)", t);

    do
      rdevice = playerc_client_read(client);
    while (rdevice == client);

    if (rdevice == device)
    {
      PASS();
      printf("aio: [%8.3f]  AI0,...,AI7: [%5.3f] [%5.3f] [%5.3f] [%5.3f] [%5.3f] [%5.3f] [%5.3f] [%5.3f]\n",
             device->info.datatime, device->voltages[0], device->voltages[1],device->voltages[2],device->voltages[3],device->voltages[4],device->voltages[5],device->voltages[6],device->voltages[7]);
    }
    else
    {
      //printf("error: %s", playerc_error_str());
      FAIL();
      break;
    }
  }

  TEST("unsubscribing");
  if (playerc_aio_unsubscribe(device) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();
  
  playerc_aio_destroy(device);
  
  return 0;
}

