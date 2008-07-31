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
// Desc: Driver for detecting laser barcodes.
// Author: Andrew Howard
// Date: 29 Jan 2001
// CVS: $Id: laserbarcode.cc 4232 2007-11-01 22:16:23Z gerkey $
//
// Theory of operation:
//  Will detect binary coded beacons (i.e. bar-codes) in laser data.
//  Reflectors represent '1' bits, non-reflectors represent '0' bits.
//  The first and last bits of the beacon must be '1'.
//
// Requires: laser
// Provides: fiducial
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_laserbarcode laserbarcode
 * @brief Laser barcode detector

@todo This driver has not been tested with the player 2.0 API

The laser barcode detector searches for specially constructed barcodes in
the laser range finder data.  An example laser barcode is shown below.
The barcode is constructed using strips of retro-reflective paper.
Each retro-reflective strip represents a `1' bit; each non-reflective
strip represents a `0' bit.  By default, the laserbarcode driver
searches for barcodes containing 8 bits, each of which is exactly 50mm
wide (the total barcode width is thus 400mm).  The first and last bits
are used as start and end markers, and the remaining bits are used to
determine the identity of the barcode; with an 8-bit barcode there are
64 unique IDs.  The number of bits and the width of each bit can be set
in the configuration file.

The range at which barcodes can be detected identified is dependent on the
bit width and the angular resolution of the laser.  With 50mm bits and an
angular resolution of 0.5 deg, barcodes can be detected and identified
at a range of about 2.5m.  With the laser resolution set to  0.25 deg,
this distance is roughly doubled to about 5m.

See also the @ref driver_laserbar and
@ref driver_laservisualbarcode drivers.

@image html beacon.jpg "A sample laser barcode.  This barcode has 8 bits, each of which is 50mm wide."

@par Compile-time dependencies

- none

@par Provides

- This driver provides detected target information through a @ref
  interface_fiducial device.

@par Requires

- This driver finds targets in scans from a @ref interface_laser
  device.

@par Configuration requests

- PLAYER_FIDUCIAL_REQ_GET_GEOM

@par Configuration file options

- bit_count (integer)
  - Default: 8
  - Number of bits in each barcode.
- bit_width (length)
  - Default: 0.05 m
  - Width of each bit.
- max_depth (length)
  - Default: 0.05 m
  - Maximum variance in the flatness of the beacon.
- accept_thresh (float)
  - Default: 1.0
  - Acceptance threshold
- zero_thresh (float)
  - Default: 0.6
  - Zero threshold
- one_thresh (float)
  - Default: 0.6
  - One threshold

@par Example

@verbatim
driver
(
  name "laserbarcode"
  requires ["laser:0"]
  provides ["fiducial:0"]
  bit_count 5
  bit_width 0.1
)
@endverbatim

@author Andrew Howard
*/
/** @} */

#define PLAYER_ENABLE_TRACE 0
#define PLAYER_ENABLE_MSG 0

#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>  // for atoi(3)
#include <netinet/in.h>  /* for htons(3) */
#include <unistd.h>

#include <libplayercore/playercore.h>

// The laser barcode detector.
class LaserBarcode : public Driver
{
  // Constructor
  public: LaserBarcode( ConfigFile* cf, int section);

  // Setup/shutdown routines
  //
  public: virtual int Setup();
  public: virtual int Shutdown();

  // Process incoming messages from clients 
  int ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data);

  // Main function for device thread
  //private: virtual void Main(void);

  // Get the laser data
  //private: int ReadLaser();

  // Analyze the laser data and return beacon data
  private: void FindBeacons(const player_laser_data_t *laser_data,
                            player_fiducial_data_t *beacon_data);

  // Analyze the candidate beacon and return its id (0 == none)
  private: int IdentBeacon(int a, int b, double ox, double oy, double oth,
                           const player_laser_data_t *laser_data);

  // Write fidicual data 
  private: void WriteFiducial();

  // Pointer to laser to get data from
  private: player_devaddr_t laser_id;
  private: Device *laser;
  
  // Magic numbers
  private: int bit_count;
  private: double bit_width;
  private: double max_depth;
  private: double accept_thresh, zero_thresh, one_thresh;

  // Current laser data
  private: player_laser_data_t laser_data;
  private: struct timeval laser_timestamp;
  
  // Current fiducial data
  private: player_fiducial_data_t data;
  unsigned int fdata_allocated;
};


// Initialization function
Driver* LaserBarcode_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new LaserBarcode( cf, section)));
}

// a driver registration function
void LaserBarcode_Register(DriverTable* table)
{
  table->AddDriver("laserbarcode", LaserBarcode_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
LaserBarcode::LaserBarcode( ConfigFile* cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_FIDUCIAL_CODE)
{
  // Must have an input laser
  if (cf->ReadDeviceAddr(&this->laser_id, section, "requires",
                       PLAYER_LASER_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);    
    return;
  }

  // Get beacon settings.
  this->bit_count = cf->ReadInt(section, "bit_count", 8);
  this->bit_width = cf->ReadLength(section, "bit_width", 0.05);
  
  // Maximum variance in the flatness of the beacon
  this->max_depth = cf->ReadLength(section, "max_depth", 0.05);

  // Default thresholds
  this->accept_thresh = cf->ReadFloat(section, "accept_thresh", 1.0);
  this->zero_thresh = cf->ReadFloat(section, "zero_thresh", 0.60);
  this->one_thresh = cf->ReadFloat(section, "one_thresh", 0.60);

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device
int LaserBarcode::Setup()
{
  fdata_allocated = 0;
  data.fiducials = NULL;

  // Subscribe to the laser.
  if (Device::MatchDeviceAddress (laser_id, device_addr))
  {
    PLAYER_ERROR ("attempt to subscribe to self");
    return -1;
  }
  if (!(laser = deviceTable->GetDevice (laser_id)))
  {
    PLAYER_ERROR ("unable to locate suitable camera device");
    return -1;
  }
  if (laser->Subscribe (InQueue) != 0)
  {
    PLAYER_ERROR ("unable to subscribe to camera device");
    return -1;
  }

  
  PLAYER_MSG2(2, "laserbarcode device: bitcount [%d] bitwidth [%fm]",
              this->bit_count, this->bit_width);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int LaserBarcode::Shutdown()
{

  // Unsubscribe from devices.
  laser->Unsubscribe(InQueue);

  free(data.fiducials);

  PLAYER_MSG0(2, "laserbarcode device: shutdown");
  return 0;
}



////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int LaserBarcode::ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
  assert(hdr);
  assert(data);
  
  if(Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN, laser_id))
  {
    assert(hdr->size == sizeof(player_laser_data_t));
    laser_data = *reinterpret_cast<player_laser_data_t * > (data);

    // Analyse the laser data
    this->FindBeacons(&this->laser_data, &this->data);

    // Write out the fiducials
    this->WriteFiducial();

    return 0;
  }
 
  return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Analyze the laser data and return beacon data
void LaserBarcode::FindBeacons(const player_laser_data_t *laser_data,
                               player_fiducial_data_t *data)
{
  data->fiducials_count = 0;

  int ai = -1;
  int bi = -1;
  double ax, ay;
  double bx, by;

  // Expected width of beacon
  //
  double min_width = (this->bit_count - 1) * this->bit_width;
  double max_width = (this->bit_count + 1) * this->bit_width;

  ax = ay = 0;
  bx = by = 0;
    
  // Find the beacons in this scan
  for (unsigned int i = 0; i < laser_data->ranges_count; i++)
  {
    double range = (laser_data->ranges[i]);
    double bearing = (laser_data->min_angle + i * laser_data->resolution);
    int intensity = (laser_data->intensity[i]);

    double px = range * cos(bearing);
    double py = range * sin(bearing);
        
    if (intensity > 0)
    {
      if (ai < 0)
      {
        ai = i;
        ax = px;
        ay = py;
      }
      bi = i;
      bx = px;
      by = py;
    }
    if (ai < 0)
      continue;

    double width = sqrt((px - ax) * (px - ax) + (py - ay) * (py - ay));
    if (width < max_width)
      continue;

    width = sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));
    if (width < min_width)
      continue;
    if (width > max_width)
    {
      ai = -1;
      continue;
    }

    // Assign an id to the beacon
    double orient = atan2(by - ay, bx - ax);
    int id = IdentBeacon(ai, bi, ax, ay, orient, laser_data);

    // Reset counters so we can find new beacons
    //
    ai = -1;
    bi = -1;

    // Ignore invalid ids
    //
    if (id < 0)
      continue;

    if (this->data.fiducials_count+1 > this->fdata_allocated)
    {
      this->fdata_allocated = this->data.fiducials_count+1;
      this->data.fiducials = (player_fiducial_item_t*)realloc(this->data.fiducials, sizeof(this->data.fiducials[0])*this->fdata_allocated);
    }
    
    double ox = (bx + ax) / 2;
    double oy = (by + ay) / 2;
    range = sqrt(ox * ox + oy * oy);
    bearing = atan2(oy, ox);

    // Create an entry for this beacon.
    // Note that we return the surface normal for the beacon orientation.
    data->fiducials[data->fiducials_count].id = (id > 0 ? id : -1);
    data->fiducials[data->fiducials_count].pose.px = range * cos(bearing);
    data->fiducials[data->fiducials_count].pose.py = range * sin(bearing);
    data->fiducials[data->fiducials_count].pose.pyaw = NORMALIZE(orient + M_PI/2);
    data->fiducials_count++;
  }
}


////////////////////////////////////////////////////////////////////////////////
// Analyze the candidate beacon and return its id.
// Will return -1 if this is not a beacon.
// Will return  0 if this is a beacon, but it cannot be identified.
// Will return beacon id otherwise.
int LaserBarcode::IdentBeacon(int a, int b, double ox, double oy, double oth,
                              const player_laser_data_t *laser_data)
{
  // Compute pose of laser relative to beacon
  double lx = -ox * cos(-oth) + oy * sin(-oth);
  double ly = -ox * sin(-oth) - oy * cos(-oth);
  double la = -oth;

  // Initialise our probability distribution.
  // We determine the probability that each bit is set using Bayes law.
  double prob[8][2];
  for (int bit = 0; bit < ARRAYSIZE(prob); bit++)
    prob[bit][0] = prob[bit][1] = 0;

  // Scan through the readings that make up the candidate.
  for (int i = a; i <= b; i++)
  {
    double range = laser_data->ranges[i];
    double bearing = laser_data->min_angle + i * laser_data->resolution;
    int intensity = (int) (laser_data->intensity[i]);
    double res = laser_data->resolution;

    // Compute point relative to beacon
    double py = ly + range * sin(la + bearing);

    // Discard candidate points are not close to x-axis (ie candidate
    // is not flat).
    if (fabs(py) > this->max_depth)
      return -1;

    // Compute intercept with beacon
    //double cx = lx + ly * tan(la + bearing + M_PI/2);
    double ax = lx + ly * tan(la + bearing - res/2 + M_PI/2);
    double bx = lx + ly * tan(la + bearing + res/2 + M_PI/2);

    // Update our probability distribution
    // Use Bayes law
    for (int bit = 0; bit < this->bit_count; bit++)
    {
      // Use a rectangular distribution
      double a = (bit + 0.0) * this->bit_width;
      double b = (bit + 1.0) * this->bit_width;

      double p = 0;
      if (ax < a && bx > b)
        p = 1;
      else if (ax > a && bx < b)
        p = 1;
      else if (ax > b || bx < a)
        p = 0;
      else if (ax < a && bx < b)
        p = 1 - (a - ax) / (bx - ax);
      else if (ax > a && bx > b)
        p = 1 - (bx - b) / (bx - ax);
      else
        assert(0);

      //printf("prob : %f %f %f %f %f\n", ax, bx, a, b, p);
            
      if (intensity == 0)
      {
        assert(bit >= 0 && bit < ARRAYSIZE(prob));            
        prob[bit][0] += p;
        prob[bit][1] += 0;
      }
      else
      {
        assert(bit >= 0 && bit < ARRAYSIZE(prob));            
        prob[bit][0] += 0;
        prob[bit][1] += p;
      }
    }
  }

  // Now assign the id
  int id = 0;
  for (int bit = 0; bit < this->bit_count; bit++)
  {
    double pn = prob[bit][0] + prob[bit][1];
    double p0 = prob[bit][0] / pn;
    double p1 = prob[bit][1] / pn;
       
    if (pn < this->accept_thresh)
      id = -1;
    else if (p0 > this->zero_thresh)
      id |= (0 << bit);
    else if (p1 > this->one_thresh)
      id |= (1 << bit);
    else
      id = -1;

    //printf("%d %f %f : %f %f %f\n", bit, prob[bit][0], prob[bit][1], p0, p1, pn);
  }
  //printf("\n");

  if (id < 0)
    id = 0;
    
  return id;
}


////////////////////////////////////////////////////////////////////////////////
// Write fidicual data 
void LaserBarcode::WriteFiducial()
{
  // Write the data with the laser timestamp
  this->Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_FIDUCIAL_DATA_SCAN, &this->data);
  
  return;
}

