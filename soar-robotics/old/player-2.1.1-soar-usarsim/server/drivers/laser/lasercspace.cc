/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey   &  Kasper Stoy
 *                      gerkey@usc.edu    kaspers@robotics.usc.edu
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
///////////////////////////////////////////////////////////////////////////
//
// Desc: Driver for computing the config space for a laser scan.
// Author: Andrew Howard
// Date: 1 Sep 2002
// CVS: $Id: lasercspace.cc 4232 2007-11-01 22:16:23Z gerkey $
//
// Theory of operation - Shortens each range reading in the laser scan
// such that the new scan delimits the boundary of free configuration
// space (for a robot of some known radius).
//
// Requires - Laser device.
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_lasercspace lasercspace
 * @brief Laser configuration space

The lasercspace driver processes a laser scan to compute the
configuration space (`C-space') boundary.  That is, it shortens the
range of each laser scan such that the resultant scan delimits the
obstacle-free portion of the robot's configuration space.  This driver
is particular useful for writing obstacle avoidance algorithms, since the
robot may safely move to any point in the obstacle-free portion of the 
configuration space.

Note that driver computes the configuration space for a robot of some
fixed radius; this radius may be set in the configuration file.

@image html lasercspace-1.jpg "Standard laser scan"
@image html lasercspace-2.jpg "Corresponding C-space scan for a robot of 0.5 m"

@par Compile-time dependencies

- none

@par Provides

- @ref interface_laser : output of the C-space scan

@par Requires

- @ref interface_laser : raw laser data from which to make C-space scan

@par Configuration requests

- PLAYER_LASER_REQ_GET_GEOM
  
@par Configuration file options

- radius (length)
  - Default: 0.5 m
  - Radius of robot for which to make C-space scan
- step (integer)
  - Default: 1
  - Step size for subsampling the scan (saves CPU cycles)
      
@par Example 

@verbatim
driver
(
  name "sicklms200"
  provides ["laser:0"]
  port "/dev/ttyS0"
)
driver
(
  name "lasercspace"
  requires ["laser:0"] # read from laser:0
  provides ["laser:1"] # output results on laser:1
  radius 0.5
)
@endverbatim

@author Andrew Howard

*/
/** @} */

#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>       // for atoi(3)
#include <unistd.h>

#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

#include "lasertransform.h"

// Driver for computing the free c-space from a laser scan.
class LaserCSpace : public LaserTransform
{
  // Constructor
  public: LaserCSpace( ConfigFile* cf, int section);

  // Process laser data.  Returns non-zero if the laser data has been
  // updated.
  private: int UpdateLaser(player_laser_data_t * data);

  // Pre-compute a bunch of stuff
  private: void Precompute(player_laser_data_t* data);

  // Compute the maximum free-space range for sample n.
  private: double FreeRange(player_laser_data_t* data, int n);


  // Step size for subsampling the scan (saves CPU cycles)
  private: int sample_step;

  // Robot radius.
  private: double radius;

  // Lookup table for precomputations
  private: double (*lu)[4];

};


// Initialization function
Driver* LaserCSpace_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new LaserCSpace( cf, section)));
}


// a driver registration function
void LaserCSpace_Register(DriverTable* table)
{
  table->AddDriver("lasercspace", LaserCSpace_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
LaserCSpace::LaserCSpace( ConfigFile* cf, int section)
  : LaserTransform(cf, section)
{
  // Settings.
  this->radius = cf->ReadLength(section, "radius", 0.50);
  this->sample_step = cf->ReadInt(section, "step", 1);
  
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Process laser data.
int LaserCSpace::UpdateLaser(player_laser_data_t * data)
{
  unsigned int i;
  
  // Construct the outgoing laser packet
  this->data.resolution = data->resolution;
  this->data.min_angle = data->min_angle;
  this->data.max_angle = data->max_angle;
  this->data.max_range = data->max_range;
  this->data.ranges_count = data->ranges_count;
  this->data.ranges = new float [data->ranges_count];
  this->lu = new double[data->ranges_count][4];

  // Do some precomputations to save time
  this->Precompute(data);

  // Generate the range estimate for each bearing.
  for (i = 0; i < data->ranges_count; i++)
    this->data.ranges[i]  = this->FreeRange(data,i);

  this->Publish(this->device_addr,  
                PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN,
                (void*)&this->data);
  delete [] this->data.ranges;
  delete [] this->lu;

  return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Pre-compute a bunch of stuff
void LaserCSpace::Precompute(player_laser_data_t* data)
{
  unsigned int i;
  double r, b, x, y;
  
  for (i = 0; i < data->ranges_count; i++)
  {
    r = data->ranges[i];
    b = data->min_angle + data->resolution * i;
    x = r * cos(b);
    y = r * sin(b);

    this->lu[i][0] = r;
    this->lu[i][1] = b;
    this->lu[i][2] = x;
    this->lu[i][3] = y;
  }
  return;
}
  

////////////////////////////////////////////////////////////////////////////////
// Compute the maximum free-space range for sample n.
double LaserCSpace::FreeRange(player_laser_data_t* data, int n)
{
  unsigned int i; 
  int step;
  double r, b, x, y;
  double r_, b_, x_, y_;
  double s, nr, nx, ny, dx, dy;
  double d, h;
  double max_r;

  // Step size for subsampling the scan (saves CPU cycles)
  step = this->sample_step;
  
  // Range and bearing of this reading.
  r = this->lu[n][0];
  b = this->lu[n][1];
  x = this->lu[n][2];
  y = this->lu[n][3];  

  max_r = r - this->radius;

  // Look for intersections with obstacles.
  for (i = 0; i < data->ranges_count; i += step)
  {
    r_ = this->lu[i][0];
    if (r_ - this->radius > max_r)
      continue;
    b_ = this->lu[i][1];
    x_ = this->lu[i][2];
    y_ = this->lu[i][3];  

    // Compute parametric point on ray that is nearest the obstacle.
    s = (x * x_ + y * y_) / (x * x + y * y);
    if (s < 0 || s > 1)
      continue;

    // Compute the nearest point.
    nr = s * r;
    nx = s * x;
    ny = s * y;

    // Compute distance from nearest point to obstacle.
    dx = nx - x_;
    dy = ny - y_;
    d = sqrt(dx * dx + dy * dy);
    
    if (d > this->radius)
      continue;
    
    // Compute the shortened range.
    h = nr - sqrt(this->radius * this->radius - d * d);
    if (h < max_r)
      max_r = h;
  }

  // Clip negative ranges.
  if (max_r < 0)
    max_r = 0;

  return max_r;
}
