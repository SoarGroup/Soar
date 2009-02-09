/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2006 -
 *     Brian Gerkey
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

#ifndef ROOMBA_COMMS_H
#define ROOMBA_COMMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>

/* command opcodes */
#define ROOMBA_OPCODE_START            128
#define ROOMBA_OPCODE_BAUD             129
#define ROOMBA_OPCODE_CONTROL          130
#define ROOMBA_OPCODE_SAFE             131
#define ROOMBA_OPCODE_FULL             132
#define ROOMBA_OPCODE_POWER            133
#define ROOMBA_OPCODE_SPOT             134
#define ROOMBA_OPCODE_CLEAN            135
#define ROOMBA_OPCODE_MAX              136
#define ROOMBA_OPCODE_DRIVE            137
#define ROOMBA_OPCODE_MOTORS           138
#define ROOMBA_OPCODE_LEDS             139
#define ROOMBA_OPCODE_SONG             140
#define ROOMBA_OPCODE_PLAY             141
#define ROOMBA_OPCODE_SENSORS          142
#define ROOMBA_OPCODE_FORCEDOCK        143

#define ROOMBA_DELAY_MODECHANGE_MS      20

#define ROOMBA_MODE_OFF                  0
#define ROOMBA_MODE_PASSIVE              1
#define ROOMBA_MODE_SAFE                 2
#define ROOMBA_MODE_FULL                 3

#define ROOMBA_TVEL_MAX_MM_S           500     
#define ROOMBA_RADIUS_MAX_MM          2000

#define ROOMBA_SENSOR_PACKET_SIZE       26

#define ROOMBA_CHARGING_NOT              0
#define ROOMBA_CHARGING_RECOVERY         1
#define ROOMBA_CHARGING_CHARGING         2
#define ROOMBA_CHARGING_TRICKLE          3
#define ROOMBA_CHARGING_WAITING          4
#define ROOMBA_CHARGING_ERROR            5

#define ROOMBA_AXLE_LENGTH            0.258

#define ROOMBA_DIAMETER 0.33

#define ROOMBA_BUMPER_XOFFSET 0.05

#ifndef MIN
  #define MIN(a,b) ((a < b) ? (a) : (b))
#endif
#ifndef MAX
  #define MAX(a,b) ((a > b) ? (a) : (b))
#endif
#ifndef NORMALIZE
  #define NORMALIZE(z) atan2(sin(z), cos(z))
#endif

typedef struct
{
  /* Serial port to which the robot is connected */
  char serial_port[PATH_MAX];
  /* File descriptor associated with serial connection (-1 if no valid
   * connection) */
  int fd;
  /* Current operation mode; one of ROOMBA_MODE_* */
  unsigned char mode;
  /* Integrated odometric position [m m rad] */
  double ox, oy, oa;

  /* Various Boolean flags */
  int bumper_left, bumper_right;
  unsigned char wheeldrop_caster, wheeldrop_left, wheeldrop_right;
  unsigned char wall;
  unsigned char cliff_left, cliff_frontleft, cliff_frontright, cliff_right;
  unsigned char virtual_wall;
  unsigned char overcurrent_driveleft, overcurrent_driveright;
  unsigned char overcurrent_mainbrush, overcurrent_sidebrush;
  unsigned char overcurrent_vacuum;
  unsigned char dirtdetector_right, dirtdetector_left;
  unsigned char remote_opcode;
  unsigned char button_power, button_spot, button_clean, button_max;

  /* One of ROOMBA_CHARGING_* */
  unsigned char charging_state;
  /* Volts */
  double voltage;
  /* Amps */
  double current;
  /* degrees C */
  double temperature;
  /* Ah */
  double charge;
  /* Capacity */
  double capacity;
} roomba_comm_t;

roomba_comm_t* roomba_create(const char* serial_port);
void roomba_destroy(roomba_comm_t* r);
int roomba_open(roomba_comm_t* r, unsigned char fullcontrol);
int roomba_init(roomba_comm_t* r, unsigned char fullcontrol);
int roomba_close(roomba_comm_t* r);
int roomba_set_speeds(roomba_comm_t* r, double tv, double rv);
int roomba_parse_sensor_packet(roomba_comm_t* r, 
                               unsigned char* buf, size_t buflen);
int roomba_get_sensors(roomba_comm_t* r, int timeout);
void roomba_print(roomba_comm_t* r);
int roomba_clean(roomba_comm_t* r);
int roomba_forcedock(roomba_comm_t* r);

int roomba_set_song(roomba_comm_t* r, unsigned char songNumber, 
                    unsigned char songLength, unsigned char *notes, 
                    unsigned char *noteLengths);
int roomba_play_song(roomba_comm_t *r, unsigned char songNumber);

int roomba_vacuum(roomba_comm_t *r, int state);
int roomba_set_leds(roomba_comm_t *r, uint8_t dirt_detect, uint8_t max, 
                    uint8_t clean, uint8_t spot, uint8_t status, 
                    uint8_t power_color, uint8_t power_intensity );

#ifdef __cplusplus
}
#endif

#endif

