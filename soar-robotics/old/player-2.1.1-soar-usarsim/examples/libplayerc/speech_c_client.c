#include <stdio.h>
#include <libplayerc/playerc.h>

int
main(int argc, const char **argv)
{
  //int i;
  playerc_client_t *client;
  playerc_speechrecognition_t *speech_recognition;

  // Create a client and connect it to the server.
  client = playerc_client_create(NULL, "localhost", 6665);
  if (0 != playerc_client_connect(client))
    return -1;

  // Create and subscribe to a speech_recognition device.
  speech_recognition = playerc_speechrecognition_create(client, 0);
  if (playerc_speechrecognition_subscribe(speech_recognition, PLAYER_OPEN_MODE))
    return -1;

  for (;;)
  {
    // Wait for new data from server
    playerc_client_read(client);

    //playerc_speech_recognition_clear_messages(speech_recognition);
  }

  // Shutdown
  playerc_speechrecognition_unsubscribe(speech_recognition);
  playerc_speechrecognition_destroy(speech_recognition);
  playerc_client_disconnect(client);
  playerc_client_destroy(client);

  return 0;
}
