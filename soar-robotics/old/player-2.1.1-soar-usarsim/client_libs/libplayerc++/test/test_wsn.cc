/*
 * $Id: test_wsn.cc 3483 2006-04-11 15:40:31Z veedee $
 *
 * a test for the C++ WSNProxy
 */

#include "test.h"

int test_wsn(PlayerClient* client, int index)
{
  TEST("wsn");
  try
  {
    using namespace PlayerCc;

    WSNProxy cp(client, index);

    for (int i=0; i<10; ++i)
    {
	TEST("read wsn");
	client->Read();
	PASS();

	std::cout << cp << std::endl;
    }
       
    TEST("setting the data frequency rate");
    cp.DataFreq(-1, -1, 1);
    PASS ();
    
    TEST("enabling all LEDs");
    cp.SetDevState(-1, -1, 3, 7);
    PASS();
  }
  catch (PlayerCc::PlayerError e)
  {
      std::cerr << e << std::endl;
      return -1;
  }
  return 1;
}
