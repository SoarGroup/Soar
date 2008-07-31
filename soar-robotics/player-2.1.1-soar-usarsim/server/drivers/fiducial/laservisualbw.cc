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
// Desc: Driver for detecting combined laser reflectors with B&W barcodes
// Author: Andrew Howard
// Date: 9 Jan 2004
// CVS: $Id: laservisualbw.cc 4232 2007-11-01 22:16:23Z gerkey $
//
// Theory of operation:
//   Parses a laser scan to find the retro-reflective patches (lines or
//   circles), then points the camera at the patch, zooms in, and
//   attempts to read the B&W barcode.  Will not return sensible
//   orientations for circular patches.
//
// Requires:
//   Laser, PTZ and camera devices.
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_laservisualbw laservisualbw
 * @brief Black & white laser/visual barcode detector
 
@todo This driver has not been tested with the player 2 API.

Parses a laser scan to find the retro-reflective patches (lines or
circles), then points the camera at the patch, zooms in, and attempts
to read the B&W barcode.  Will not return sensible orientations for
circular patches.

@par Compile-time dependencies

- none

@par Provides

- This driver provides detected target information through a @ref
  interface_fiducial device.

@par Requires

- @ref interface_laser
- @ref interface_ptz
- @ref interface_camera

@par Configuration requests

- PLAYER_FIDUCIAL_REQ_GET_GEOM

@par Configuration file options

- max_ptz_attention (float)
  - Default: 6.0
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
- guard_min (integer)
  - Default: 4
  - Minimum height of bit (pixels)
- guard_tol (length)
  - Default: 0.2 m
  - Height tolerance of bit (ratio)
- digit_err_first (float)
  - Default: 0.5
  - Error threshold on the best bit
- digit_err_second (float)
  - Default: 1.0
  - Error threshold on the second-best bit

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

#include <libplayercore/playercore.h>

// Driver for detecting laser retro-reflectors.
class LaserVisualBW : public Driver
{
  // Constructor
  public: LaserVisualBW( ConfigFile* cf, int section);

  // Setup/shutdown routines.
  public: virtual int Setup();
  public: virtual int Shutdown();

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

  // Process incoming messages from clients 
  int ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data);

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

  // Process any new camera data.
  private: int UpdateCamera(player_camera_data_t * data, double timestamp);

  // Extract a bit string from the image.  
  private: int ExtractSymbols(int x, unsigned int symbol_max_count, int symbols[]);

  // Extract a code from a symbol string.
  private: int ExtractCode(int symbol_count, int symbols[]);
  
  // Write the device data (the data going back to the client).
  private: void WriteData();

  // Image processing
  private: double edge_thresh;
  
  // Barcode tolerances
  private: int barcount;
  private: double barwidth;
  private: double guard_min, guard_tol;
  private: double err_first, err_second;

  // Max time to spend looking at a fiducial.
  private: double max_ptz_attention;

  // Retirement age for fiducials that havent been seen for a while.
  private: double retire_time;

  // Max distance between fiducials in successive laser scans.
  private: double max_dist;

  // Laser stuff
  private: Device *laser;
  private: player_devaddr_t laser_id;
  private: double laser_time;

  // PTZ stuff
  private: Device *ptz;
  private: player_devaddr_t ptz_id;
  private: double ptz_time;

  // Camera stuff
  private: Device *camera;
  private: player_devaddr_t camera_id;
  private: double camera_time;
  private: player_camera_data_t camera_data;

  // List of currently tracked fiducials.
  private: int fiducial_count;
  private: fiducial_t fiducials[256];

  // The current selected fiducial for the ptz, the time at which we
  // selected it, and the time at which we first locked on to it.
  private: fiducial_t *ptz_fiducial;

  // Dimensions of the zoomed image for the target fiducial (m).
  private: double zoomwidth, zoomheight;
  
  // Local copy of the current fiducial data.
  private: player_fiducial_data_t fdata;
  int fdata_allocated;
};


// Initialization function
Driver* LaserVisualBW_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new LaserVisualBW( cf, section)));
}


// a driver registration function
void LaserVisualBW_Register(DriverTable* table)
{
  table->AddDriver("laservisualbw", LaserVisualBW_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
LaserVisualBW::LaserVisualBW( ConfigFile* cf, int section)
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

  // Must have a camera
  if (cf->ReadDeviceAddr(&this->camera_id, section, "requires",
                       PLAYER_CAMERA_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);    
    return;
  }
  this->camera = NULL;
  this->camera_time = 0;

  this->max_ptz_attention = cf->ReadFloat(section, "max_ptz_attention", 6.0);
  this->retire_time = cf->ReadFloat(section, "retire_time", 1.0);
  this->max_dist = cf->ReadFloat(section, "max_dist", 0.2);

  // Image processing
  this->edge_thresh = cf->ReadFloat(section, "edge_thresh", 20);
  
  // Default fiducial properties.
  this->barwidth = cf->ReadLength(section, "bit_width", 0.08);
  this->barcount = cf->ReadInt(section, "bit_count", 3);

  // Barcode properties: minimum height (pixels), height tolerance (ratio).
  this->guard_min = cf->ReadInt(section, "guard_min", 4);
  this->guard_tol = cf->ReadLength(section, "guard_tol", 0.20);

  // Error threshold on the first and second best digits
  this->err_first = cf->ReadFloat(section, "digit_err_first", 0.5);
  this->err_second = cf->ReadFloat(section, "digit_err_second", 1.0);

  // Reset fiducial list.
  this->fiducial_count = 0;

  // Reset PTZ target.
  this->ptz_fiducial = NULL;

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int LaserVisualBW::Setup()
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
  if (!(camera = deviceTable->GetDevice (camera_id)))
  {
    PLAYER_ERROR ("unable to locate suitable camera device");
    return -1;
  }
  if (camera->Subscribe (InQueue) != 0)
  {
    PLAYER_ERROR ("unable to subscribe to camera device");
    return -1;
  }
  
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int LaserVisualBW::Shutdown()
{
  // Unsubscribe from devices.
  laser->Unsubscribe(InQueue);
  ptz->Unsubscribe(InQueue);
  camera->Unsubscribe(InQueue);

  free(fdata.fiducials);

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int LaserVisualBW::ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data)
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

  if(Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE, camera_id))
  {
    assert(hdr->size == sizeof(player_camera_data_t));
    UpdateCamera(reinterpret_cast<player_camera_data_t * > (data), hdr->timestamp);
    return 0;
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Process laser data.
int LaserVisualBW::UpdateLaser(player_laser_data_t * data, double timestamp)
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
void LaserVisualBW::FindLaserFiducials(double time, player_laser_data_t *data)
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
void LaserVisualBW::FitLaserFiducial(player_laser_data_t *data,
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
void LaserVisualBW::MatchLaserFiducial(double time, double pose[3])
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
void LaserVisualBW::RetireLaserFiducials(double time, player_laser_data_t *data)
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
int LaserVisualBW::UpdatePtz(player_ptz_data_t * data, double timestamp)
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
void LaserVisualBW::SelectPtzTarget(double time, player_ptz_data_t *data)
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
void LaserVisualBW::ServoPtz(double time, player_ptz_data_t *data)
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
// Process any new camera data.
int LaserVisualBW::UpdateCamera(player_camera_data_t * data, double timestamp)
{
  int id, best_id;
  unsigned int x;
  int symbol_count;
  int symbols[480];

  this->camera_time = timestamp;
  
  best_id = -1;
  
  // Barcode may not be centered, so look across entire image
  for (x = 0; x < this->camera_data.width; x += 16)
  {
    // Extract raw symbols
    symbol_count = this->ExtractSymbols(x, sizeof(symbols) / sizeof(symbols[0]), symbols);

    // Identify barcode
    id = this->ExtractCode(symbol_count, symbols);

    // If we see multiple barcodes, we dont know which is the correct one
    if (id >= 0)
    {
      if (best_id < 0)
      {
        best_id = id;
      }
      else if (best_id != id)
      {
        best_id = -1;
        break;
      }
    }
  }

  // Assign id to fiducial we are currently looking at.
  // TODO: check for possible aliasing (not looking at the correct barcode).
  if (best_id >= 0 && this->ptz_fiducial)
  {
    if (this->ptz_fiducial->ptz_lockon_time >= 0)
    {
      this->ptz_fiducial->id = best_id;
      this->ptz_fiducial->id_time = timestamp;
    }
  }
  
  return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Extract a bit string from the image.  Takes a vertical column in
// the image and thresholds it.
int LaserVisualBW::ExtractSymbols(int x, unsigned int symbol_max_count, int symbols[])
{
  unsigned int i;
  int j, off, inc, pix;
  double fn, fv;
  int state, start, symbol_count;
  double kernel[] = {+1, +2, 0, -2, -1};

  // GREY
  if (this->camera_data.bpp == 8)
  {
    off = x * this->camera_data.bpp / 8;
    inc = this->camera_data.width * this->camera_data.bpp / 8;
  }

  // RGB24, use G channel
  else if (this->camera_data.bpp == 24)
  {
    off = x * this->camera_data.bpp / 8 + 1;
    inc = this->camera_data.width * this->camera_data.bpp / 8;
  }

  // RGB32, use G channel
  else if (this->camera_data.bpp == 32)
  {
    off = x * this->camera_data.bpp / 8 + 1;
    inc = this->camera_data.width * this->camera_data.bpp / 8;
  }

  else
  {
    PLAYER_ERROR1("no support for image depth %d", this->camera_data.bpp);
    return 0;
  }

  //FILE *file = fopen("edge.out", "a+");

  assert(symbol_max_count >= this->camera_data.height);

  state = -1;
  start = -1;
  symbol_count = 0;

  for (i = 2, pix = off + 2 * inc; i < this->camera_data.height - 2; i++, pix += inc)
  {
   // Run an edge detector
    fn = fv = 0.0;
    for (j = -2; j <= 2; j++)
    {
      fv += kernel[j + 2] * this->camera_data.image[pix + j * inc];
      fn += fabs(kernel[j + 2]);
    }
    fv /= fn;
    
    // Pick the transitions
    if (state == -1)
    {
      if (fv > +this->edge_thresh)
      {
        state = 1;
        start = i;
      }
      else if (fv < -this->edge_thresh)
      {
        state = 0;
        start = i;
      }
    }
    else if (state == 0)
    {
      if (fv > +this->edge_thresh)
      {
        symbols[symbol_count++] = -(i - start);
        state = 1;
        start = i;
      }
    }
    else if (state == 1)
    {
      if (fv < -this->edge_thresh)
      {
        symbols[symbol_count++] = +(i - start);
        state = 0;
        start = i;
      }
    }

    //fprintf(file, "%d %f %f %d\n", i, fv, fn, state);
  }

  if (state == 0)
    symbols[symbol_count++] = -(i - start);
  else if (state == 1)
    symbols[symbol_count++] = +(i - start);

  //fprintf(file, "\n\n");
  //fclose(file);

  return symbol_count;
}


////////////////////////////////////////////////////////////////////////////////
// Extract a code from a symbol string.
int LaserVisualBW::ExtractCode(int symbol_count, int symbols[])
{
  int i, j, k;
  double a, b, c;
  double mean, min, max, wm, wo;
  int best_digit;
  double best_err;
  double err[10];

  // These are UPC the mark-space patterns for digits.  From:
  // http://www.ee.washington.edu/conselec/Sp96/projects/ajohnson/proposal/project.htm
  double digits[][4] =
    {
      {-3,+2,-1,+1}, // 0
      {-2,+2,-2,+1}, // 1
      {-2,+1,-2,+2}, // 2
      {-1,+4,-1,+1}, // 3
      {-1,+1,-3,+2}, // 4
      {-1,+2,-3,+1}, // 5
      {-1,+1,-1,+4}, // 6
      {-1,+3,-1,+2}, // 7
      {-1,+2,-1,+3}, // 8
      {-3,+1,-1,+2}, // 9
    };

  best_digit = -1;
  
  // Note that each code has seven symbols in it, not counting the
  // initial space.
  for (i = 0; i < symbol_count - 7; i++)
  {
    /*
    for (j = 0; j < 7; j++)
      printf("%+d", symbols[i + j]);
    printf("\n");
    */

    a = symbols[i];
    b = symbols[i + 1];
    c = symbols[i + 2];

    // Look for a start guard: +N-N+N
    if (a > this->guard_min && b < -this->guard_min && c > this->guard_min)
    {
      mean = (a - b + c) / 3.0;
      min = MIN(a, MIN(-b, c));
      max = MAX(a, MAX(-b, c));
      assert(mean > 0);

      if ((mean - min) / mean > this->guard_tol)
        continue;
      if ((max - mean) / mean > this->guard_tol)
        continue;

      //printf("guard %d %.2f\n", i, mean);

      best_err = this->err_first;
      best_digit = -1;
      
      // Read the code digit (4 symbols) and compare against the known
      // digit patterns
      for (k = 0; k < (int) (sizeof(digits) / sizeof(digits[0])); k++)
      {
        err[k] = 0;        
        for (j = 0; j < 4; j++)
        {
          wm = digits[k][j];
          wo = symbols[i + 3 + j] / mean;
          err[k] += fabs(wo - wm);
          //printf("digit %d = %.3f %.3f\n", k, wm, wo);
        }
        //printf("digit %d = %.3f\n", k, err[k]);

        if (err[k] < best_err)
        {
          best_err = err[k];
          best_digit = k;
        }
      }

      // Id is good if it fits on and *only* one pattern.  So find the
      // second best digit and make sure it has a much higher error.
      for (k = 0; k < (int) (sizeof(digits) / sizeof(digits[0])); k++)
      {
        if (k == best_digit)
          continue;
        if (err[k] < this->err_second)
        {
          best_digit = -1;
          break;
        }
      }

      // Stop if we found a valid digit
      if (best_digit >= 0)
        break;
    }    
  }

  //if (best_digit >= 0)
  //  printf("best = %d\n", best_digit);

  return best_digit;
}


////////////////////////////////////////////////////////////////////////////////
// Update the device data (the data going back to the client).
void LaserVisualBW::WriteData()
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
  Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_FIDUCIAL_DATA_SCAN, (void*) &data, sizeof(data), &timestamp);
}


