/*
 * $Id: test_aio.cc 4029 2007-04-19 17:00:44Z gerkey $
 *
 * a test for the C++ PositionProxy
 */

#include "test.h"
#include <unistd.h>

using namespace PlayerCc;

int
test_aio(PlayerClient* client, int index)
{
  TEST("aio");
  try {
    AioProxy ap(client,index);

    for(int t = 0; t < 5; t++)
    {
      TEST1("reading data (attempt %d)", t);

      client->Read();
      PASS();

      std::cerr << ap << std::endl;
    }


  } catch(std::exception& e) {
    FAIL();
    std::cerr << e.what() << std::endl;
    return -1;
  }

  PASS();
  return 0;
}
