/*#include <stdlib.h>
#include "nav200.h"

#define XMIN 100
#define XMAX 1000

int main(int argc,char ** argv)
{
  Nav200 testing;
  testing.Initialise();
  if (!testing.EnterStandby())
  {
      fprintf(stderr,"unable to enter standby mode\n");
      return EXIT_FAILURE;
  }
  LaserPos laser;

  if(testing.EnterPositioning())
  {
      printf("\n\n\nEntered positioning mode\n\n");
      if(testing.SetActionRadii(XMIN,XMAX))
	printf("changed operation radii\n");
  }
  else
  {
      fprintf(stderr,"unable to enter positioning mode\n");
      return EXIT_FAILURE;
  }
  
  while(1)
  {
      if(testing.GetPositionAuto(laser))
	printf("Position of the laser scanner: X=%d, Y=%d, orientation=%d, quality=%d, number of reflectors = %d\n",laser.pos.x,laser.pos.y,laser.orientation,laser.quality,laser.number);
  }

  return 0;
}
*/

int main()
{}
