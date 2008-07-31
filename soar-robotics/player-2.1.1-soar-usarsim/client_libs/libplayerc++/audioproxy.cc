/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000-2003
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
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
/********************************************************************
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************/

/*
 * $Id: audioproxy.cc 4227 2007-10-24 22:32:04Z thjc $
 *
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cassert>
#include <sstream>
#include <iomanip>

#include "playerc++.h"
#include "debug.h"

using namespace PlayerCc;

AudioProxy::AudioProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  mInfo = &(mDevice->info);
}

AudioProxy::~AudioProxy()
{
  Unsubscribe();
}

void AudioProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_audio_create(mClient, aIndex);
  if (mDevice==NULL)
    throw PlayerError("AudioProxy::AudioProxy()", "could not create");

  if (playerc_audio_subscribe(mDevice, PLAYER_OPEN_MODE) != 0)
    throw PlayerError("AudioProxy::AudioProxy()", "could not subscribe");
}

void AudioProxy::Unsubscribe(void)
{
  assert(mDevice!=NULL);
  scoped_lock_t lock(mPc->mMutex);
  playerc_audio_unsubscribe(mDevice);
  playerc_audio_destroy(mDevice);
  mDevice = NULL;
}

// interface that all proxies SHOULD provide
std::ostream& std::operator << (std::ostream& os, const PlayerCc::AudioProxy& a)
{
  player_audio_mixer_channel_detail_t channel_detail;
  player_audio_mixer_channel_t channel;

  int old_precision = os.precision(3);
  std::_Ios_Fmtflags old_flags = os.flags();
  os.setf(std::ios::fixed);

  int NumChannelDetails = a.GetMixerDetailsCount();
  int NumChannels = a.GetChannelCount();
  int MinChan = NumChannels < NumChannelDetails ? NumChannels : NumChannelDetails;
  uint32_t state = a.GetState();

  os << MinChan << " channels:" << std::endl;
  os << "Index\tValue\tState\tType\tName" << std::endl;
  for (int ii = 0; ii < MinChan; ii++)
  {
    channel_detail = a.GetMixerDetails(ii);
    channel = a.GetChannel(ii);
    os <<  ii << '\t'
       << channel.amplitude << '\t'
       << channel.active.state << '\t'
       << channel_detail.caps << '\t'
       << channel_detail.name
       << std::endl;
  }

  cout << "State:\t";
  if (state & PLAYER_AUDIO_STATE_PLAYING)
    cout << "Playing\t";
  if (state & PLAYER_AUDIO_STATE_RECORDING)
    cout << "Recording";
  if (state == PLAYER_AUDIO_STATE_STOPPED)
    cout << "Stopped";
  cout << std::endl;

  os.precision(old_precision);
  os.flags(old_flags);

  return os;
}




/** @brief Command to play an audio block */
void AudioProxy::PlayWav(uint32_t aDataCount, uint8_t *aData, uint32_t aFormat)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_audio_wav_play_cmd(mDevice, aDataCount, aData, aFormat);
}

/** @brief Command to set recording state */
void AudioProxy::SetWavStremRec(bool aState)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_audio_wav_stream_rec_cmd(mDevice, aState);
}

/** @brief Command to play prestored sample */
void AudioProxy::PlaySample(int aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_audio_sample_play_cmd(mDevice, aIndex);
}

/** @brief Command to play sequence of tones */
void AudioProxy::PlaySeq(player_audio_seq_t * aTones)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_audio_seq_play_cmd(mDevice, aTones);
}

/** @brief Command to set multiple mixer levels */
void AudioProxy::SetMultMixerLevels(player_audio_mixer_channel_list_t * aLevels)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_audio_mixer_multchannels_cmd(mDevice, aLevels);
}

/** @brief Command to set a single mixer level */
void AudioProxy::SetMixerLevel(uint32_t index, float amplitude, uint8_t active)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_audio_mixer_channel_cmd(mDevice, index, amplitude, active);
}

/** @brief Request to record a single audio block
result is stored in wav_data */
void AudioProxy::RecordWav()
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_audio_wav_rec(mDevice);

  if (ret == -2)
    throw PlayerError("AudioProxy::RecordWav", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("AudioProxy::RecordWav",
                      playerc_error_str(),
                      ret);
}

/** @brief Request to load an audio sample */
void AudioProxy::LoadSample(int aIndex, uint32_t aDataCount, uint8_t *aData, uint32_t aFormat)
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_audio_sample_load(mDevice, aIndex, aDataCount, aData, aFormat);

  if (ret == -2)
    throw PlayerError("AudioProxy::LoadSample", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("AudioProxy::LoadSample",
                      playerc_error_str(),
                      ret);
}

/** @brief Request to retrieve an audio sample
  Data is stored in wav_data */
void AudioProxy::GetSample(int aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_audio_sample_retrieve(mDevice, aIndex);

  if (ret == -2)
    throw PlayerError("AudioProxy::GetSample", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("AudioProxy::GetSample",
                      playerc_error_str(),
                      ret);
}

/** @brief Request to record new sample */
void AudioProxy::RecordSample(int aIndex, uint32_t aLength)
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_audio_sample_rec(mDevice, aIndex, aLength);

  if (ret == -2)
    throw PlayerError("AudioProxy::RecordSample", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("AudioProxy::RecordSample",
                      playerc_error_str(),
                      ret);
}

/** @brief Request mixer channel data
result is stored in mixer_data*/
void AudioProxy::GetMixerLevels()
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_audio_get_mixer_levels(mDevice);

  if (ret == -2)
    throw PlayerError("AudioProxy::GetMixerLevels", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("AudioProxy::GetMixerLevels",
                      playerc_error_str(),
                      ret);
}

/** @brief Request mixer channel details list
result is stored in channel_details_list*/
void AudioProxy::GetMixerDetails()
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_audio_get_mixer_details(mDevice);

  if (ret == -2)
    throw PlayerError("AudioProxy::GetMixerDetails", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("AudioProxy::GetMixerDetails",
                      playerc_error_str(),
                      ret);
}
