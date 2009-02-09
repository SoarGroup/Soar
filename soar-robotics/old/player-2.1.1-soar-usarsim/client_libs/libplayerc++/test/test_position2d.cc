/*
 * $Id: test_position2d.cc 3971 2007-02-02 17:12:04Z gerkey $
 *
 * a test for the C++ Position2DProxy
 */

#include "test.h"
#include <unistd.h>
#include <math.h>

using namespace PlayerCc;

int
test_position2d(PlayerClient* client, int index)
{
  TEST("position2d");
  Position2dProxy p2d(client,index);

  // wait for P2OS to start up
  for(int i=0;i<20;i++)
    client->Read();

  for(int t = 0; t < 3; t++)
  {
    TEST1("reading data (attempt %d)", t);

    client->Read();

    PASS();

    std::cerr << p2d << std::endl;
  }

  const double ox = 0.1, oy = -0.2;
  const int oa = 180;
  
  TEST("Setting odometry" );
  p2d.SetOdometry(ox, oy, DTOR((double)oa));

  printf("\n - initial \t[%.3f %.3f %.3f]\n"
   " - requested \t[%.3f %.3f %.3f]\n", 
   p2d.GetXPos(), p2d.GetYPos(), RTOD(p2d.GetYaw()), 
   ox, oy, (double)oa);
  
  
  for( int s=0; s<10; s++ )
  {
    client->Read();
    printf( " - reading \t[%.3f %.3f %.3f]\r", 
      p2d.GetXPos(), p2d.GetYPos(), RTOD(p2d.GetYaw()) );
    fflush(stdout);
  }

  puts("");
  
  if((p2d.GetXPos() != ox) || 
     (p2d.GetYPos() != oy) || 
     ((int)rint(RTOD(p2d.GetYaw())) != oa))
  {
    FAIL();
    //return(-1);
  }
  else
  {
    PASS();
  }

  TEST("resetting odometry");
  p2d.ResetOdometry();
  sleep(1);
  PASS();

  TEST("enabling motors");
  p2d.SetMotorEnable(1);
  PASS();

  TEST("moving forward");
  p2d.SetSpeed(0.1,0);
  sleep(3);
  PASS();
  
  TEST("moving backward");
  p2d.SetSpeed(-0.1,0);
  sleep(3);
  PASS();
  
  TEST("moving left");
  p2d.SetSpeed(0,0.1,0);
  sleep(3);
  PASS();
  
  TEST("moving right");
  p2d.SetSpeed(0,-0.1,0);
  sleep(3);
  PASS();
  
  TEST("turning right");
  p2d.SetSpeed(0,DTOR(-25.0));
  sleep(3);
  PASS();

  TEST("turning left");
  p2d.SetSpeed(0,DTOR(25.0));
  sleep(3);
  PASS();

  TEST("moving left and anticlockwise (testing omnidrive)");
  p2d.SetSpeed( 0, 0.1, DTOR(45.0) );
  sleep(3);
  PASS();
  
  
  TEST("moving right and clockwise (testing omnidrive)");
  p2d.SetSpeed( 0, -0.1, DTOR(-45) );
  sleep(3);
  PASS();
  
  TEST("stopping");
  p2d.SetSpeed(0,0);
  sleep(3);
  PASS();


  TEST("disabling motors");
  p2d.SetMotorEnable(0);
  sleep(1);
  PASS();
  
  /*
  TEST("changing to separate velocity control");
  p2d.SelectVelocityControl(1);
  sleep(1);
  PASS();
  
  TEST("changing to direct wheel velocity control");
  p2d.SelectVelocityControl(0);
      sleep(1);
      PASS();
  */
  
  TEST("resetting odometry");
  p2d.ResetOdometry();
      sleep(1);
      PASS();
    
  
  PASS();
  return 0;
}

