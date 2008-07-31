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

#include "acpGarcia.h"

////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class GarciaDriver : public Driver
{
  public:

    // Constructor; need that
    GarciaDriver(ConfigFile* cf, int section);

    virtual ~GarciaDriver();

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
    void ProcessPos2dPosCmd(player_msghdr_t* hdr, player_position2d_cmd_pos_t &data);
    void ProcessPos2dVelCmd(player_msghdr_t* hdr, player_position2d_cmd_vel_t &data);
    void ProcessSpeechCommand(player_msghdr_t* hdr, player_speech_cmd_t &data);
    void ProcessDioCommand(player_msghdr_t* hdr, player_dio_cmd_t &data);

    void ProcessPos2dGeomReq(player_msghdr_t* hdr);
    void ProcessIrPoseReq(player_msghdr_t* hdr);

    void RefreshData();

 private:

    // position2d interface
    player_devaddr_t           mPos2dAddr;
    player_position2d_data_t   mPos2dData;
    player_position2d_cmd_pos_t    mPos2dPosCmd;
    player_position2d_cmd_vel_t    mPos2dVelCmd;

    // ir interface
    player_devaddr_t       mIrAddr;
    player_ir_data_t       mIrData;

    // speech interface
    player_devaddr_t       mSpeechAddr;
    player_speech_cmd_t    mSpeechCmd;

    // dio interface
    player_devaddr_t       mDioAddr;
    player_dio_data_t      mDioData;
    player_dio_cmd_t       mDioCmd;

    // power interface
    player_devaddr_t       mPowerAddr;
    player_power_data_t      mPowerData;

    int32_t mSleep;

    double mLength;
    double mWidth;
    double mWheelBase;
    double mWheelRadius;

    acpValue mSpeed;
    acpValue mPitch;
    acpValue mVolume;

    acpGarcia* mGarcia;
};
