/*
 * $Id: test_actarray.cc 3971 2007-02-02 17:12:04Z gerkey $
 *
 * a test for the C++ SonarProxy
 */

#include "test.h"
#include <unistd.h>

using namespace PlayerCc;

int
test_actarray(PlayerClient* client, int index)
{
  ActArrayProxy aap(client,index);

  // wait for P2OS to start up
  for(int i=0; i < 20; i++)
    client->Read();


  aap.RequestGeometry();
  aap.GetActuatorGeom(0);

  for(int t = 0; t < 3; t++)
  {
    TEST1("reading data (attempt %d)", t);
    client->Read();

    std::cerr << "got actarray data: " << std::endl << aap << std::endl;

    PASS();
  }

  const int wait_iters = 50;

  TEST("homing actuator #0");
  aap.MoveHome(0);
  for(int i = 0; i < wait_iters; ++i)
  {
    client->Read();
    if(i % 5 == 0)
      std::cerr << aap << std::endl;
  }
  PASS();

  TEST("moving #0 to 1.0");
  aap.MoveTo(0, 1.0);
  for(int i = 0; i < wait_iters; ++i)
  {
    client->Read();
    if(i % 5 == 0)
      std::cerr << aap << std::endl;
  }
  PASS();

  TEST("moving #0 to 0.0");
  aap.MoveTo(0, 0.0);
  for(int i = 0; i < wait_iters; ++i)
  {
    client->Read();
    if(i % 5 == 0)
      std::cerr << aap << std::endl;
  }
  PASS();

  TEST("moving #0 to 0.5");
  aap.MoveTo(0, 0.5);
  for(int i = 0; i < wait_iters; ++i)
  {
    client->Read();
    if(i % 5 == 0)
      std::cerr << aap << std::endl;
  }
  PASS();

  TEST("moving #0 at speed 0.25, then setting speed to 0");
  aap.MoveAtSpeed(0, 0.25);
  for(int i = 0; i < wait_iters; ++i)
  {
    client->Read();
    if(i % 5 == 0)
      std::cerr << aap << std::endl;
  }
  aap.MoveAtSpeed(0, 0.0);
  PASS();

  TEST("moving #0 at speed -0.3, then setting speed to 0");
  aap.MoveAtSpeed(0, -0.3);
  for(int i = 0; i < wait_iters; ++i)
  {
    client->Read();
    if(i % 5 == 0)
      std::cerr << aap << std::endl;
  }
  aap.MoveAtSpeed(0, 0.0);
  PASS();

  TEST("homing #0");
  aap.MoveHome(0);
  for(int i = 0; i < wait_iters; ++i)
  {
    client->Read();
    if(i % 5 == 0)
      std::cerr << aap << std::endl;
  }
  PASS();

  PASS();
  return(0);
}


 	  	 
