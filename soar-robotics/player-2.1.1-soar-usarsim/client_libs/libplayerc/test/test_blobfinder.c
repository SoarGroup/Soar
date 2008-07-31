/***************************************************************************
 * Desc: Tests for the vision device
 * Author: Andrew Howard
 * Date: 23 May 2002
 # CVS: $Id: test_blobfinder.c 3041 2005-09-05 14:20:27Z bradkratochvil $
 **************************************************************************/

#include "test.h"
#include "playerc.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

// Basic blobfinder test
int test_blobfinder(playerc_client_t *client, int index)
{
  int t, i;
  void *rdevice;
  playerc_blobfinder_t *device;

  printf("device [blobfinder] index [%d]\n", index);

  device = playerc_blobfinder_create(client, index);

  TEST("subscribing (read)");
  if (playerc_blobfinder_subscribe(device, PLAYER_OPEN_MODE) == 0)
    PASS();
  else
    FAIL();

  for (t = 0; t < 10; t++)
  {
    TEST1("reading data (attempt %d)", t);

    do
      rdevice = playerc_client_read(client);
    while (rdevice == client);

    if (rdevice == device)
    {
      PASS();
      printf("blobfinder: [%d, %d] [%d] ", device->width, device->height, device->blobs_count);
      for (i = 0; i < min(3, device->blobs_count); i++)
        printf("[%d : (%d %d) (%d %d %d %d) : %d] ", device->blobs[i].id,
               device->blobs[i].x, device->blobs[i].y,
               device->blobs[i].left, device->blobs[i].top,
               device->blobs[i].right, device->blobs[i].bottom,
               device->blobs[i].area);
      printf("\n");
    }
    else
    {
      FAIL();
      break;
    }
  }

  TEST("unsubscribing");
  if (playerc_blobfinder_unsubscribe(device) == 0)
    PASS();
  else
    FAIL();

  playerc_blobfinder_destroy(device);

  return 0;
}


