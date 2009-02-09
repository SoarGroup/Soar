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
// Desc: Driver for detecting retro-reflective markers in a laser scan.
// Author: Andrew Howard
// Date: 16 Aug 2002
// CVS: $Id: laserbar.cc 4232 2007-11-01 22:16:23Z gerkey $
//
// Theory of operation
//
// Parses a laser scan to find retro-reflective markers.  Currently only
// cylindrical markers are supported.
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_laserbar laserbar
 * @brief Laser bar detector.
 
The laser bar detector searches for retro-reflective targets in the
laser range finder data.  Targets can be either planar or cylindrical,
as shown below. For planar targets, the range, bearing and orientation
will be determined; for cylindrical targets, only the range and bearing
will be determined.  The target size and shape can be set in the
configuration file.

The range at which targets can be detected is dependant on the target
size, the angular resolution of the laser and the quality of the
retro-reflective material used on the target.

See also the @ref driver_laserbarcode and 
@ref driver_laservisualbarcode drivers.

@image html laservisualbeacon.jpg "A sample laser bar (ignore the colored bands)"

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

- width (length)
  - Default: 0.08 m
  - Target width.

- tol (length)
  - Default: 0.5 m
  - Tolerance.

@par Example

@verbatim
driver
(
  name "laserbar"
  requires ["laser:0"]
  provides ["fiducial:0"]
  width 0.2
)
@endverbatim

@author Andrew Howard
*/
/** @} */

#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>       // for atoi(3)
#include <netinet/in.h>   // for htons(3)
#include <unistd.h>

#define PLAYER_ENABLE_TRACE 0
#define PLAYER_ENABLE_MSG 0

#include <libplayercore/playercore.h>

/*#include "error.h"
#include "driver.h"
#include "devicetable.h"
#include "drivertable.h"
#include "clientdata.h"
*/

// Driver for detecting laser retro-reflectors.
class LaserBar : public Driver
{
  // Constructor
  public: LaserBar( ConfigFile* cf, int section);

  // Setup/shutdown routines.
  public: virtual int Setup();
  public: virtual int Shutdown();

  // Process incoming messages from clients 
  int ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data);

  // Main function for device thread.
  private: virtual void Main();

  // Data configuration.
  /*public: virtual size_t GetData(player_device_id_t id,
                                 void* dest, size_t len,
                                 struct timeval* timestamp);
  public: virtual int PutConfig(player_device_id_t id, void *client, 
                                void *src, size_t len,
                                struct timeval* timestamp);*/

  // Handle geometry requests.
  //private: void HandleGetGeom(void *client, void *request, int len);

  // Analyze the laser data and pick out reflectors.
  private: void Find();

  // Test a patch to see if it has valid moments.
  private: bool TestMoments(double mn, double mr, double mb, double mrr, double mbb);

  // Find the line of best fit for the given segment of the laser
  // scan.  Fills in the pose and pose uncertainty of the reflector
  // (range, bearing, orientation).
  private: void FitCircle(int first, int last,
                          double *pr, double *pb, double *po,
                          double *ur, double *ub, double *uo);

  // Add a item into the fiducial list.
  private: void Add(double pr, double pb, double po,
                    double ur, double ub, double uo);
  
  // Pointer to laser to get data from.
  private:
  Device *laser_device;
  player_devaddr_t laser_addr;

  // Reflector properties.
  private: double reflector_width;
  private: double reflector_tol;
  
  // Local copy of the current laser data.
  private: player_laser_data_t ldata;

  // Local copy of the current fiducial data.
  private: struct timeval ftimestamp;
  private: player_fiducial_data_t fdata;
  unsigned int fdata_allocated;
};


// Initialization function
Driver* LaserBar_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new LaserBar( cf, section)));
}


// a driver registration function
void LaserBar_Register(DriverTable* table)
{
  table->AddDriver("laserbar", LaserBar_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
LaserBar::LaserBar( ConfigFile* cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_FIDUCIAL_CODE) 
{

  // Must have an input laser
  if (cf->ReadDeviceAddr(&this->laser_addr, section, "requires",
                       PLAYER_LASER_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);    
    return;
  }

  // Default reflector properties.
  this->reflector_width = cf->ReadLength(section, "width", 0.08);
  this->reflector_tol = cf->ReadLength(section, "tol", 0.50);
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int LaserBar::Setup()
{
  fdata_allocated = 0;
  fdata.fiducials = NULL;
  this->laser_device = deviceTable->GetDevice(this->laser_addr);

  if (!this->laser_device)
  {
    PLAYER_ERROR("unable to locate suitable laser device");
    return(-1);
  }

  if (this->laser_device->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to laser device");
    return -1;
  }
    
  // Subscribe to the laser device, but fail if it fails
/*  if (BaseClient->Subscribe(this->laser_id) != 0)
  {
    PLAYER_ERROR("unable to subscribe to laser device");
    return(-1);
  }*/

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int LaserBar::Shutdown()
{
  this->laser_device->Unsubscribe(this->InQueue);
  this->laser_device = NULL;

  free(fdata.fiducials);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void LaserBar::Main() 
{
  while (true)
  {
    // Let the laser drive update rate
    this->Wait();

    // Test if we are supposed to cancel this thread.
    pthread_testcancel();

    // Process any pending requests.
    this->ProcessMessages();

    //usleep(100000);
  }

  return;
}



////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int LaserBar::ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data)
{

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA, 
                            PLAYER_LASER_DATA_SCAN, this->laser_addr))
  {
  	//assert(hdr->size == sizeof(player_laser_data_t));
  	player_laser_data_t *laser_data = reinterpret_cast<player_laser_data_t * > (data);

  	this->Lock();

    this->ldata.min_angle = laser_data->min_angle;
    this->ldata.max_angle = laser_data->max_angle;
    this->ldata.resolution = laser_data->resolution;
    this->ldata.max_range = laser_data->max_range;
    this->ldata.ranges_count = laser_data->ranges_count;
    this->ldata.intensity_count = laser_data->intensity_count;

    for (unsigned int i = 0; i < laser_data->ranges_count; i++)
    {
      this->ldata.ranges[i] = laser_data->ranges[i];
      this->ldata.intensity[i] = laser_data->intensity[i];
    }

    // Analyse the laser data
    this->Find();


  	this->Unlock();

    printf("Count[%d]\n",this->fdata.fiducials_count);

    this->Publish(this->device_addr, 
                  PLAYER_MSGTYPE_DATA, PLAYER_FIDUCIAL_DATA_SCAN, 
                  reinterpret_cast<void*>(&this->fdata),0, &hdr->timestamp);

  	return 0;
  }
 
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                            PLAYER_FIDUCIAL_REQ_GET_GEOM, this->device_addr))
  {
    Message *msg;

    player_fiducial_geom_t fgeom;

    if (!(msg = this->laser_device->Request(this->InQueue, PLAYER_MSGTYPE_REQ, PLAYER_LASER_REQ_GET_GEOM, NULL, 0, NULL, false)))
    {
      PLAYER_WARN("failed to get laer geometry");
      memset(&fgeom.pose,0,sizeof(fgeom.pose));
    }
    else
    {
      player_laser_geom_t *lgeom = (player_laser_geom_t*)msg->GetPayload();

      fgeom.pose = lgeom->pose;
      fgeom.size = lgeom->size;
    }

    fgeom.fiducial_size.sw = this->reflector_width;
    fgeom.fiducial_size.sl = this->reflector_width;

    delete msg;

    this->Publish(this->device_addr, resp_queue, 
                  PLAYER_MSGTYPE_RESP_ACK, PLAYER_FIDUCIAL_REQ_GET_GEOM, 
                  (void*)&fgeom);

    return 0;
  }

  return -1;
}


////////////////////////////////////////////////////////////////////////////////
// Analyze the laser data to find reflectors.
void LaserBar::Find()
{
  unsigned int i;
  int h;
  double r, b;
  double mn, mr, mb, mrr, mbb;
  double pr, pb, po;
  double ur, ub, uo;

  // Empty the fiducial list.
  this->fdata.fiducials_count = 0;
  
  // Initialise patch statistics.
  mn = 0.0;
  mr = 0.0;
  mb = 0.0;
  mrr = 0.0;
  mbb = 0.0;
    
  // Look for a candidate patch in scan.
  for (i = 0; i < this->ldata.ranges_count; i++)
  {
    r = (double) (this->ldata.ranges[i]);
    b = (double) (this->ldata.min_angle + i * this->ldata.resolution);
    h = (int) (this->ldata.intensity[i]);

    // If there is a reflection...
    if (h > 0)
    {
      mn += 1;
      mr += r;
      mb += b;
      mrr += r * r;
      mbb += b * b;
    }

    // If there is no reflection and we have a patch...
    else if (mn > 0)
    {
      // Compute the moments of the patch.
      mr /= mn;
      mb /= mn;
      mrr = mrr / mn - mr * mr;
      mbb = mbb / mn - mb * mb;
      
      // Apply tests to see if this is a sensible looking patch.
      if (this->TestMoments(mn, mr, mb, mrr, mbb))
      {
        // Do a best fit to determine the pose of the reflector.
        this->FitCircle(i - (int) mn, i - 1, &pr, &pb, &po, &ur, &ub, &uo);

        // Fill in the fiducial data structure.
        this->Add(pr, pb, po, ur, ub, uo);
      }
      
      mn = 0.0;
      mr = 0.0;
      mb = 0.0;
      mrr = 0.0;
      mbb = 0.0;
    }
  }
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Test a patch to see if it has valid moments.
bool LaserBar::TestMoments(double mn, double mr, double mb, double mrr, double mbb)
{
  double dr, db;
  
  //printf("Testing moments %.0f %f %f %f %f\n", mn, mr, mb, mrr, mbb);

  if (mn < 2.0)
    return false;

  // These are tests for a cylindrical reflector.
  dr = (1 + this->reflector_tol) * this->reflector_width / 2;
  db = (1 + this->reflector_tol) * atan2(this->reflector_width / 2, mr);
  if (mrr > dr * dr)
    return false;
  if (mbb > db * db)
    return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////
// Find the line of best fit for the given segment of the laser scan.
// Fills in the pose and pose uncertainty of the reflector (range,
// bearing, orientation).  This one works for cylindrical fiducials.
void LaserBar::FitCircle(int first, int last,
                               double *pr, double *pb, double *po,
                               double *ur, double *ub, double *uo)
{
  int i;
  double r, b;
  double mn, mr, mb;

  mn = 0.0;
  mr = 1e6;
  mb = 0.0;

  for (i = first; i <= last; i++)
  {
    r = (double) (this->ldata.ranges[i]);
    b = (double) (this->ldata.min_angle + i * this->ldata.resolution);

    if (r < mr)
      mr = r;
    mn += 1.0;
    mb += b;
  }

  mr += this->reflector_width / 2;
  mb /= mn;

  *pr = mr;
  *pb = mb;
  *po = 0.0;

  // TODO: put in proper uncertainty estimates.
  *ur = 0.02;  
  *ub = this->ldata.resolution;
  *uo = 1e6;
  
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Add a item into the fiducial list.
void LaserBar::Add(double pr, double pb, double po,
                         double ur, double ub, double uo)

{
  player_fiducial_item_t *fiducial;
  this->fdata.fiducials_count++;
  if (this->fdata.fiducials_count > this->fdata_allocated)
  {
    this->fdata_allocated = this->fdata.fiducials_count;
    this->fdata.fiducials = (player_fiducial_item_t*)realloc(this->fdata.fiducials, sizeof(this->fdata.fiducials[0])*this->fdata_allocated);
  }
  fiducial = &this->fdata.fiducials[fdata.fiducials_count-1]; 
  fiducial->id = (int16_t) -1;

  fiducial->pose.px = pr * cos(pb);
  fiducial->pose.py = pr * sin(pb);
  fiducial->pose.pyaw = po;

  return;
}


