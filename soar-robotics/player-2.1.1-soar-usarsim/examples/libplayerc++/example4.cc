#include <iostream>
#include <libplayerc++/playerc++.h>
#include <time.h>

enum IRName { L90=0, L60, L30, L0, R0, R30, R60, R90 };

int main(int argc, char *argv[])
{
  using namespace PlayerCc;
  using namespace std;

  int preferredTurn = 1;

  std::cout << "connecting to robot" << std::endl;
  PlayerClient    robot( "localhost" );
  std::cout << "connecting to ir proxy" << std::endl;
  IrProxy         ir(&robot,0);
  std::cout << "connecting to pos2d proxy" << std::endl;
  Position2dProxy pp(&robot,0);
  const float     tooClose = 0.400;
  time_t          t;
  time_t          firstt = 0;
  int             samples = 0;


  pp.SetMotorEnable( true );

  for(;;)
  {
    double turnrate = 0;
    double speed = 0;

    time( &t );
    if ( firstt != 0 && firstt != t )
    {
      //      cout << ((double)samples/(difftime( t, firstt ))) << " samples/sec" << endl;
    }

    if ( firstt == 0 )
    {
      firstt = t;
    }

    // read from the proxies
    robot.Read();
    ++samples;

    //#ifdef FOO
    // print out ir readings for fun
    //    std::cout << ir << std::endl;

    // do simple collision avoidance

    double left;
    double right;
    // Only react to objects closer than 0.4m off to the sides
    left = MIN( 0.4, ir[L90] );
    left += MIN( 0.4, ir[L60] );
    right = MIN( 0.4, ir[R90] );
    right += MIN( 0.4, ir[R60] );

    // Turn into the more open area
    if( left < right )
    {
      turnrate = dtor(-20); // turn 20 degrees per second
      preferredTurn = -1;
    }
    else if ( left > right )
    {
      turnrate = dtor(20);
      preferredTurn = 1;
    }
    else
      turnrate = 0;

    // Stop if the front sensors detect an object that is too close
    // Start up again if the path gets cleared.
    if (( ir[L0] < tooClose ) || (ir[R0] < tooClose ) ||
        ( ir[L30] < tooClose ) || (ir[R30] < tooClose ))
    {
      turnrate = dtor(20*preferredTurn);
      speed = 0;
    }
    else
      //#endif
      speed = 0.2;

    // command the motors
//    std::cout << "Setting speed to " << speed << " and turnrate to " << rtod(turnrate) << std::endl;

    pp.SetSpeed(speed, turnrate);
  }
}

