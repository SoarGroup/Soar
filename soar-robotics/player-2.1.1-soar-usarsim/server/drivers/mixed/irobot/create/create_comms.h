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

#ifndef CREATE_COMMS_H
#define CREATE_COMMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>

/* command opcodes */
#define CREATE_OPCODE_START            128
#define CREATE_OPCODE_BAUD             129
#define CREATE_OPCODE_SAFE             131
#define CREATE_OPCODE_FULL             132
#define CREATE_OPCODE_SPOT             134
#define CREATE_OPCODE_COVER            135
#define CREATE_OPCODE_DEMO             136
#define CREATE_OPCODE_DRIVE            137
#define CREATE_OPCODE_MOTORS           138
#define CREATE_OPCODE_LEDS             139
#define CREATE_OPCODE_SONG             140
#define CREATE_OPCODE_PLAY             141
#define CREATE_OPCODE_SENSORS          142
#define CREATE_OPCODE_COVERDOCK        143
#define CREATE_OPCODE_PWM_MOTORS       144
#define CREATE_OPCODE_DRIVE_WHEELS     145
#define CREATE_OPCODE_DIGITAL_OUTPUTS  147
#define CREATE_OPCODE_STREAM           148
#define CREATE_OPCODE_QUERY_LIST       149
#define CREATE_OPCODE_DO_STREAM        150
#define CREATE_OPCODE_SEND_IR_CHAR     151
#define CREATE_OPCODE_SCRIPT           152
#define CREATE_OPCODE_PLAY_SCRIPT      153
#define CREATE_OPCODE_SHOW_SCRIPT      154
#define CREATE_OPCODE_WAIT_TIME        155
#define CREATE_OPCODE_WAIT_DISTANCE    156
#define CREATE_OPCODE_WAIT_ANGLE       157
#define CREATE_OPCODE_WAIT_EVENT       158


#define CREATE_DELAY_MODECHANGE_MS      20

#define CREATE_MODE_OFF                  0
#define CREATE_MODE_PASSIVE              1
#define CREATE_MODE_SAFE                 2
#define CREATE_MODE_FULL                 3

#define CREATE_TVEL_MAX_MM_S           500     
#define CREATE_RADIUS_MAX_MM          2000

#define CREATE_SENSOR_PACKET_SIZE       26

#define CREATE_CHARGING_NOT              0
#define CREATE_CHARGING_RECOVERY         1
#define CREATE_CHARGING_CHARGING         2
#define CREATE_CHARGING_TRICKLE          3
#define CREATE_CHARGING_WAITING          4
#define CREATE_CHARGING_ERROR            5

#define CREATE_AXLE_LENGTH            0.258

#define CREATE_DIAMETER 0.33

#define CREATE_BUMPER_XOFFSET 0.05

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
  /* Current operation mode; one of CREATE_MODE_* */
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

  /* One of CREATE_CHARGING_* */
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
} create_comm_t;

create_comm_t* create_create(const char* serial_port);
void create_destroy(create_comm_t* r);
int create_open(create_comm_t* r, unsigned char fullcontrol);
int create_init(create_comm_t* r, unsigned char fullcontrol);
int create_close(create_comm_t* r);
int create_set_speeds(create_comm_t* r, double tv, double rv);
int create_parse_sensor_packet(create_comm_t* r, 
                               unsigned char* buf, size_t buflen);
int create_get_sensors(create_comm_t* r, int timeout);
void create_print(create_comm_t* r);

int create_set_song(create_comm_t* r, unsigned char songNumber, 
                    unsigned char songLength, unsigned char *notes, 
                    unsigned char *noteLengths);
int create_play_song(create_comm_t *r, unsigned char songNumber);

int create_vacuum(create_comm_t *r, int state);
int create_set_leds(create_comm_t *r, uint8_t dirt_detect, uint8_t max, 
                    uint8_t clean, uint8_t spot, uint8_t status, 
                    uint8_t power_color, uint8_t power_intensity );

int create_run_demo(create_comm_t *r, uint8_t num);

#ifdef __cplusplus
}
#endif

#endif

