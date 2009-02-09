/*
 * $Id: test_gripper.cc 3971 2007-02-02 17:12:04Z gerkey $
 *
 * a test for the C++ SonarProxy
 */

#include "test.h"
#include <unistd.h>

using namespace PlayerCc;

int
test_gripper(PlayerClient* client, int index)
{
  GripperProxy gp(client,index);

  // wait for P2OS to start up
  for(int i=0; i < 20; i++)
    client->Read();

  for(int t = 0; t < 3; t++)
  {
    TEST1("reading data (attempt %d)", t);
    client->Read();

    std::cerr << "got gripper data: " << std::endl << gp << std::endl;

    PASS();
  }

  TEST("gripper open");
  gp.Open();
  sleep(5);
  PASS();

  TEST("gripper close");
  gp.Close();
  sleep(8);
  PASS();

  TEST("gripper open");
  gp.Open();
  sleep(5);
  PASS();

  TEST("gripper store object (only on some grippers, e.g. stage)");
  gp.Store();
  sleep(3);
  PASS();

  TEST("gripper retrieve object (only on some grippers, e.g. stage");
  gp.Retrieve();
  sleep(3);
  PASS();

  PASS();
  return(0);
}

