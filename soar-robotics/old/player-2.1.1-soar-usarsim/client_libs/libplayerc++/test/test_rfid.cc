/*
 * $Id: test_rfid.cc 3395 2006-02-24 02:56:36Z gerkey $
 *
 * a test for the C++ RFIDProxy
 */

#include "test.h"

int test_rfid(PlayerClient* client, int index)
{
  TEST("rfid");
  try
  {
	  using namespace PlayerCc;

	  RFIDProxy cp(client, index);

      // wait for the rfid to warm up
	  for(int i=0;i<20;i++)
		  client->Read();

	  for (int i=0; i<10; ++i)
	  {
		  TEST("read rfid");
		  client->Read();
		  PASS();

		  std::cout << cp << std::endl;
	   }
  }
  catch (PlayerCc::PlayerError e)
  {
	  std::cerr << e << std::endl;
	  return -1;
  }
  return 1;
}
