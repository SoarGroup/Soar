/*
 * $Id: test_bumper.cc 3971 2007-02-02 17:12:04Z gerkey $
 *
 * a test for the C++ BumperProxy
 */

#include "test.h"
#include <unistd.h>

int
test_bumper(PlayerClient* client, int index)
{
  TEST("bumper");
  BumperProxy sp(client,index);

  // wait for P2OS to start up
  for(int i=0; i < 20; i++)
    client->Read();

  /*
  TEST("getting bumper geometry");

  player_bumper_geom_t bumper_geom;
  sp.GetBumperGeom( &bumper_geom );
  sleep(1);
  PASS();
  printf( "Discovered %d bumper geometries\n", bumper_geom.bumper_count );
  for(int i=0;i<bumper_geom.bumper_count;i++)
    {
    printf("Bumper[%d]: (%4d,%4d,%4d) len: %4u radius: %4u\n", i, 
           bumper_geom.bumper_def[i].x_offset,
           bumper_geom.bumper_def[i].y_offset,
           bumper_geom.bumper_def[i].th_offset,
           bumper_geom.bumper_def[i].length,
           bumper_geom.bumper_def[i].radius );
  }
  */

  for(int t = 0; t < 3; t++)
  {
    TEST1("reading data (attempt %d)", t);

    client->Read();

    PASS();

    std::cerr << sp << std::endl;
    if(sp.IsAnyBumped()) {
      std::cerr << "A bumper switch is pressed.\n";
    }
  }


  PASS();

  return(0);
}

