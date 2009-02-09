/***************************************************************************
 * Desc: Tests for the RFID device
 * Author: Radu Bogdan Rusu
 * Date: 31 January 2006
 **************************************************************************/

#include "test.h"
#include "playerc.h"

// Basic RFID test
int test_rfid(playerc_client_t *client, int index)
{
  int t, i, j;
  void *rdevice;
  playerc_rfid_t *device;

  printf("device [rfid] index [%d]\n", index);

  device = playerc_rfid_create(client, index);

  TEST("subscribing (read)");
  if (playerc_rfid_subscribe(device, PLAYER_OPEN_MODE) == 0)
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
      printf("rfid tags count: [%d] \n", device->tags_count);
      for (i = 0; i < device->tags_count; i++)
      {
        for (j = 0; j < device->tags[i].guid_count; j++)
    	    printf("[%2x] ", device->tags[i].guid[j]);
        printf ("\n");
      }
      printf("\n");
    }
    else
    {
      FAIL();
      break;
    }
  }
  
  TEST("unsubscribing");
  if (playerc_rfid_unsubscribe(device) == 0)
    PASS();
  else
    FAIL();
  
  playerc_rfid_destroy(device);
  
  return 0;
}
