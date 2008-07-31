/*
 * $Id: test_position2d_holdsub.cc 3971 2007-02-02 17:12:04Z gerkey $
 *
 * a test for the C++ Position2DProxy
 */

#include "test.h"
#include <unistd.h>
#include <math.h>

using namespace PlayerCc;

int
test_position2d_holdsubscribe(PlayerClient* client, int index)
{
  TEST("position2d");
  Position2dProxy p2d(client,index);

  // wait for P2OS to start up
  for(int i=0;i<20;i++)
    client->Read();

  while(1)
  {
    client->Read();
    std::cerr << p2d << std::endl;
  }

  return -1;
}


 	  	 
