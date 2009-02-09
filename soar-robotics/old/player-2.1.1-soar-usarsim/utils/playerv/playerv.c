/*
 *  PlayerViewer
 *  Copyright (C) Andrew Howard 2002
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
/**************************************************************************
 * Desc: PlayerView, program entry point
 * Author: Andrew Howard
 * Date: 28 Mar 2002
 * CVS: $Id: playerv.c 4325 2008-01-09 11:45:37Z thjc $
 *************************************************************************/

/** @ingroup utils */
/** @{ */
/** @defgroup util_playerv playerv
 * @brief General-purpose sensor visualization and device control GUI

@par Synopsis

playerv is a GUI client program that visualizes sensor data from
a player server.  It also provides some teleoperation capabilities.

playerv requires the GTK+-2.0 development libraries and headers.

@par Usage

playerv is installed alongside player in $prefix/bin, so if player is
in your PATH, then playerv should also be.  Command-line usage is:
@verbatim
$ playerv [-h <hostname>] [-p <port>] [-rate <Hz>] [--<device>:<index>] [--<device>:<index>] ...
@endverbatim
For example, to connect to Player on localhost at the default port
(6665), and subscribe to the 0th position and sonar devices:
@verbatim
$ playerv --position:0 --sonar:0
@endverbatim
To connect to Player on another machine (foo) at a non-default port
(7000), and not subscribe to any devices:
@verbatim
$ playerv -h foo -p 7000
@endverbatim

If rate is a positive value, playerv attempts to poll the server for data
at the requested rate, in Hz.  If rate is 0, then playerv switches to PUSH
mode, in which the server pushes all data, when it's ready.  The default rate is 5Hz.

When playerv starts, a window will pop up.  Click and drag with the left
mouse button to pan the window.  Click and drag with the right mouse
buttom to zoom the window.  These are the same controls as Stage 1.3.x.

Use the "Devices" menu to control device subscriptions.  For devices
that can be teleoperated via playerv, click the "Command" item in the
submenu for that device.  See below for how to teleoperate different
kinds of devices.

The "View" menu offers options for changing the look of the display.

The "File" menu offers options for dumping JPG and PPM screenshots and
making MPEG movies (to make movies, you must have enabled movie support
when building librtk).

@par Features

playerv can visualize data from the following kinds of devices:
- @ref interface_actarray
- @ref interface_blobfinder
- @ref interface_bumper
- @ref interface_fiducial
- @ref interface_gripper
- @ref interface_ir
- @ref interface_laser
- @ref interface_localize
- @ref interface_map
- @ref interface_position2d
- @ref interface_power
- @ref interface_ptz
- @ref interface_ranger
- @ref interface_sonar
- @ref interface_wifi

playerv provides teleoperation of the following kinds of devices:
- @ref interface_position2d : In velocity mode (the default),
  click and drag with the left mouse button to set desired velocity vector
  (this will only work if the underlying driver supports velocity control;
  most position drivers do).  In position mode (select "Position mode"
  from the device's submenu), click and drag with the left mouse button
  to set a position target (this will only work if the underlying driver
  support position control; @ref driver_vfh is one example).
- @ref interface_ptz : Click and drag the green circle to pan and zoom;
  click and drag the blue circle to tilt.


@par Screenshots
@image html playerv-sonar.jpg "Screenshot of playerv showing position and sonar data"
@image html playerv-laser-blobfinder-ptz.jpg "Screenshot of playerv showing position, laser, blobfinder, and ptz data"

@author Andrew Howard

*/

/** @} */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "playerv.h"


// Set flag to 1 to force program to quit
static int quit = 0;

// Handle quit signals
void sig_quit(int signum)
{
  quit = 1;
}


// Print the usage string
void print_usage()
{
  printf("\nPlayerViewer %s, ", VERSION);
  printf("a visualization tool for the Player robot device server.\n");
  printf("Usage  : playerv [-h <hostname>] [-p <port>] [-rate <Hz>]\n");
  printf("                 [--<device>:<index>] [--<device>:<index>] ... \n");
  printf("Example: playerv -p 6665 --position:0 --sonar:0\n");
  printf("\n");
}


// Main
int main(int argc, char **argv)
{
  playerc_client_t *client;
  rtk_app_t *app;
  mainwnd_t *mainwnd;
  opt_t *opt;
  const char *host;
  int port;
  int i;
  int count;
  double rate;
  char section[256];
  int device_count;
  device_t devices[PLAYER_MAX_DEVICES];
  device_t *device;
  struct timeval tv, tc = {0, 0};
  struct timespec st = {0,0};

  printf("PlayerViewer %s\n", VERSION);

  // Initialise rtk lib (after we have read the program options we
  // want).
  rtk_init(&argc, &argv);

  // Register signal handlers
  signal(SIGINT, sig_quit);
  signal(SIGQUIT, sig_quit);

  // Load program options
  opt = opt_init(argc, argv, NULL);
  if (!opt)
  {
    print_usage();
    return -1;
  }

  // Pick out some important program options
  host = opt_get_string(opt, "", "host", NULL);
  if (!host)
    host = opt_get_string(opt, "", "h", "localhost");

  port = opt_get_int(opt, "", "port", -1);
  if (port < 0)
    port = opt_get_int(opt, "", "p", 6665);

  rate = opt_get_double(opt, "", "rate", 5.0);
  if(rate < 0.0)
    rate = 0.0;

  // Connect to the server
  printf("Connecting to [%s:%d]\n", host, port);
  client = playerc_client_create(NULL, host, port);
  if (playerc_client_connect(client) != 0)
  {
    PRINT_ERR1("%s", playerc_error_str());
    print_usage();
    return -1;
  }

  if(rate == 0.0)
  {
    printf("Setting delivery mode to PLAYER_DATAMODE_PUSH\n", host, port);
    // Change the server's data delivery mode.
    if (playerc_client_set_replace_rule(client, -1, -1, -1, -1, 0) != 0)
    {
      PRINT_ERR1("%s", playerc_error_str());
      return -1;
    }

    // Change the server's data delivery mode.
    // PLAYERC_DATAMODE_PUSH, PLAYERC_DATAMODE_PULL
    if (playerc_client_datamode(client, PLAYERC_DATAMODE_PUSH) != 0)
    {
      PRINT_ERR1("%s", playerc_error_str());
      return -1;
    }
  }

  // Get the available devices.
  if (playerc_client_get_devlist(client) != 0)
  {
    PRINT_ERR1("%s", playerc_error_str());
    return -1;
  }

  // Create gui
  app = rtk_app_create();

  // Create a window for most of the sensor data
  mainwnd = mainwnd_create(app, host, port);
  if (!mainwnd)
    return -1;

  // Create a list of available devices, with their gui proxies.
  device_count = 0;
  for (i = 0; i < client->devinfo_count; i++)
  {
    device = devices + device_count;

    device->addr = client->devinfos[i].addr;
    device->drivername = strdup(client->devinfos[i].drivername);

    // See if the device should be subscribed immediately.
    snprintf(section, sizeof(section), "%s:%d",
             interf_to_str(device->addr.interf), device->addr.index);
    device->subscribe = opt_get_int(opt, section, "", 0);
    device->subscribe = opt_get_int(opt, section, "subscribe", device->subscribe);
    if (device->addr.index == 0)
    {
      snprintf(section, sizeof(section), "%s",
               interf_to_str(device->addr.interf));
      device->subscribe = opt_get_int(opt, section, "", device->subscribe);
      device->subscribe = opt_get_int(opt, section, "subscribe", device->subscribe);
    }

    // Allow for --position instead of --position2d
    if(device->addr.interf == PLAYER_POSITION2D_CODE)
    {
      snprintf(section, sizeof(section), "%s:%d",
               PLAYER_POSITION2D_STRING, device->addr.index);
      device->subscribe = opt_get_int(opt, section, "", device->subscribe);
      device->subscribe = opt_get_int(opt, section, "subscribe", device->subscribe);
      if (device->addr.index == 0)
      {
        snprintf(section, sizeof(section), "%s", PLAYER_POSITION2D_STRING);
        device->subscribe = opt_get_int(opt, section, "", device->subscribe);
        device->subscribe = opt_get_int(opt, section, "subscribe", device->subscribe);
      }
    }

    // Create the GUI proxy for this device.
    create_proxy(device, opt, mainwnd, client);

    device_count++;
  }

  // Print the list of available devices.
  printf("Available devices: %s:%d\n", host, port);
  for (i = 0; i < device_count; i++)
  {
    device = devices + i;
    snprintf(section, sizeof(section), "%s:%d",
             interf_to_str(device->addr.interf), device->addr.index);
    printf("%-16s %-40s", section, device->drivername);
    if (device->proxy)
    {
      if (device->subscribe)
        printf("subscribed");
      else
        printf("ready");
    }
    else
      printf("unsupported");
    printf("\n");
  }

  // Print out a list of unused options.
  opt_warn_unused(opt);

  // Start the gui; dont run in a separate thread and dont let it do
  // its own updates.
  rtk_app_main_init(app);

  // start out timer if in pull mode
  if(rate > 0.0)
    gettimeofday(&tv, NULL);
  
  while (!quit)
  {
    // Let gui process messages
    rtk_app_main_loop(app);

    if(rate == 0.0)  // if we're in push mode
    {
      // see if there's data
      count = playerc_client_peek(client, 50);
      if (count < 0)
      {
        PRINT_ERR1("%s", playerc_error_str());
        break;
      }
      if (count > 0)
      {
        /*proxy = */playerc_client_read_nonblock(client);
      }
    }
    else // we're in pull mode
    {
      // we only want to request new data at the target rate
      gettimeofday(&tc, NULL);
      if(((tc.tv_sec - tv.tv_sec) + (tc.tv_usec - tv.tv_usec)/1e6) > 1.0/rate)
      {
        tv = tc;
        // this requests a round of data from the server to be read
        playerc_client_requestdata(client);
        playerc_client_read_nonblock(client);
       }
       else
       {
        // sleep for the minimum time we can, so we don't use up too much
        // processor
        nanosleep(&st, NULL);
       }
    }


    // Update the devices
    for (i = 0; i < device_count; i++)
    {
      device = devices + i;
      if(device->proxy)
        (*(device->fnupdate)) (device->proxy);
    }
    // Update the main window
    if (mainwnd_update(mainwnd) != 0)
      break;
  }

  // Stop the gui
  rtk_app_main_term(app);

  // Destroy devices
  for (i = 0; i < device_count; i++)
  {
    device = devices + i;
    if (device->proxy)
      (*(device->fndestroy)) (device->proxy);
    free(device->drivername);
  }

  // Disconnect from server
  if (playerc_client_disconnect(client) != 0)
  {
    PRINT_ERR1("%s", playerc_error_str());
    return -1;
  }
  playerc_client_destroy(client);

  // For some reason, either of the following calls makes the program
  // segfault on exit.  I haven't figured out why, so I'm commenting them out.  - BPG

  // Destroy the windows
  //mainwnd_destroy(mainwnd);

  // Destroy the gui
  //rtk_app_destroy(app);

  opt_term(opt);

  return 0;
}
