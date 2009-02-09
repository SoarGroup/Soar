#define USAGE \
  "USAGE: sonarobstacleavoid [-h <host>] [-p <port>] [-m]\n" \
  "       -h <host>: connect to Player on this host\n" \
  "       -p <port>: connect to Player on this TCP port\n" \
  "       -m       : turn on motors (be CAREFUL!)"

#include <libplayerc++/playerc++.h>
#include <iostream>

#include "args.h"

int main(int argc, char **argv)
{
  double min_front_dist = 0.500;
  double really_min_front_dist = 0.300;
  char avoid;

  parse_args(argc,argv);

  // we throw exceptions on creation if we fail
  try
  {
    using namespace PlayerCc;

    PlayerClient robot (gHostname, gPort);
    Position2dProxy pp (&robot, gIndex);
    SonarProxy sp (&robot, gIndex);

    std::cout << robot << std::endl;

    pp.SetMotorEnable (true);

    // go into read-think-act loop
    double newspeed = 0.0f, newturnrate = 0.0f;
    for(;;)
    {

      /* this blocks until new data comes; 10Hz by default */
      robot.Read();

      /*
       * sonar avoid.
       *   policy (pretty stupid):
       *     if(object really close in front)
       *       backup and turn away;
       *     else if(object close in front)
       *       stop and turn away;
       */
      avoid = 0;
      newspeed = 0.200;

      if (avoid == 0)
      {
          if((sp[2] < really_min_front_dist) ||
             (sp[3] < really_min_front_dist) ||
             (sp[4] < really_min_front_dist) ||
             (sp[5] < really_min_front_dist))
          {
              avoid = 50;
              newspeed = -0.100;
          }
          else if((sp[2] < min_front_dist) ||
                  (sp[3] < min_front_dist) ||
                  (sp[4] < min_front_dist) ||
                  (sp[5] < min_front_dist))
          {
              newspeed = 0;
              avoid = 50;
          }
      }

      if(avoid > 0)
      {
        if((sp[0] + sp[1]) <
           (sp[6] + sp[7]))
          newturnrate = dtor(-30);
        else
          newturnrate = dtor(30);
        avoid--;
      }
      else
        newturnrate = 0;

      // write commands to robot
      pp.SetSpeed(2 * newspeed, newturnrate);
    }
  }
  catch (PlayerCc::PlayerError e)
  {
    std::cerr << e << std::endl;
    return -1;
  }
}
