/*
 * $Id: test_sonar.cc 3971 2007-02-02 17:12:04Z gerkey $
 *
 * a test for the C++ SonarProxy
 */

#include "test.h"
#include <unistd.h>

int
test_sonar(PlayerClient* client, int index)
{
  TEST("sonar");
  SonarProxy sp(client,index);

  // wait for P2OS to start up, throwing away data as fast as possible
  for(int i=0; i < 60; i++)
  {
    client->Read();
  }

  sp.RequestGeom();

  /*
  for(int i=0;i<sp.pose_count;i++)
  {
    printf("Sonar[%d]: (%.3f,%.3f,%.3f)\n", i, 
           sp.poses[i][0],
           sp.poses[i][1],
           RTOD(sp.poses[i][2]));
  }
  */

  std::cout << "There are " << sp.GetCount() << " sonar sensors.\n";

  for(int t = 0; t < 3; t++)
  {
    TEST1("reading data (attempt %d)", t);

    client->Read();

    PASS();
    std::cout << sp << std::endl;

  }

  PASS();
  return 0;
}

