/***************************************************************************
 * Desc: Test program for the Player C client
 * Author: Andrew Howard
 * Date: 13 May 2002
 # CVS: $Id: test.c 4346 2008-02-07 02:23:00Z rtv $
 **************************************************************************/

#include "playerc.h"
#include "test.h"



int main(int argc, const char *argv[])
{
  playerc_client_t *client;
  const char *host;
  int port;
  int all;
  int i;
  char *arg;
  const char *opt, *val;
  const char *device, *sindex; int index;

  // Default host, port
  host = "localhost";
  port = 6665;
  all = 1;

  // Read program options (host and port).
  for (i = 1; i < argc - 1; i += 2)
  {
    opt = argv[i];
    val = argv[i + 1];
    if (strcmp(opt, "-h") == 0)
      host = val;
    else if (strcmp(opt, "-p") == 0)
      port = atoi(val);
  }

  // If there are individual device arguments, dont do all tests.
  for (i = 1; i < argc; i++)
  {
    opt = argv[i];
    if (strncmp(opt, "--", 2) == 0)
      all = 0;
  }

  printf("host [%s:%d]\n", host, port);

  client = playerc_client_create(NULL, host, port);

  TEST("connecting");
  if (playerc_client_connect(client) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();

  if (all)
  {
    // Get the available device list from the server.
    TEST("querying interface list");
    if (playerc_client_get_devlist(client) < 0)
    {
      FAIL();
      return -1;
    }
    PASS();
  }
  else
  {
    // Override the auto-detected stuff with command line directives.
    for (i = 1; i < argc; i++)
    {
      if (strncmp(argv[i], "--", 2) != 0)
        continue;

      // Get device name and index
      arg = strdup(argv[i]);
      device = strtok(arg + 2, ":");
      sindex = strtok(NULL, "");
      index = (sindex ? atoi(sindex) : 0);

      client->devinfos[client->devinfo_count].addr.interf =
        str_to_interf(device);
      client->devinfos[client->devinfo_count].addr.index = index;
      client->devinfo_count++;

      free(arg);
    }
  }

  // Print interface list.
  printf("selected devices [%s:%d]:\n", host, port);
  for (i = 0; i < client->devinfo_count; i++)
    printf("  %d:%s:%d (%s)\n",
           client->devinfos[i].addr.robot,
           interf_to_str(client->devinfos[i].addr.interf),
           client->devinfos[i].addr.index,
           client->devinfos[i].drivername);

  // Run all tests
  for (i = 0; i < client->devinfo_count; i++)
  {
    switch (client->devinfos[i].addr.interf)
      {
	// Laser device
      case PLAYER_LASER_CODE:
        test_laser(client, client->devinfos[i].addr.index);
        break;

	// Position device
      case PLAYER_POSITION2D_CODE:
        test_position2d(client, client->devinfos[i].addr.index);
        break;

	// Position device
      case PLAYER_POSITION3D_CODE:
        test_position3d(client, client->devinfos[i].addr.index);
        break;

	// log device
      case PLAYER_LOG_CODE:
	test_log(client, client->devinfos[i].addr.index);
	break;

	// gripper device
      case PLAYER_GRAPHICS2D_CODE:
	test_graphics2d(client, client->devinfos[i].addr.index);
	break;

	// gripper device
      case PLAYER_GRAPHICS3D_CODE:
	test_graphics3d(client, client->devinfos[i].addr.index);
	break;

	// gripper device
      case PLAYER_GRIPPER_CODE:
	test_gripper(client, client->devinfos[i].addr.index);
	break;

	// rfid device
      case PLAYER_RFID_CODE:
        test_rfid(client, client->devinfos[i].addr.index);
        break;

        // imu device
      case PLAYER_IMU_CODE:
        test_imu (client, client->devinfos[i].addr.index);
	break;

	// simulation device
      case PLAYER_SIMULATION_CODE:
        test_simulation(client, client->devinfos[i].addr.index);
        break;

#if 0
	// Sonar device
      case PLAYER_SONAR_CODE:
        test_sonar(client, client->devinfos[i].addr.index);
        break;

      // Power device
      case PLAYER_POWER_CODE:
        test_power(client, client->devinfos[i].addr.index);
        break;

      // map device
      case PLAYER_MAP_CODE:
        test_map(client, client->devinfos[i].addr.index);
        break;

#endif

      // Blobfinder device
      case PLAYER_BLOBFINDER_CODE:
        test_blobfinder(client, client->devinfos[i].addr.index);
        break;

      // Blobfinder device
      case PLAYER_BLINKENLIGHT_CODE:
        test_blinkenlight(client, client->devinfos[i].addr.index);
        break;

      // Camera device
      case PLAYER_CAMERA_CODE:
        test_camera(client, client->devinfos[i].addr.index);
        break;

#if 0
      // Fiducial detector
      case PLAYER_FIDUCIAL_CODE:
        test_fiducial(client, client->devinfos[i].addr.index);
        break;

      // GPS device
      case PLAYER_GPS_CODE:
        test_gps(client, client->devinfos[i].addr.index);
        break;

      // Joystick device
      case PLAYER_JOYSTICK_CODE:
        test_joystick(client, client->devinfos[i].addr.index);
        break;

      // Localization device
      case PLAYER_LOCALIZE_CODE:
        test_localize(client, client->devinfos[i].addr.index);
        break;

      // Position device
      case PLAYER_POSITION3D_CODE:
        test_position3d(client, client->devinfos[i].addr.index);
        break;
#endif

      // PTZ device
      case PLAYER_PTZ_CODE:
        test_ptz(client, client->devinfos[i].addr.index);
        break;

#if 0
      // Truth device
      case PLAYER_TRUTH_CODE:
        test_truth(client, client->devinfos[i].addr.index);
        break;

      // WiFi device
      case PLAYER_WIFI_CODE:
        test_wifi(client, client->devinfos[i].addr.index);
        break;
#endif
      // WSN device
      case PLAYER_WSN_CODE:
        test_wsn(client, client->devinfos[i].addr.index);
        break;

      // AIO device
      case PLAYER_AIO_CODE:
        test_aio(client, client->devinfos[i].addr.index);
        break;

      // DIO device
      case PLAYER_DIO_CODE:
        test_dio(client, client->devinfos[i].addr.index);
        break;

      // SPEECH device
      case PLAYER_SPEECH_CODE:
        test_speech(client, client->devinfos[i].addr.index);
        break;


      default:
        printf("no test for interface [%s]\n",
               interf_to_str(client->devinfos[i].addr.interf));
        break;
    }
  }

  TEST("disconnecting");
  if (playerc_client_disconnect(client) != 0)
  {
    FAIL();
    return -1;
  }
  PASS();

  playerc_client_destroy(client);

  return 0;
}


