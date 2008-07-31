/*
 * $Id: test_power.cc 3971 2007-02-02 17:12:04Z gerkey $
 *
 * a test for the C++ PositionProxy
 */

#include "test.h"
#include <unistd.h>

int
test_power(PlayerClient* client, int index)
{
  TEST("power");
  PowerProxy pp(client,index);

  // wait for P2OS to start up
  for(int i=0;i<20;i++)
    client->Read();

  for(int t = 0; t < 3; t++)
  {
    TEST1("reading data (attempt %d)", t);
    client->Read();
    PASS();

    std::cerr << pp << std::endl;
  }


  PASS();

  return(0);
}

