#include <stdio.h>
//#include <stdint.h>
#include <libplayercore/playerconfig.h>

#include <libplayerc/playerc.h>

#include "sharedstruct.h"

int
main(int argc, const char **argv)
{
  playerc_client_t *client;
  playerc_opaque_t *opaque;
  test_t t;
  int i;

  // Create a client and connect it to the server.
  client = playerc_client_create(NULL, "localhost", 6665);
  if (0 != playerc_client_connect(client))
    return -1;

  // Create and subscribe to a opaque device.
  opaque = playerc_opaque_create(client, 0);
  if (playerc_opaque_subscribe(opaque, PLAYER_OPEN_MODE))
    return -1;


  for (i=0; i<10; ++i)
  {
    // Wait for new data from server
    playerc_client_read(client);

    t = *((test_t*)opaque->data);

    printf("test data %i\n", i);
    printf("%i\n", t.uint8);
    printf("%i\n", t.int8);
    printf("%i\n", t.uint16);
    printf("%i\n", t.int16);
    printf("%i\n", t.uint32);
    printf("%i\n", t.int32);
    printf("%0.3f\n", t.doub);
  }

  // Shutdown
  playerc_opaque_unsubscribe(opaque);
  playerc_opaque_destroy(opaque);
  playerc_client_disconnect(client);
  playerc_client_destroy(client);

  return 0;
}
