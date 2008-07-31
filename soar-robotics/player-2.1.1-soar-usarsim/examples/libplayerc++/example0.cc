#include <iostream>
#include <libplayerc++/playerc++.h>

int
main(int argc, char *argv[])
{
  using namespace PlayerCc;

  PlayerClient    robot("localhost");
  SonarProxy      sp(&robot,0);
  Position2dProxy pp(&robot,0);

  for(;;)
  {
    double turnrate, speed;

    // read from the proxies
    robot.Read();

    // print out sonars for fun
    std::cout << sp << std::endl;

    // do simple collision avoidance
    if((sp[0] + sp[1]) < (sp[6] + sp[7]))
      turnrate = dtor(-20); // turn 20 degrees per second
    else
      turnrate = dtor(20);

    if(sp[3] < 0.500)
      speed = 0;
    else
      speed = 0.100;

    // command the motors
    pp.SetSpeed(speed, turnrate);
  }
}

