/*
 * $Id: test_ptz.cc 3971 2007-02-02 17:12:04Z gerkey $
 *
 * a test for the C++ PositionProxy
 */

#include "test.h"
#include <unistd.h>

using namespace PlayerCc;

int
test_ptz(PlayerClient* client, int index)
{
  TEST("ptz");
  try {
    PtzProxy zp(client,index);

    for(int t = 0; t < 3; t++)
    {
      TEST1("reading data (attempt %d)", t);

      client->Read();
      PASS();

      std::cerr << zp << std::endl;
    }

    TEST("panning left");
    zp.SetCam(DTOR(90),0,0);
    sleep(3);
    PASS();

    TEST("panning right");
    zp.SetCam(DTOR(-90),0,0);
    sleep(3);
    PASS();

    TEST("tilting up");
    zp.SetCam(0,DTOR(25),0);
    sleep(3);
    PASS();

    TEST("tilting down");
    zp.SetCam(0,DTOR(-25),0);
    sleep(3);
    PASS();

    TEST("zooming in");
    zp.SetCam(0,0,DTOR(10));
    sleep(3);
    PASS();

    TEST("zooming out");
    zp.SetCam(0,0,DTOR(60));
    sleep(3);
    PASS();


  } catch(std::exception& e) {
    FAIL();
    std::cerr << e.what() << std::endl;
    return -1;
  }

  PASS();
  return 0;
}
