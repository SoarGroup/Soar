/*
 * $Id: test_dio.cc 4029 2007-04-19 17:00:44Z gerkey $
 *
 * a test for the C++ PositionProxy
 */

#include "test.h"
#include <unistd.h>

using namespace PlayerCc;

int
test_dio(PlayerClient* client, int index)
{
  TEST("dio");
  DioProxy dp(client,index);

  for(int t = 0; t < 5; t++)
  {
    TEST1("reading data (attempt %d)", t);

    client->Read();

    PASS();

    std::cerr << dp << std::endl;
  }

  PASS();


  TEST("dio: setting outputs");
  unsigned int value(0);
  const unsigned int do_count(8);

  for(int t = 0; t < 5; t++)
  {
    TEST1("writing data (attempt %d)", t);
    TEST1("value: %d", value);

    dp.SetOutput(do_count,value);
    ++value;

    PASS();
    usleep(200000);
  }

  PASS();

  //turn all outputs off
  value=0;
  dp.SetOutput(do_count,value);
  
  
  return(0);
}

