/***************************************************************************
 * Desc: Tests for the WSN device
 * Author: Radu Bogdan Rusu
 * Date: 30 March 2006
 **************************************************************************/

#include "test.h"
#include "playerc.h"

// Basic WSN test
int test_wsn(playerc_client_t *client, int index)
{
  int t;//, i, j;
  void *rdevice;
  playerc_wsn_t *device;

  printf("device [wsn] index [%d]\n", index);

  device = playerc_wsn_create(client, index);

  TEST("subscribing (read)");
  if (playerc_wsn_subscribe(device, PLAYER_OPEN_MODE) == 0)
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
      printf("Node type: %d, with ID %d and parent %d holds:\n"
		"accel_{X,Y,Z}  : [%f,%f,%f]\n"
		"magn_{X,Y,Z}   : [%f,%f,%f]\n"
		"temperature    : [%f]\n"
		"light          : [%f]\n"
		"microphone     : [%f]\n"
		"battery voltage: [%f]\n", 
		device->node_type, device->node_id, device->node_parent_id,
		device->data_packet.accel_x, device->data_packet.accel_y, 
		device->data_packet.accel_z, device->data_packet.magn_x,
		device->data_packet.magn_y, device->data_packet.magn_z,
		device->data_packet.temperature, device->data_packet.light, 
		device->data_packet.mic, device->data_packet.battery);
    }
    else
    {
      FAIL();
      break;
    }
  }
  
  TEST("setting the data frequency rate");
  if(playerc_wsn_datafreq(device, -1, -1, 1) < 0)
    FAIL();
  else
  {
    sleep(3);
    PASS();
  }
  
  TEST("enabling all LEDs");
  if(playerc_wsn_set_devstate(device, -1, -1, 3, 7) < 0)
    FAIL();
  else
  {
    sleep(3);
    PASS();
  }
  
/*  TEST("going to sleep");
  if(playerc_wsn_power(device, 2, -1, 0) < 0)
    FAIL();
  else
  {
    sleep(3);
    PASS();
  }
  
  TEST("waking up");
  if(playerc_wsn_power(device, 1, -1, 1) < 0)
    FAIL();
  else
  {
    sleep(3);
    PASS();
  }
*/  

  TEST("unsubscribing");
  if (playerc_wsn_unsubscribe(device) == 0)
    PASS();
  else
    FAIL();
  
  playerc_wsn_destroy(device);
  
  return 0;
}
