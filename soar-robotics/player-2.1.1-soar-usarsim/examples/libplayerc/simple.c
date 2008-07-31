#include <stdio.h>

#include <libplayerc/playerc.h>

int
main(int argc, const char **argv)
{
  int i;
  playerc_client_t *client;
  playerc_position2d_t *position2d;

  // Create a client and connect it to the server.
  client = playerc_client_create(NULL, "localhost", 6665);
  if (0 != playerc_client_connect(client))
    return -1;

  // Create and subscribe to a position2d device.
  position2d = playerc_position2d_create(client, 0);
  if (playerc_position2d_subscribe(position2d, PLAYER_OPEN_MODE))
    return -1;

  // Make the robot spin!
  if (0 != playerc_position2d_set_cmd_vel(position2d, 0, 0, DTOR(40.0), 1))
    return -1;

  for (i = 0; i < 200; i++)
  {
    // Wait for new data from server
    playerc_client_read(client);

    // Print current robot pose
    printf("position2d : %f %f %f\n",
           position2d->px, position2d->py, position2d->pa);
  }

  // Shutdown
  playerc_position2d_unsubscribe(position2d);
  playerc_position2d_destroy(position2d);
  playerc_client_disconnect(client);
  playerc_client_destroy(client);

  return 0;
}
