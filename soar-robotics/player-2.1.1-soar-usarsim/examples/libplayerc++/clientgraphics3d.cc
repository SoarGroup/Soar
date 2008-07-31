/*
 * laserobstacleavoid.cc
 *
 * a simple obstacle avoidance demo
 *
 * @todo: this has been ported to libplayerc++, but not tested
 */

#include <libplayerc++/playerc++.h>
#include <iostream>

#include "args.h"

#define RAYS 32

int
main(int argc, char **argv)
{
  parse_args(argc,argv);

  using namespace PlayerCc;

    PlayerClient robot(gHostname, gPort);
    Graphics3dProxy gp(&robot, gIndex);
    
    std::cout << robot << std::endl;
    
    player_point_3d_t pts[RAYS];

    double r;
    for( r=0; r<1.0; r+=0.05 )
      {
	int p;
	for( p=0; p<RAYS; p++ )
	  {
	    pts[p].px = r * cos(p * M_PI/(RAYS/2));
	    pts[p].py = r * sin(p * M_PI/(RAYS/2));
	    pts[p].pz = p/RAYS;
	  }	
	
	gp.Color( 255,0,0,0 );
	gp.Draw( PLAYER_DRAW_POINTS, pts, RAYS );

	usleep(500000);

	gp.Color( (int)(255.0*r),(int)(255-255.0*r),0,0 );
	gp.Draw( PLAYER_DRAW_LINE_STRIP, pts, RAYS/2 );

      }

    sleep(1);
    
    player_color_t col;
    memset( &col, 0, sizeof(col));

    for( r=3; r>0; r-=0.1 )
      {
	col.blue = (int)(r * 255.0 /3);
	col.red  = (int)((255.0 - r * 255.0)/3);
	
	player_point_3d_t pts[4];
	pts[0].px = -r;
	pts[0].py = -r;
	pts[0].pz = 1-r/3;
	pts[1].px = r;
	pts[1].py = -r;
	pts[1].pz = 1-r/3;
	pts[2].px = r;
	pts[2].py = r;
	pts[2].pz = 1-r/3;
	pts[3].px = -r;
	pts[3].py = r;
	pts[3].pz = 1-r/3;
	
	gp.Draw( PLAYER_DRAW_POLYGON, pts, 4);
	
	usleep(300000);
  }

  sleep(1);

  gp.Clear();

  return 0;
}
