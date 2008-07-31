/***************************************************************************
 * Desc: Test program for the Player C client
 * Author: Andrew Howard, Richard Vaughan
 * Date: 13 May 2002
 # CVS: $Id: test.cc 4346 2008-02-07 02:23:00Z rtv $
 **************************************************************************/

#include <unistd.h>

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include "test.h"

bool use_stage;

int main(int argc, const char *argv[])
{
  const char *host;
  int port;
  int i;
  char *arg;
  const char *device, *sindex; int index;

  // Default host, port
  host = "localhost";
  port = 6665;

  // Read program options
  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-h") == 0)
    {
      if(++i >= argc)
      {
        puts("missing hostname");
        exit(-1);
      }
      host = argv[i];
    }
    else if (strcmp(argv[i], "-p") == 0)
    {
      if(++i >= argc)
      {
        puts("missing port");
        exit(-1);
      }
      port = atoi(argv[i]);
    }
    else if(!strcmp(argv[i],"-stage"))
      use_stage = true;
    else if(!strcmp(argv[i], "-help") || !strcmp(argv[i], "--help"))
    {
      puts("usage: test [-h <host>] [-p <port>] [-stage] [--all|<tests>]\n"\
        "where <tests> is one or more of --<interface>[:<index>] (default index is 0).\n"
        "Available interfaces to test are:\n"
        "  rfid\n"\
        "  wsn\n"\
        "  power\n"\
        "  dio\n"\
        "  position2d\n"\
        "  sonar\n"\
        "  laser\n"\
        "  ptz\n"\
        "  gripper\n"\
        "  camera\n"\
        "  actarray\n"\
        "  aio\n"\
        "  speech\n"\
        "  ranger\n"\
        "  position2d-subscribe  (subscribe to position2d indefinitely)\n"\
        "  gripper-subscribe     (subscribe to gripper indefinitely)\n"\
        "");
      exit(0);
    }
  }

  printf("host [%s:%d]\n", host, port);
  PlayerCc::PlayerClient client(host, port);

  // Check each command line argument for the form --<interface>:<index>, and
  // run the test for <interface> if recognized.
  for (i = 1; i < argc; i++)
  {
    if (strncmp(argv[i], "--", 2) != 0)
      continue;

    // Get device name and index
    arg = strdup(argv[i]);
    device = strtok(arg + 2, ":");
    sindex = strtok(NULL, "");
    index = (sindex ? atoi(sindex) : 0);

/*
    // Test the ClientProxy and PlayerClient
#ifdef HAVE_BOOST_SIGNALS
    // we need both signals and threads
#ifdef HAVE_BOOST_THREAD
    if(strcmp(device, "client") == 0 || strcmp(device, "all") == 0)
      test_client(&client, index);
#endif
#endif
*/

    try {
      if(strcmp(device, "rfid") == 0 || strcmp(device, "all") == 0)
        test_rfid(&client, index);
      if(strcmp(device, "wsn") == 0 || strcmp(device, "all") == 0)
        test_wsn(&client, index);
      if(strcmp(device, "power") == 0 || strcmp(device, "all") == 0)
        test_power(&client, index);
      if(strcmp(device, "dio") == 0 || strcmp(device, "all") == 0)
        test_dio(&client, index);
      /*
      if (strcmp(device, "motor") == 0 || strcmp(device, "all") == 0)
        test_motor(&client, index);
      if (strcmp(device, "position") == 0 || strcmp(device, "all") == 0)
        test_position(&client, index);
      */
      if (strcmp(device, "position2d") == 0 || strcmp(device, "all") == 0)
        test_position2d(&client, index);
      if (strcmp(device, "position2d-subscribe") == 0)
        test_position2d_holdsubscribe(&client, index);
      /*
      if (strcmp(device, "position3d") == 0 || strcmp(device, "all") == 0)
        test_position3d(&client, index);
      // Position device - position control mode
      // not many robots support this mode but Stage's position model does.
      if (strcmp(device, "position_control") == 0 || strcmp(device, "all") == 0)
        test_position_control(&client, index);
      */

      if(strcmp(device, "sonar") == 0 || strcmp(device, "all") == 0)
        test_sonar(&client, index);

      /*
      if(strcmp(device, "lookup") == 0 || strcmp(device, "all") == 0)
        test_lookup(&client, index);
      */

      /*
      if(strcmp(device, "blobfinder") == 0 || strcmp(device, "all") == 0)
        test_blobfinder(&client, index);
      */

      if(strcmp(device, "laser") == 0 || strcmp(device, "all") == 0)
        test_laser(&client, index);

      /*
      if(strcmp(device, "fiducial") == 0 || strcmp(device, "all") == 0)
        test_fiducial(&client, index);
      */

      if(strcmp(device, "ptz") == 0 || strcmp(device, "all") == 0)
        test_ptz(&client, index);

      
      if(strcmp(device, "speech") == 0 || strcmp(device, "all") == 0)
        test_speech(&client, index);
      /*

      if(strcmp(device, "vision") == 0 || strcmp(device, "all") == 0)
        test_vision(&client, index);
      */

      /*
      if(strcmp(device, "gps") == 0 || strcmp(device, "all") == 0)
        test_gps(&client, index);
      */

      if(strcmp(device, "gripper") == 0 || strcmp(device, "all") == 0)
        test_gripper(&client, index);
      if(strcmp(device, "gripper-subscribe") == 0)
        test_gripper_holdsubscribe(&client, index);

      /*
      if(strcmp(device, "truth") == 0 || strcmp(device, "all") == 0)
        test_truth(&client, index);
      */

      /*
      if(strcmp(device, "laserbeacon") == 0 || strcmp(device, "all") == 0)
        test_lbd(&client, index);
      */

      if(strcmp(device, "bumper") == 0 || strcmp(device, "all") == 0)
        test_bumper(&client, index);

      /*
      if(strcmp(device, "wifi") == 0 || strcmp(device, "all") == 0)
        test_wifi(&client, index);
      */

      /*
      if(strcmp(device, "log") == 0 || strcmp(device, "all") == 0)
        test_log(&client, index);
      */

      /*
      if(strcmp(device, "mcom") == 0 || strcmp(device, "all") == 0)
        test_mcom(&client, index);

      if(strcmp(device, "localize") == 0 || strcmp(device, "all") == 0)
        test_localize(&client, index);

      if(strcmp(device, "audiodsp") == 0 || strcmp(device, "all") == 0)
        test_audiodsp(&client, index);

      if(strcmp(device, "audiomixer") == 0 || strcmp(device, "all") == 0)
        test_audiomixer(&client, index);

      if(strcmp(device, "blinkenlight") == 0 || strcmp(device, "all") == 0)
        test_blinkenlight(&client, index);

      */
      if(strcmp(device, "camera") == 0 || strcmp(device, "all") == 0)
        test_camera(&client, index);

      /*
      if(strcmp(device, "bps") == 0 || strcmp(device, "all") == 0)
        test_bps(&client, index);
       */

      /*
      if(strcmp(device, "idar") == 0 || strcmp(device, "all") == 0)
        test_idar(&client, index);

      if(strcmp(device, "idarturret") == 0 || strcmp(device, "all") == 0)
        test_idarturret(&client, index);
      */

      if(strcmp(device, "actarray") == 0 || strcmp(device, "all") == 0)
        test_actarray(&client, index);

      if(strcmp(device, "aio") == 0 || strcmp(device, "all") == 0) 
        test_aio(&client, index);

      if(strcmp(device, "ranger") == 0 || strcmp(device, "all") == 0)
        test_ranger(&client, index);


    } catch(std::exception& e) {
      FAIL();
      std::cerr << "Caught exception: " << e.what() << std::endl;
    }

    free(arg);
  } 

  return 0;
}

