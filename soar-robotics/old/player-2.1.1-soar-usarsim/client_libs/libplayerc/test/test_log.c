/***************************************************************************
 * Desc: Tests for the log device
 * Author: Brian Gerkey
 * Date: June 2004
 # CVS: $Id: test_log.c 4121 2007-08-06 06:30:50Z thjc $
 **************************************************************************/

#include <math.h>
#include "test.h"
#include "playerc.h"


// Basic test for log device.
int test_log(playerc_client_t *client, int index)
{
  int t;
//   int isplayback=0;
  playerc_log_t *device;

  printf("device [log] index [%d]\n", index);

  device = playerc_log_create(client, index);

  TEST("subscribing (read)");
  if (playerc_log_subscribe(device, PLAYER_OPEN_MODE) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();

  TEST("getting log state");
  if(playerc_log_get_state(device) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();

  if(device->type == PLAYER_LOG_TYPE_WRITE)
  {
    TEST("starting logging");
    if(playerc_log_set_write_state(device,1) != 0)
    {
      FAIL();
      return -1;
    }
    PASS();
  }
  else
  {
    TEST("rewinding logfile");
    if(playerc_log_set_read_rewind(device) != 0)
    {
      FAIL();
      return -1;
    }
    PASS();

    TEST("starting playback");
    if(playerc_log_set_read_state(device,1) != 0)
    {
      FAIL();
      return -1;
    }
    PASS();
  }

  TEST("getting log state");
  if(playerc_log_get_state(device) != 0)
  {
    FAIL();
    return -1;
  }
  if(device->state != 1)
  {
    FAIL();
    return -1;
  }
  PASS();

  // let it log/playback
  TEST("logging/playback proceeding");
  for(t=0;t<50;t++)
  {
    if(!playerc_client_read(client))
    {
      FAIL();
      return -1;
    }
  }
  PASS();

  if(device->type == PLAYER_LOG_TYPE_WRITE)
  {
    TEST("stopping logging");
    if(playerc_log_set_write_state(device,0) != 0)
    {
      FAIL();
      return -1;
    }
    PASS();
  }
  else
  {
    TEST("stopping playback");
    if(playerc_log_set_read_state(device,0) != 0)
    {
      FAIL();
      return -1;
    }
    PASS();
  }

  TEST("getting log state");
  if(playerc_log_get_state(device) != 0)
  {
    FAIL();
    return -1;
  }
  if(device->state != 0)
  {
    FAIL();
    return -1;
  }
  PASS();

  TEST("unsubscribing");
  if (playerc_log_unsubscribe(device) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();
  
  playerc_log_destroy(device);
  
  return 0;
}

