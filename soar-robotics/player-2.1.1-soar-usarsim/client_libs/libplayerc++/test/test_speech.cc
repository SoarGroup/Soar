/*
 * Testing the Speech Proxy. Alexis Maldonado. May 4 2007.
 */

#include "test.h"
#include <unistd.h>
#include <string>

using namespace std;

using namespace PlayerCc;

int
test_speech(PlayerClient* client, int index)
{
  TEST("speech");
  SpeechProxy sp(client,index);

  TEST("speech: saying something");

  string hello("Hello World!");
  string numbers("12345678901234567890123456789012345678901234567890");


  TEST1("writing data (attempt %d)", 1);

  sp.Say(hello.c_str());

  PASS();
  usleep(1000000);

  TEST1("writing data (attempt %d)", 2);

  sp.Say(numbers.c_str());

  PASS();
  
  return(0);
}

