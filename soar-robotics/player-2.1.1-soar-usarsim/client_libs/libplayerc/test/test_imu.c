/***************************************************************************
 * Desc: Tests for the IMU device
 * Author: Radu Bogdan Rusu
 * Date: 08th of September 2006
 **************************************************************************/

#include "test.h"
#include "playerc.h"

// Basic IMU test
int test_imu (playerc_client_t *client, int index)
{
  int t;
  void *rdevice;
  playerc_imu_t *device;

  printf ("device [imu] index [%d]\n", index);

  device = playerc_imu_create (client, index);

  TEST ("subscribing (read)");
  if (playerc_imu_subscribe (device, PLAYER_OPEN_MODE) == 0)
    PASS ();
  else
    FAIL ();

  for (t = 0; t < 10; t++)
  {
    TEST1 ("reading data (attempt %d)", t);

    do
      rdevice = playerc_client_read (client);
    while (rdevice == client);
    
    if (rdevice == device)
    {
      PASS ();
      printf ("roll  : [%f]\n"
	      "pitch : [%f]\n"
	      "yaw   : [%f]\n", 
	    device->pose.proll, device->pose.ppitch, device->pose.pyaw);
    }
    else
    {
      FAIL ();
      break;
    }
  }

  TEST ("resetting the orientation (global reset)...");
  if (playerc_imu_reset_orientation (device, 2) < 0)
    FAIL ();
  else
  {
    usleep(3000000);
    PASS ();
  }
  
  for (t = 0; t < 10; t++)
  {
    TEST1 ("reading data (attempt %d)", t);

    do
      rdevice = playerc_client_read (client);
    while (rdevice == client);
    
    if (rdevice == device)
    {
      PASS ();
      printf ("roll  : [%f]\n"
	      "pitch : [%f]\n"
	      "yaw   : [%f]\n", 
	    device->pose.proll, device->pose.ppitch, device->pose.pyaw);
    }
    else
    {
      FAIL ();
      break;
    }
  }
  
  TEST("unsubscribing");
  if (playerc_imu_unsubscribe (device) == 0)
    PASS ();
  else
    FAIL ();
  
  playerc_imu_destroy (device);
  
  return 0;
}
