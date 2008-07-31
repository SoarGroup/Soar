/*
 * ptz.cc
 *
 * a simple demo to show how to send commands to and get feedback from
 * the Sony PTZ camera.  this program will pan the camera in a loop
 * from side to side
 */

#include <libplayerc++/playerc++.h>

#include <iostream>
#include <string>

// defines gHostname, gPort, ...
#include "args.h"

int main(int argc, char **argv)
{
  parse_args(argc,argv);

  using namespace PlayerCc;

  /* Connect to Player server */
  PlayerClient robot(gHostname, gPort);

  /* Request sensor data */
  PtzProxy zp(&robot, gIndex);
  CameraProxy cp(&robot, gIndex);

  int dir = 1;
  double newpan;
  for(;;)
  {
    try
    {
      robot.Read();

      std::cout << zp << std::endl;

      if(zp.GetPan() > dtor(40) || zp.GetPan() < dtor(-40))
      {
        newpan = dtor(dir*30);
        zp.SetCam(newpan, zp.GetTilt(), zp.GetZoom());
        for(int i=0; i<10; ++i)
        {
          robot.Read();
        }
        std::cout << zp << std::endl;
        dir = -dir;
      }
      newpan = zp.GetPan() + dir * dtor(5);
      zp.SetCam(newpan, zp.GetTilt(), zp.GetZoom());
    }
    catch (PlayerError e)
    {
      std::cerr << e << std::endl;
      return 1;
    }
  }

  return(0);
}

