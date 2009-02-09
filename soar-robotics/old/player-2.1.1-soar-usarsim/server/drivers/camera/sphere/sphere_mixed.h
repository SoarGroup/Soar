/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey et al.
 *
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

#include <stdint.h>

#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

#include "v4lcapture.h"

#define PLAYER_CAMERA_FORMAT_RGB 6
#define PLAYER_CAMERA_FORMAT_YUV420P 7

// Time for timestamps
extern PlayerTime *GlobalTime;

////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class SphereDriver : public Driver
{
  public:

    // Constructor; need that
    SphereDriver(ConfigFile* cf, int section);

    // Must implement the following methods.
    int Setup();
    int Shutdown();
    // Main function for device thread.
    virtual void Main();

    // This method will be invoked on each incoming message
    virtual int ProcessMessage(QueuePointer & resp_queue,
                               player_msghdr * hdr,
                               void * data);
    //void ProcessConfig();
    void ProcessCommand(player_msghdr_t* hdr, player_ptz_cmd_t &data);
    void RefreshData();

 private:

    void YUV422toRGB(uint8_t* argInputData, uint8_t* argOutputData);

    inline double LimitPan(double argP)
    {return (argP < mPanMin) ? mPanMin : ((argP > mPanMax) ? mPanMax : argP);};
    inline double LimitTilt(double argT)
    {return (argT < mTiltMin) ? mTiltMin : ((argT > mTiltMax) ? mTiltMax : argT);};

    // Camera interface
    player_devaddr_t       mCameraAddr;
    player_camera_data_t   mCameraData;
    // PTZ interface
    player_devaddr_t    mPtzAddr;
    player_ptz_data_t   mPtzData;
    player_ptz_cmd_t    mPtzCmd;

    int32_t mSleep;
    int32_t mFrameRate;
    int32_t mShutterSpeed;
    int32_t mCompressionPreference;
    int32_t mAutomaticGain;
    int32_t mSharpness;
    int32_t mBacklight;
    int32_t mFlicker;
    int32_t mNoiseReduction;
    int32_t mDumpSettings;
    int32_t mDebug;
    const char *mAutomaticWb;

    double mPanMin;
    double mPanMax;
    double mTiltMin;
    double mTiltMax;

    // Video device
    const char *mDevice;
    // Input source
    int32_t mSource;

    // Camera palette
    const char *mPalette;
    // Frame grabber interface
    FRAMEGRABBER* mFg;

    // The current image (local copy)
    FRAME* mFrame;
    FRAME* mRGBFrame;
    int32_t mFrameNumber;

    // Image dimensions
    int32_t mWidth, mHeight;
    // Pixel depth
    int32_t mDepth;

    // Write frames to disk?
    int32_t mSave;

};
