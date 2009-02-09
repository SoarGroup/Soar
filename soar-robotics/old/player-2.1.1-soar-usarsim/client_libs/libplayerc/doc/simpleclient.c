
#include <stdio.h>

#include <libplayerc/playerc.h>

int main(int argc, const char **argv)
{
  int i;
  playerc_client_t *client;
  playerc_position2d_t *position2d;

  // Create a client object and connect to the server; the server must
  // be running on "localhost" at port 6665
  client = playerc_client_create(NULL, "localhost", 6665);
  if (playerc_client_connect(client) != 0)
  {
    fprintf(stderr, "error: %s\n", playerc_error_str());
    return -1;
  }

  // Create a position2d proxy (device id "position2d:0") and susbscribe
  // in read/write mode
  position2d = playerc_position2d_create(client, 0);
  if (playerc_position2d_subscribe(position2d, PLAYERC_OPEN_MODE) != 0)
  {
    fprintf(stderr, "error: %s\n", playerc_error_str());
    return -1;
  }

  // Enable the robots motors
  playerc_position2d_enable(position2d, 1);

  // Start the robot turning slowing
  playerc_position2d_set_cmd_vel(position2d, 0, 0, 0.1, 1);

  for (i = 0; i < 200; i++)
  {
    // Read data from the server and display current robot position
    playerc_client_read(client);
    printf("position : %f %f %f\n",
           position2d->px, position2d->py, position2d->pa);
  } 

  // Shutdown and tidy up
  playerc_position2d_unsubscribe(position2d);
  playerc_position2d_destroy(position2d);
  playerc_client_disconnect(client);
  playerc_client_destroy(client);

  return 0;
}
