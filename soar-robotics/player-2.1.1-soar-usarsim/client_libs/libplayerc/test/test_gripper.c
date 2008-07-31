/***************************************************************************
 * Desc: Tests for the gripper device
 * Author: Richard Vaughan, based on Andrew Howard's test_laser.c
 * Date: 9 October 2005
 # CVS: $Id: test_gripper.c 3768 2006-07-10 16:55:38Z gerkey $
 **************************************************************************/

#include <unistd.h>

#include "test.h"
#include "playerc.h"

int test_gripper(playerc_client_t *client, int index)
{
  int t;
  void *rdevice;
  playerc_gripper_t *device;

  printf("device [gripper] index [%d]\n", index);

  device = playerc_gripper_create(client, index);

  TEST("subscribing (read/write)");
  if (playerc_gripper_subscribe(device, PLAYER_OPEN_MODE) < 0)
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
      playerc_gripper_printout( device, "gripper" );
    }
    else
    {
      //printf("error: %s", playerc_error_str());
      FAIL();
      break;
    }
  }

  TEST("closing gripper");
  if(playerc_gripper_close_cmd(device) < 0)
    FAIL();
  else
  {
    sleep(3);

    do
      rdevice = playerc_client_read(client);
    while (rdevice == client);

    playerc_gripper_printout( device, "gripper" );

    PASS();
  }


  TEST("opening gripper");
  if(playerc_gripper_open_cmd(device) < 0)
    FAIL();
  else
  {
    sleep(3);

    do
      rdevice = playerc_client_read(client);
    while (rdevice == client);

    playerc_gripper_printout( device, "gripper" );

    PASS();
  }


  TEST("unsubscribing");
  if (playerc_gripper_unsubscribe(device) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();

  playerc_gripper_destroy(device);

  return 0;
}

