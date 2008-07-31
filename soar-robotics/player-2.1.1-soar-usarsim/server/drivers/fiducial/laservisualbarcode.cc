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
// Desc: Driver for detecting combined laser/visual barcodes.
// Author: Andrew Howard
// Date: 17 Aug 2002
// CVS: $Id: laservisualbarcode.cc 4232 2007-11-01 22:16:23Z gerkey $
//
// Theory of operation:
//   Parses a laser scan to find the retro-reflective patches (lines or
//   circles), then points the camera at the patch, zooms in, and
//   attempts to read the colored barcode.  Will not return sensible
//   orientations for circular patches.
//
// Requires:
//   Laser, PTZ and blobfinder devices.
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_laservisualbarcode laservisualbarcode
 * @brief Color laser/visual barcode detector

@todo This driver has not been tested with the v2.0 API

The laser visual barcode detector uses both searches for fiducials that
are both retro-reflective and color-coded.  Fiducials can be either planar
or cylindical, as shown below.  For planar targets, the range, bearing,
orientation and identity will be determined; for cylindrical targets,
the orientation will be undefined.  The target size and shape can be
set in the configuration file.

The laser visual barcode detector searches the laser range data to
find retro-reflective targets, points the camera at each of these
targets in turn, then uses color information to determine the presence
and identity of fiducials.  Thus, this detector makes use of three
underlying devices: a laser range finder, a pan-tilt-zoom camera and a
color blob detector.  Note that the laser is used to determine the
geometry of the fidicual (range, bearing and orientation), while the
camera is used to determine its identity.

The range at which fiducials can be both detected and identified
depends on a number of factors, including the size of the fiducial and
the angular resolution of the laser.  Generally speaking, however,
this detector has better range than the @ref driver_laserbarcode
detector, but produces fewer observations.

See also the @ref driver_laserbar and @ref
driver_laserbarcode drivers.

@image html laservisualbeacon.jpg "A sample laser visual barcode."

@par Compile-time dependencies

- none

@par Provides

- This driver provides detected target information through a @ref
  interface_fiducial device.

@par Requires

- @ref interface_laser
- @ref interface_ptz
- @ref interface_blobfinder

@par Configuration requests

- PLAYER_FIDUCIAL_REQ_GET_GEOM

@par Configuration file options

- max_ptz_attention (float)
  - Default: 2.0
  - ??
- retire_time (float)
  - Default: 1.0
  - ??
- max_dist (float) (should be a length?)
  - Default: 0.2
  - ??
- bit_count (integer)
  - Default: 3
  - Number of bits in visual barcode
- bit_width (length)
  - Default: 0.08 m
  - Width of each bit in visual barcode
- bit_height (length)
  - Default: 0.02 m
  - Height of each bit in visual barcode

@par Example

@verbatim
driver
(
  name "laservisualbarcode"
  requires ["laser:0" "ptz:0" "blobfinder:0"]
  provides ["fiducial:0"]
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

#include <libplayercore/playercore.h>

// Driver for detecting laser retro-reflectors.
class LaserVisualBarcode : public Driver
{
  // Constructor
  public: LaserVisualBarcode( ConfigFile* cf, int section);

  // Setup/shutdown routines.
  public: virtual int Setup();
  public: virtual int Shutdown();

  // Process incoming messages from clients 
  int ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data);

  // Info on potential fiducials.
  private: struct fiducial_t
  {
    // Id (-1) if undetermined.
    int id;

    // Pose of fiducial.
    double pose[3];

    // Uncertainty in pose.
    double upose[3];

    // Time at which fiducial was last seen by the laser.
    double laser_time;

    // Time at which ptz selected this fiducial,
    // and the time at which the ptz locked on to this fiducial.
    double ptz_select_time;
    double ptz_lockon_time;

    // Time at which the fiducial was identified.
    double id_time;
  };

  // Info on valid blobs.
  private: struct blob_t
  {
    // Blob channel.
    int ch;
  
    // Blob position in image.
    int x, y;
  };

  // Process laser data.
  // Returns non-zero if the laser data has been updated.
  private: int UpdateLaser(player_laser_data_t * data, double timestamp);

  // Analyze the laser data to find fidicuials (reflectors).
  private: void FindLaserFiducials(double time, player_laser_data_t *data);
    
  // Find the line of best fit for the given segment of the laser
  // scan.  Fills in the pose of the reflector relative to the laser.
  private: void FitLaserFiducial(player_laser_data_t *data, int first, int last, double pose[3]);

  // Match a new laser fiducial against the ones we are already
  // tracking.  The pose is relative to the laser.
  private: void MatchLaserFiducial(double time, double pose[3]);

  // Retire fiducials we havent seen for a while.
  private: void RetireLaserFiducials(double time, player_laser_data_t *data);

  // Update the PTZ to point at one of the laser reflectors.
  private: int UpdatePtz(player_ptz_data_t * data, double timestamp);

  // Select a target fiducial for the PTZ to inspect.
  private: void SelectPtzTarget(double time, player_ptz_data_t *data);

  // Servo the PTZ to a target fiducial.
  private: void ServoPtz(double time, player_ptz_data_t *data);

  // Process any new blobfinder data.
  private: int UpdateBlobfinder(player_blobfinder_data_t * data, double timestamp);

  // Find blobs with valid properties.
  private: void FindBlobs(double time, player_blobfinder_data_t *data);

  // Search the list of valid blobs to locate the visual fiducials.
  private: int FindVisualFiducials(double time, player_blobfinder_data_t *data,
                                   int depth, blob_t *prevblob);
    
  // Update the device data (the data going back to the client).
  private: void UpdateData();
  
  // Fiducial properties.
  private: int barcount;
  private: double barwidth, barheight;

  // Max time to spend looking at a fiducial.
  private: double max_ptz_attention;

  // Retirement age for fiducials that havent been seen for a while.
  private: double retire_time;

  // Max distance between fiducials in successive laser scans.
  private: double max_dist;

  // Laser stuff.
  private: Device *laser;
  private: player_devaddr_t laser_id;
  private: double laser_time;

  // PTZ stuff
  private: Device *ptz;
  private: player_devaddr_t ptz_id;
  private: double ptz_time;

  // Blobfinder stuff.
  private: Device *blobfinder;
  private: player_devaddr_t blobfinder_id;
  private: double blobfinder_time;

  // List of currently tracked fiducials.
  private: int fiducial_count;
  private: fiducial_t fiducials[256];

  // The current selected fiducial for the ptz, the time at which we
  // selected it, and the time at which we first locked on to it.
  private: fiducial_t *ptz_fiducial;

  // Dimensions of the zoomed image for the target fiducial (m).
  private: double zoomwidth, zoomheight;

  // List of current valid blobs.
  private: int blob_count;
  private: blob_t blobs[256];
  
  // Local copy of the current fiducial data.
  private: player_fiducial_data_t fdata;
  int fdata_allocated;
};


// Initialization function
Driver* LaserVisualBarcode_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new LaserVisualBarcode( cf, section)));
}


// a driver registration function
void LaserVisualBarcode_Register(DriverTable* table)
{
  table->AddDriver("laservisualbarcode", LaserVisualBarcode_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
LaserVisualBarcode::LaserVisualBarcode( ConfigFile* cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_FIDUCIAL_CODE)
{
  // Must have an input laser
  if (cf->ReadDeviceAddr(&this->laser_id, section, "requires",
                       PLAYER_LASER_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);    
    return;
  }
  this->laser = NULL;
  this->laser_time = 0;

  // Must have a ptz
  if (cf->ReadDeviceAddr(&this->ptz_id, section, "requires",
                       PLAYER_PTZ_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);    
    return;
  }
  this->ptz = NULL;
  this->ptz_time = 0;

  // Must have a blobfinder
  if (cf->ReadDeviceAddr(&this->blobfinder_id, section, "requires",
                       PLAYER_BLOBFINDER_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);    
    return;
  }
  this->blobfinder = NULL;
  this->blobfinder_time = 0;

  this->max_ptz_attention = cf->ReadFloat(section, "max_ptz_attention", 2.0);
  this->retire_time = cf->ReadFloat(section, "retire_time", 1.0);
  this->max_dist = cf->ReadFloat(section, "max_dist", 0.2);
  
  // Default fiducial properties.
  this->barcount = cf->ReadInt(section, "bit_count", 3);
  this->barwidth = cf->ReadLength(section, "bit_width", 0.08);
  this->barheight = cf->ReadLength(section, "bit_height", 0.02);

  // Reset fiducial list.
  this->fiducial_count = 0;

  // Reset PTZ target.
  this->ptz_fiducial = NULL;

  // Reset blob list.
  this->blob_count = 0;

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int LaserVisualBarcode::Setup()
{
  fdata_allocated = 0;
  fdata.fiducials = NULL;
  
  // Subscribe to the laser.
  if (!(laser = deviceTable->GetDevice (laser_id)))
  {
    PLAYER_ERROR ("unable to locate suitable laser device");
    return -1;
  }
  if (laser->Subscribe (InQueue) != 0)
  {
    PLAYER_ERROR ("unable to subscribe to laser device");
    return -1;
  }

  // Subscribe to the ptz.
  if (!(ptz = deviceTable->GetDevice (ptz_id)))
  {
    PLAYER_ERROR ("unable to locate suitable ptz device");
    return -1;
  }
  if (ptz->Subscribe (InQueue) != 0)
  {
    PLAYER_ERROR ("unable to subscribe to ptz device");
    return -1;
  }

  // Subscribe to the blobfinder.
  if (!(blobfinder = deviceTable->GetDevice (blobfinder_id)))
  {
    PLAYER_ERROR ("unable to locate suitable blobfinder device");
    return -1;
  }
  if (blobfinder->Subscribe (InQueue) != 0)
  {
    PLAYER_ERROR ("unable to subscribe to blobfinder device");
    return -1;
  }

  // Reset blob list.
  this->blob_count = 0;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int LaserVisualBarcode::Shutdown()
{
  // Unsubscribe from devices.
  laser->Unsubscribe(InQueue);
  ptz->Unsubscribe(InQueue);
  blobfinder->Unsubscribe(InQueue);

  free(fdata.fiducials);

  return 0;
}



////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int LaserVisualBarcode::ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
  assert(hdr);
  assert(data);
  
  if(Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN, laser_id))
  {
    assert(hdr->size == sizeof(player_laser_data_t));
    player_laser_data_t * l_data = reinterpret_cast<player_laser_data_t * > (data);

    UpdateLaser(l_data, hdr->timestamp);

    return 0;
  }

  if(Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, PLAYER_PTZ_DATA_STATE, ptz_id))
  {
    assert(hdr->size == sizeof(player_ptz_data_t));
    UpdatePtz(reinterpret_cast<player_ptz_data_t * > (data), hdr->timestamp);
    return 0;
  }

  if(Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, PLAYER_BLOBFINDER_DATA_BLOBS, blobfinder_id))
  {
    assert(hdr->size == sizeof(player_blobfinder_data_t));
    UpdateBlobfinder(reinterpret_cast<player_blobfinder_data_t * > (data), hdr->timestamp);
    return 0;
  }


  return -1;
}


////////////////////////////////////////////////////////////////////////////////
// Process laser data.
int LaserVisualBarcode::UpdateLaser(player_laser_data_t * data, double timestamp)
{
  this->laser_time = timestamp;
  
  // Find possible fiducials in this scan.
  this->FindLaserFiducials(timestamp, data);

  // Retire fiducials we havent seen for a while.
  this->RetireLaserFiducials(timestamp, data);

  return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Analyze the laser data to find fidicuials (reflectors).
void LaserVisualBarcode::FindLaserFiducials(double time, player_laser_data_t *data)
{
  unsigned int i;
  int h;
  int valid;
  double r, b;
  double db, dr;
  double mn, mr, mb, mrr, mbb;
  double pose[3];

  // Empty the fiducial list.
  this->fdata.fiducials_count = 0;
  
  // Initialise patch statistics.
  mn = 0.0;
  mr = 0.0;
  mb = 0.0;
  mrr = 0.0;
  mbb = 0.0;
    
  // Look for a candidate patch in scan.
  for (i = 0; i < data->ranges_count; i++)
  {
    r = (double) (data->ranges[i]);
    b = (double) (data->min_angle + i * data->resolution);
    h = (int) (data->intensity[i]);

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

      // Test moments to see if they are valid.
      valid = 1;
      valid &= (mn >= 1.0);
      dr = this->barwidth / 2;
      db = atan2(this->barwidth / 2, mr);
      valid &= (mrr < (dr * dr));
      valid &= (mbb < (db * db));
      
      if (valid)
      {
        // Do a best fit to determine the pose of the reflector.
        this->FitLaserFiducial(data, i - (int) mn, i - 1, pose);

        // Match this fiducial against the ones we are already tracking.
        this->MatchLaserFiducial(time, pose);
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
// Find the line of best fit for the given segment of the laser scan.
// Fills in the pose and pose of the reflector relative to the laser.
void LaserVisualBarcode::FitLaserFiducial(player_laser_data_t *data,
                                          int first, int last, double pose[3])
{
  int i;
  double r, b;
  double mn, mr, mb;

  mn = 0.0;
  mr = 1e6;
  mb = 0.0;

  for (i = first; i <= last; i++)
  {
    r = (double) (data->ranges[i]) / 1000;
    b = (double) (data->min_angle + i * data->resolution) / 100.0 * M_PI / 180;

    if (r < mr)
      mr = r;
    mn += 1.0;
    mb += b;
  }

  mr += this->barwidth / 2;
  mb /= mn;

  pose[0] = mr * cos(mb);
  pose[1] = mr * sin(mb);
  pose[2] = mb;

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Match a new laser fiducial against the ones we are already
// tracking.  The pose is relative to the laser.
void LaserVisualBarcode::MatchLaserFiducial(double time, double pose[3])
{
  int i;
  double dx, dy, dr;
  double mindr;
  fiducial_t *fiducial;
  fiducial_t *minfiducial;
  
  // Observations must be at least this close to the existing
  // fiducial.
  mindr = this->max_dist;
  minfiducial = NULL;

  // Find the existing fiducial which is closest to the new
  // observation.
  for (i = 0; i < this->fiducial_count; i++)
  {
    fiducial = this->fiducials + i;

    dx = pose[0] - fiducial->pose[0];
    dy = pose[1] - fiducial->pose[1];
    dr = sqrt(dx * dx + dy * dy);
    if (dr < mindr)
    {
      mindr = dr;
      minfiducial = fiducial;
    }
  }

  // If we didnt find a matching fiducial, add a new one.
  if (minfiducial == NULL)
  {
    this->fiducial_count++;
    if (this->fiducial_count+1 > this->fdata_allocated)
    {
      this->fdata_allocated = this->fiducial_count+1;
      this->fdata.fiducials = (player_fiducial_item_t*)realloc(this->fdata.fiducials, sizeof(this->fdata.fiducials[0])*this->fdata_allocated);
    }
    minfiducial = &this->fiducials[this->fiducial_count-1];
    minfiducial->id = -1;
    minfiducial->pose[0] = pose[0];
    minfiducial->pose[1] = pose[1];
    minfiducial->pose[2] = pose[2];
    minfiducial->laser_time = time;
    minfiducial->ptz_select_time = -1;
    minfiducial->ptz_lockon_time = -1;
    minfiducial->id_time = -1;
  }

  // Otherwise, update the existing fiducial.
  else
  {
    minfiducial->pose[0] = pose[0];
    minfiducial->pose[1] = pose[1];
    minfiducial->pose[2] = pose[2];
    minfiducial->laser_time = time;
  }
  
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Retire fiducials we havent seen for a while.
void LaserVisualBarcode::RetireLaserFiducials(double time, player_laser_data_t *data)
{
  int i;
  fiducial_t *fiducial;
  
  // Remove any old fiducials.
  for (i = 0; i < this->fiducial_count; i++)
  {
    fiducial = this->fiducials + i;

    if (time - fiducial->laser_time > this->retire_time)
    {
      if (this->ptz_fiducial == fiducial)
        this->ptz_fiducial = NULL;
      memmove(fiducial, fiducial + 1, (this->fiducial_count - i - 1) * sizeof(fiducial_t));
      this->fiducial_count--;
      i--;
    }
  }
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Update the PTZ to point at one of the laser reflectors.
int LaserVisualBarcode::UpdatePtz(player_ptz_data_t * data, double timestamp)
{
  this->ptz_time = timestamp;
  
  // Pick a fiducial to look at.
  this->SelectPtzTarget(timestamp, data);

  // Point the fiducial
  this->ServoPtz(timestamp, data);

  return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Select a target fiducial for the PTZ to inspect.
// This algorithm picks the one that we havent looked at for a long time.
void LaserVisualBarcode::SelectPtzTarget(double time, player_ptz_data_t *data)
{
  int i;
  double t, maxt;
  fiducial_t *fiducial;

  // Consider the currently selected target for a while to
  // give the blobfinder time to identify it.
  if (this->ptz_fiducial != NULL)
  {
    if (time - this->ptz_fiducial->ptz_select_time < this->max_ptz_attention)
      return;
  }

  // Find one we havent looked at for while.
  this->ptz_fiducial = NULL;
  maxt = -1;
  
  for (i = 0; i < this->fiducial_count; i++)
  {
    fiducial = this->fiducials + i;

    t = time - fiducial->ptz_select_time;
    if (t > maxt)
    {
      maxt = t;
      this->ptz_fiducial = fiducial;
    }
  }

  if (this->ptz_fiducial)
  {
    this->ptz_fiducial->ptz_select_time = time;
    this->ptz_fiducial->ptz_lockon_time = -1;
  }
  
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Servo the PTZ to a target fiducial.
void LaserVisualBarcode::ServoPtz(double time, player_ptz_data_t *data)
{
  double dx, dy, r, pan, tilt, zoom;
  fiducial_t *fiducial;
  player_ptz_cmd_t cmd;
  double maxtilt;
  double deadpan, deadzoom;

  // Max tilt value.
  maxtilt = 5 * M_PI / 180;

  // Deadband values.
  deadpan = 2;
  deadzoom = 2;
  
  fiducial = this->ptz_fiducial;
  if (fiducial == NULL)
  {
    r = 0;
    pan = 0;
    tilt = 0;
    zoom = M_PI;
  }
  else
  {
    // Compute range and bearing of fiducial relative to camera.
    // TODO: account for camera geometry.
    dx = fiducial->pose[0];
    dy = fiducial->pose[1];
    r = sqrt(dx * dx + dy * dy);
    pan = atan2(dy, dx);
    zoom = 8 * atan2(this->barwidth / 2, r);

    // See if we have locked on yet.
    if (fiducial->ptz_lockon_time < 0)
      if (fabs(pan * 180 / M_PI - data->pan) < deadpan &&
          fabs(zoom * 180 / M_PI - data->zoom) < deadzoom)
        fiducial->ptz_lockon_time = time;

    // If we havent locked on yet...
    if (fiducial->ptz_lockon_time < 0)
      tilt = 0;
    else
      tilt = maxtilt * sin((time - fiducial->ptz_lockon_time) /
                           this->max_ptz_attention * 2 * M_PI);
  }
  
  // Compose the command packet to send to the PTZ device.
  cmd.pan = pan;
  cmd.tilt = tilt;
  cmd.zoom = zoom;
  
  this->ptz->PutMsg(InQueue, PLAYER_MSGTYPE_CMD, PLAYER_PTZ_CMD_STATE, &cmd, sizeof(cmd), NULL);

  // Compute the dimensions of the image at the range of the target fiducial.
  this->zoomwidth = 2 * r * tan(data->zoom/2);
  this->zoomheight = 3.0 / 4.0 * this->zoomwidth;

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Process any new blobfinder data.
int LaserVisualBarcode::UpdateBlobfinder(player_blobfinder_data_t * data, double timestamp)
{
  int id;

  // Extract valid blobs.
  this->FindBlobs(timestamp, data);

  // Search for fiducials.
  id = this->FindVisualFiducials(timestamp, data, 0, NULL);

  // Assign id to fiducial we are currently looking at.
  if (id >= 0 && this->ptz_fiducial)
  {
    if (this->ptz_fiducial->ptz_lockon_time >= 0)
    {
      this->ptz_fiducial->id = id;
      this->ptz_fiducial->id_time = timestamp;
    }
  }
  
  return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Find blobs with valid properties.
void LaserVisualBarcode::FindBlobs(double time, player_blobfinder_data_t *data)
{
  unsigned int i;
  blob_t *nblob;
  player_blobfinder_blob_t *blob;
  unsigned int width, height;
  unsigned int minx, maxx, miny, maxy;
  unsigned int minwidth, maxwidth, minheight, maxheight;
  int minarea, maxarea;
  double tol;

  // Allowable tolerance (fractional error).
  tol = 0.5;

  // Compute expect width and height at the current range and zoom.
  width = (int) (this->barwidth / this->zoomwidth * data->width);
  height = (int) (this->barheight / this->zoomheight * data->height);
      
  // Set limits
  minx = (int) ((1 - tol) * data->width / 2);
  maxx = (int) ((1 + tol) * data->width / 2);
  miny = 0;
  maxy = data->height;
  minwidth = (int) ((1 - tol) * width);
  maxwidth = (int) ((1 + tol) * width);
  minheight = (int) ((1 - tol) * height);
  maxheight = (int) ((1 + tol) * height);
  minarea = 50;
  maxarea = maxwidth * maxheight;

  /*
    printf("zoom %.3f %.3f\n", this->zoomwidth, this->zoomheight);
    printf("image %d %d\n", data->width, data->height);
    printf("w, h %d %d\n", width, height);
  */
  
  this->blob_count = 0;
  for (i = 0; i < data->blobs_count; i++)
  {
    blob = data->blobs + i;

    // Test the blob properties.
    if (blob->x < minx || blob->x > maxx)
      continue;
    if (blob->y < miny || blob->y > maxy)
      continue;
    if ((blob->right - blob->left) < minwidth || (blob->right - blob->left) > maxwidth)
      continue;
    if ((blob->bottom - blob->top) < minheight || (blob->bottom - blob->top) > maxheight)
      continue;
    if ((int) blob->area < minarea || (int) blob->area > maxarea)
      continue;

    /*
      printf("%d %d : %d %d : %d\n", blob->x, blob->y,
      blob->right - blob->left, blob->bottom - blob->top, blob->area);
    */

    // Add to valid blob list.
    if (this->blob_count < ARRAYSIZE(this->blobs))
    {
      nblob = this->blobs + this->blob_count++;
      nblob->ch = blob->id;
      nblob->x = blob->x;
      nblob->y = blob->y;
    }
  }
  
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Do a recursive depth-first search of the blob list for fiducals.
int LaserVisualBarcode::FindVisualFiducials(double time, player_blobfinder_data_t *data,
                                            int depth, blob_t *prevblob)
{
  int i, id;
  blob_t *blob;
  double dx, dy;
  double tol;
  double width, height;

  // Allowable tolerance (fractional error).
  tol = 0.5;

  // Compute expected width and height at the current range and zoom.
  width = (int) (this->barwidth / this->zoomwidth * data->width);
  height = (int) (this->barheight / this->zoomheight * data->height);

  for (i = 0; i < this->blob_count; i++)
  {
    blob = this->blobs + i;

    if (depth > 0)
    {
      dx = blob->x - prevblob->x;
      dy = blob->y - prevblob->y;

      if (fabs(dx) > (1 - tol) * width)
        continue;
      if (dy < (1 - tol) * height)
        continue;
      if (dy > (1 + tol) * height)
        continue;
    }

    if (depth == this->barcount - 1)
    {
      id = blob->ch;
      return id;
    }
    
    id = this->FindVisualFiducials(time, data, depth + 1, blob);
    if (id >= 0)
    {
      id = 10 * id + blob->ch;
      return id;
    }
  }

  return -1;
}


////////////////////////////////////////////////////////////////////////////////
// Update the device data (the data going back to the client).
void LaserVisualBarcode::UpdateData()
{
  int i;
  double r, b, o;
  double timestamp;
  fiducial_t *fiducial;
  player_fiducial_data_t data;

  data.fiducials_count = 0;
  for (i = 0; i < this->fiducial_count; i++)
  {
    fiducial = this->fiducials + i;

    // Only report fiducials that where seen in the most recent laser
    // scan.
    if (fiducial->laser_time != this->laser_time)
      continue;
    
    r = sqrt(fiducial->pose[0] * fiducial->pose[0] +
             fiducial->pose[1] * fiducial->pose[1]);
    b = atan2(fiducial->pose[1], fiducial->pose[0]);
    o = fiducial->pose[2];

    data.fiducials[data.fiducials_count].id = fiducial->id;
    data.fiducials[data.fiducials_count].pose.px = r * cos(b);
    data.fiducials[data.fiducials_count].pose.py = r * sin(b);
    data.fiducials[data.fiducials_count].pose.pyaw = o;
    data.fiducials_count++;
  }
  
  // Compute the data timestamp (from laser).
  timestamp = this->laser_time;
  
  // Copy data to server.
  Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_FIDUCIAL_DATA_SCAN, (void*) &data, 0, &timestamp);
}


