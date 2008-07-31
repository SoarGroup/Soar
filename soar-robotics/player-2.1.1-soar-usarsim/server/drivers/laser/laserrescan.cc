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
// Desc: Driver for converting laser scan geometry
// Author: Toby Collett
// Date: 20 Sept 2005
//
// Theory of operation - linear interpolation of existing scans
// to create virtual scan geometry
//
// Requires - Laser device.
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_laserrescan laserrescan
 * @brief Laser rescanner

The laserrescan driver processes a laser scan to create a virtual scan
configuration. Existing scans are interpolated.

The driver was created for the purpose of using pmap with non-sick scanners

@par Compile-time dependencies

- none

@par Provides

- @ref interface_laser : output of the rescan

@par Requires

- @ref interface_laser : raw laser data from which to interpolate

@par Configuration requests

- PLAYER_LASER_REQ_GET_GEOM

@par Configuration file options
- min_angle (float)
  - Default: -pi/2
  - Minimum angle of the new scan data
- max_angle (float)
  - Default: pi/2
  - Maximum angle of the new scan data
- scan_count (int)
  - Default: 181
  - Number of scans from min angle to max angle

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
  name "laserrescan"
  requires ["laser:0"] # read from laser:0
  provides ["laser:1"] # output results on laser:1
)
@endverbatim

@author Toby Collett

*/
/** @} */

#include <libplayercore/playercore.h>

#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>       // for atoi(3)
#include <unistd.h>

#include "lasertransform.h"

// Driver for computing the free c-space from a laser scan.
class LaserRescan : public LaserTransform
{
  // Constructor
  public: LaserRescan( ConfigFile* cf, int section);

  // Process laser data.  Returns non-zero if the laser data has been
  // updated.
  protected: int UpdateLaser(player_laser_data_t * data);

  double min_angle, max_angle, res;
  int scan_count;
};


// Initialization function
Driver* LaserRescan_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new LaserRescan( cf, section)));
}


// a driver registration function
void LaserRescan_Register(DriverTable* table)
{
  table->AddDriver("laserrescan", LaserRescan_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
LaserRescan::LaserRescan( ConfigFile* cf, int section)
    : LaserTransform(cf, section)
{
  // Settings.
  this->max_angle = cf->ReadAngle(section, "max_angle", M_PI/2.0);
  this->min_angle = cf->ReadAngle(section, "min_angle", -M_PI/2.0);
  this->scan_count = cf->ReadInt(section, "scan_count", 181);
  res = (max_angle - min_angle) / scan_count;

  return;
}



////////////////////////////////////////////////////////////////////////////////
// Process laser data.
int LaserRescan::UpdateLaser(player_laser_data_t * data)
{
  int i;

  // Construct the outgoing laser packet
  this->data.resolution = (max_angle - min_angle)/((double)(scan_count-1));
  this->data.min_angle = (min_angle);
  this->data.max_angle = (max_angle);
  this->data.ranges_count = scan_count;
  this->data.ranges = new float[scan_count];
  this->data.max_range = data->max_range;

  double real_min = data->min_angle;
  double real_res = data->resolution;

	for (i = 0; i < scan_count; ++i)
	{
		double theta = min_angle + i*res;
		double new_i = (theta - real_min)/real_res;

		unsigned int j = (int) floor(new_i);
		unsigned int k = (int) ceil(new_i);

		if (j < 0)
			j = 0;
		if (j > data->ranges_count)
			j = data->ranges_count;

		if (k < 0)
			k = 0;
		if (k > data->ranges_count)
			k = data->ranges_count;


		double theta_j = real_min + j*real_res;

		double interpolate = ((real_min +new_i*real_res) - theta_j)/real_res;

		double new_value = data->ranges[j]+(data->ranges[k] - data->ranges[j])*interpolate;

		this->data.ranges[i] = new_value;

	}

  this->Publish(this->device_addr,
                PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN,
                &this->data);
  delete [] this->data.ranges;
  return 1;
}



