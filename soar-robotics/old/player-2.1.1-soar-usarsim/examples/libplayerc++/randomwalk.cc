/*
 * randomwalk.cc - sonar obstacle avoidance with random walk
 */

#include <libplayerc++/playerc++.h>
#include <iostream>
using namespace PlayerCc;

#include "args.h"

double minfrontdistance = 0.750;
double speed = 0.350;
double avoidspeed = 0; // -150;
double turnrate = DTOR(40);

int main(int argc, char** argv)
{
  int randint;
  int randcount = 0;
  int avoidcount = 0;
  bool obs = false;
  parse_args(argc,argv);
  LaserProxy *lp = NULL;
  SonarProxy *sp = NULL;

  // we throw exceptions on creation if we fail
  try
  {

    PlayerClient robot(gHostname, gPort);
    Position2dProxy pp(&robot, gIndex);

    if (gUseLaser)
    {
      lp = new LaserProxy (&robot, gIndex);
      // make sure we get some data
      while(lp->GetCount() == 0)
      {
        robot.Read();
      }
    }
    else
    {
      sp = new SonarProxy (&robot, gIndex);
      // make sure we get some data
      while(sp->GetCount() == 0)
      {
        robot.Read();
      }
    }

    std::cout << robot << std::endl;

    pp.SetMotorEnable (true);

    // go into read-think-act loop
    double newturnrate=0.0f, newspeed=0.0f;
    for(;;)
    {

      robot.Read();

      /* See if there is an obstacle in front */
      if (gUseLaser)
      {
        obs = false;
        for (uint32_t i = 0; i < lp->GetCount (); i++)
        {
          if(((*lp)[i]) && (*lp)[i] < minfrontdistance)
            obs = true;
        }
      }
      else
      {
        obs = ((sp->GetScan (2) < minfrontdistance) ||   // 0?
               (sp->GetScan (3) < minfrontdistance) ||
               (sp->GetScan (4) < minfrontdistance) ||   // 0?
               (sp->GetScan (5) < minfrontdistance) );
      }

      if(obs || avoidcount)// || pp.GetStall ())
      {
        newspeed = avoidspeed;

        /* once we start avoiding, continue avoiding for 2 seconds */
        /* (we run at about 10Hz, so 20 loop iterations is about 2 sec) */
        if(!avoidcount)
        {
          avoidcount = (1+(int)(10.0*rand()/(RAND_MAX+1.0))) + 20;
          //avoidcount = 30;
          randcount = 0;

          if(gUseLaser)
          {
            if(lp->MinLeft () < lp->MinRight ())
              newturnrate = -turnrate;
            else
              newturnrate = turnrate;
          }
          else
          {
            if(sp->GetScan (1) + sp->GetScan (15) < sp->GetScan (7) + sp->GetScan (8))
              newturnrate = -turnrate;
            else
              newturnrate = turnrate;
          }
        }
        avoidcount--;
      }
      else
      {
        avoidcount = 0;
        newspeed = speed;

        /* update turnrate every 3 seconds */
        if(!randcount)
        {
          /* make random int tween -20 and 20 */
          //randint = (1+(int)(40.0*rand()/(RAND_MAX+1.0))) - 20;
          randint = rand() % 41 - 20;

          newturnrate = dtor(randint);
          randcount = 30;
        }
        randcount--;
      }

      // write commands to robot
      pp.SetSpeed(newspeed, newturnrate);
    }
  }
  catch (PlayerCc::PlayerError e)
  {
    std::cerr << e << std::endl;
    return -1;
  }
}
