/*
 *  libplayerc : a Player client library
 *  Copyright (C) Andrew Howard 2002-2003
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) Andrew Howard 2003
 *
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
 */

#include <string.h>
#include <stdlib.h>

#include "playerc.h"
#include "error.h"

// Local declarations
void playerc_audio_putmsg(playerc_audio_t *device,
                             player_msghdr_t *header,
                             uint8_t *data, size_t len);

// Create an audio proxy
playerc_audio_t *playerc_audio_create(playerc_client_t *client, int index)
{
  playerc_audio_t *device;

  device = malloc(sizeof(playerc_audio_t));
  memset(device, 0, sizeof(playerc_audio_t));
  playerc_device_init(&device->info, client, PLAYER_AUDIO_CODE, index,
                       (playerc_putmsg_fn_t) playerc_audio_putmsg);

  return device;
}

// Destroy an audio proxy
void playerc_audio_destroy(playerc_audio_t *device)
{
  playerc_device_term(&device->info);
  free(device->wav_data.data);
  free(device);
}

// Subscribe to the audio device
int playerc_audio_subscribe(playerc_audio_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}

// Un-subscribe from the audio device
int playerc_audio_unsubscribe(playerc_audio_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}

void playerc_audio_putmsg(playerc_audio_t *device,
                             player_msghdr_t *header,
                             uint8_t *data, size_t len)
{
  if((header->type == PLAYER_MSGTYPE_DATA) && (header->subtype == PLAYER_AUDIO_DATA_WAV_REC))
  {
    player_audio_wav_t * wdata = (player_audio_wav_t *) data;
    assert(header->size > 0);
    device->wav_data.data_count = wdata->data_count;
    if (device->wav_data.data != NULL)
      free (device->wav_data.data);
    if ((device->wav_data.data = (uint8_t*) malloc (wdata->data_count)) == NULL)
      PLAYER_ERROR("Failed to allocate space to store wave data locally");
    else
    {
      memcpy(device->wav_data.data, wdata->data, wdata->data_count * sizeof(device->wav_data.data[0]));
      device->wav_data.format = wdata->format;
    }
  }
  else if((header->type == PLAYER_MSGTYPE_DATA) && (header->subtype == PLAYER_AUDIO_DATA_SEQ))
  {
    player_audio_seq_t * sdata = (player_audio_seq_t *) data;
    assert(header->size > 0);
    device->seq_data.tones_count = sdata->tones_count;
    memcpy(device->seq_data.tones, sdata->tones, sdata->tones_count * sizeof(device->seq_data.tones[0]));
  }
  else if((header->type == PLAYER_MSGTYPE_DATA) && (header->subtype == PLAYER_AUDIO_DATA_MIXER_CHANNEL))
  {
    player_audio_mixer_channel_list_t * wdata = (player_audio_mixer_channel_list_t *) data;
    assert(header->size > 0);
    device->mixer_data.channels_count = wdata->channels_count;
    memcpy(device->mixer_data.channels, wdata->channels, wdata->channels_count * sizeof(device->mixer_data.channels[0]));
  }
  else if((header->type == PLAYER_MSGTYPE_DATA) && (header->subtype == PLAYER_AUDIO_DATA_STATE))
  {
    player_audio_state_t *sdata = (player_audio_state_t *) data;
    assert(header->size > 0);
    device->state = sdata->state;
  }
  else
    PLAYERC_WARN2("skipping audio message with unknown type/subtype: %s/%d\n",
                  msgtype_to_str(header->type), header->subtype);
}

/** @brief Command to play an audio block */
int playerc_audio_wav_play_cmd(playerc_audio_t *device, uint32_t data_count, uint8_t data[], uint32_t format)
{
  player_audio_wav_t wave_data;
  memset (&wave_data, 0, sizeof (player_audio_wav_t));
  wave_data.data_count = data_count;
  // Use passed in array, as it will be copied by pack function - saves us having to malloc and free ourselves
  wave_data.data = data;
  wave_data.format = format;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_AUDIO_CMD_WAV_PLAY,
                              &wave_data, NULL);
}

/** @brief Command to set recording state */
int playerc_audio_wav_stream_rec_cmd(playerc_audio_t *device, uint8_t state)
{
  player_bool_t cmd;
  cmd.state = state;
  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_AUDIO_CMD_WAV_STREAM_REC,
                              &cmd, NULL);
}


/** @brief Command to play prestored sample */
int playerc_audio_sample_play_cmd(playerc_audio_t *device, int index)
{
  player_audio_sample_item_t cmd;
  cmd.index = index;
  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_AUDIO_CMD_SAMPLE_PLAY,
                              &cmd, NULL);
}

/** @brief Command to play sequence of tones */
int playerc_audio_seq_play_cmd(playerc_audio_t *device, player_audio_seq_t * tones)
{
  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_AUDIO_CMD_SEQ_PLAY,
                              tones, NULL);
}

/** @brief Command to set multiple mixer levels */
int playerc_audio_mixer_multchannels_cmd(playerc_audio_t *device, player_audio_mixer_channel_list_t * levels)
{
  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_AUDIO_CMD_MIXER_CHANNEL,
                              levels, NULL);
}

/** @brief Command to set a single mixer level */
int playerc_audio_mixer_channel_cmd(playerc_audio_t *device, uint32_t index, float amplitude, uint8_t active)
{
  player_audio_mixer_channel_list_t cmd;
  memset (&cmd, 0, sizeof (player_audio_mixer_channel_list_t));
  cmd.channels_count = 1;
  cmd.channels[0].amplitude = amplitude;
  cmd.channels[0].active.state = active;
  cmd.channels[0].index = index;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_AUDIO_CMD_MIXER_CHANNEL,
                              &cmd, NULL);
}


/** @brief Request to record a single audio block
Value is returned into wav_data, block length is determined by device */
int playerc_audio_wav_rec(playerc_audio_t *device)
{
  int result = 0;
  player_audio_wav_t * resp;

  if((result = playerc_client_request(device->info.client, &device->info,
                            PLAYER_AUDIO_REQ_WAV_REC,
                            NULL,(void**) &resp)) < 0)
    return result;
  player_audio_wav_t_cleanup(&device->wav_data);
  player_audio_wav_t_copy(&device->wav_data, resp);
  player_audio_wav_t_free(resp);
  
  return 0;
}


/** @brief Request to load an audio sample */
int playerc_audio_sample_load(playerc_audio_t *device, int index, uint32_t data_count, uint8_t data[], uint32_t format)
{
  int result = 0;
  player_audio_sample_t req;
  req.sample.data_count = data_count;
  // Use passed in array, as it will be copied by pack function - saves us having to malloc and free ourselves
  req.sample.data = data;
  req.sample.format = format;
  req.index = index;
  if((result = playerc_client_request(device->info.client, &device->info,
                            PLAYER_AUDIO_REQ_SAMPLE_LOAD, &req, NULL)) < 0)
    return result;

  return 0;
}

/** @brief Request to retrieve an audio sample
Data is stored in wav_data */
int playerc_audio_sample_retrieve(playerc_audio_t *device, int index)
{
  int result = 0;
  player_audio_sample_t req;
  player_audio_sample_t * resp;
  req.sample.data_count = 0;
  req.index = index;
  if((result = playerc_client_request(device->info.client, &device->info,
                            PLAYER_AUDIO_REQ_SAMPLE_RETRIEVE,
                            &req, (void**)&resp)) < 0)
    return result;

  device->wav_data.data_count = resp->sample.data_count;
  if (device->wav_data.data != NULL)
    free (device->wav_data.data);
  if ((device->wav_data.data = (uint8_t*) malloc (resp->sample.data_count)) == NULL)
  {
    player_audio_sample_t_free(resp);
    PLAYER_ERROR("Failed to allocate space to store wave data locally");
    return -1;
  }
  memcpy(device->wav_data.data, resp->sample.data, resp->sample.data_count * sizeof(device->wav_data.data[0]));
  device->wav_data.format = resp->sample.format;
  player_audio_sample_t_free(resp);
  
  return 0;
}

/** @brief Request to record new sample */
int playerc_audio_sample_rec(playerc_audio_t *device, int index, uint32_t length)
{
  int result = 0;
  player_audio_sample_rec_req_t req;
  player_audio_sample_rec_req_t *rep;
  memset (&rep, 0, sizeof (player_audio_sample_rec_req_t));
  req.index = index;
  req.length = length;
  if((result = playerc_client_request(device->info.client, &device->info,
                            PLAYER_AUDIO_REQ_SAMPLE_REC,
                            &req, (void**)&rep)) < 0)
    return result;

//  *index = req.index;
  device->last_index = rep->index;
  player_audio_sample_rec_req_t_free(rep);
  return 0;
}

/** @brief Request mixer channel data
result is stored in mixer_data*/
int playerc_audio_get_mixer_levels(playerc_audio_t *device)
{
  int result = 0;
  player_audio_mixer_channel_list_t *resp;
  if((result = playerc_client_request(device->info.client, &device->info,
                            PLAYER_AUDIO_REQ_MIXER_CHANNEL_LEVEL ,
                            NULL, (void**)&resp)) < 0)
    return result;


  device->mixer_data.channels_count = resp->channels_count;
  memcpy(device->mixer_data.channels, resp->channels, resp->channels_count * sizeof(device->mixer_data.channels[0]));
  player_audio_mixer_channel_list_t_free(resp);
  return 0;
}

/** @brief Request mixer channel details list
result is stored in channel_details_list*/
int playerc_audio_get_mixer_details(playerc_audio_t *device)
{
  int result;
  player_audio_mixer_channel_list_detail_t *rep;
  if((result = playerc_client_request(device->info.client, &device->info,
                            PLAYER_AUDIO_REQ_MIXER_CHANNEL_LIST ,
                            NULL, (void**)&rep)) < 0)
    return result;


  device->channel_details_list.details_count = rep->details_count;
  memcpy(device->channel_details_list.details, rep->details, rep->details_count * sizeof(device->channel_details_list.details[0]));
  device->channel_details_list.default_output = rep->default_output;
  device->channel_details_list.default_input = rep->default_input;
  
  player_audio_mixer_channel_list_detail_t_free(rep);
  return 0;
}


