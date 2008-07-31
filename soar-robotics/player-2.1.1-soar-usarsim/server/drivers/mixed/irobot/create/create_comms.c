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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <libplayercore/playercommon.h>
#include <libplayercore/playerconfig.h>
#include <replace/replace.h>
//#include <sys/poll.h>

#include "create_comms.h"

#define DTOR(d) ((d) * M_PI / 180)

create_comm_t*
create_create(const char* serial_port)
{
  create_comm_t* r;

  r = calloc(1,sizeof(create_comm_t));
  assert(r);
  r->fd = -1;
  r->mode = CREATE_MODE_OFF;
  r->oa = r->ox = r->oy = 0;

  strncpy(r->serial_port,serial_port,sizeof(r->serial_port)-1);
  return(r);
}

void
create_destroy(create_comm_t* r)
{
  free(r);
}

int
create_open(create_comm_t* r, unsigned char fullcontrol)
{
  struct termios term;
  int flags;

  if(r->fd >= 0)
  {
    puts("create connection already open!");
    return(-1);
  }

  printf("Opening connection to Create on %s...", r->serial_port);
  fflush(stdout);

  // Open it.  non-blocking at first, in case there's no create
  if((r->fd = open(r->serial_port, 
                    O_RDWR | O_NONBLOCK, S_IRUSR | S_IWUSR )) < 0 )
  {
    perror("create_open():open():");
    return(-1);
  }

  if(tcflush(r->fd, TCIFLUSH) < 0 )
  {
    perror("create_open():tcflush():");
    close(r->fd);
    r->fd = -1;
    return(-1);
  }
  if(tcgetattr(r->fd, &term) < 0 )
  {
    perror("create_open():tcgetattr():");
    close(r->fd);
    r->fd = -1;
    return(-1);
  }
  
  cfmakeraw(&term);
  cfsetispeed(&term, B57600);
  cfsetospeed(&term, B57600);
  
  if(tcsetattr(r->fd, TCSAFLUSH, &term) < 0 )
  {
    perror("create_open():tcsetattr():");
    close(r->fd);
    r->fd = -1;
    return(-1);
  }

  if(create_init(r, fullcontrol) < 0)
  {
    puts("failed to initialize connection");
    close(r->fd);
    r->fd = -1;
    return(-1);
  }


  if(create_get_sensors(r, 10000) < 0)
  {
    puts("create_open():failed to get data");
    close(r->fd);
    r->fd = -1;
    return(-1);
  }

  r->oa = r->ox = r->oy = 0;

  // We know the robot is there; switch to blocking 
  // ok, we got data, so now set NONBLOCK, and continue
  if((flags = fcntl(r->fd, F_GETFL)) < 0)
  {
    perror("create_open():fcntl():");
    close(r->fd);
    r->fd = -1;
    return(-1);
  }
  if(fcntl(r->fd, F_SETFL, flags ^ O_NONBLOCK) < 0)
  {
    perror("create_open():fcntl()");
    close(r->fd);
    r->fd = -1;
    return(-1);
  }

  puts("Done.");

  return(0);
}

int
create_init(create_comm_t* r, unsigned char fullcontrol)
{
  unsigned char cmdbuf[1];

  //usleep(CREATE_DELAY_MODECHANGE_MS * 1e3);

  cmdbuf[0] = CREATE_OPCODE_START;
  if(write(r->fd, cmdbuf, 1) < 0)
  {
    perror("create_init():write():");
    return(-1);
  }
  r->mode = CREATE_MODE_PASSIVE;

  usleep(CREATE_DELAY_MODECHANGE_MS * 1e3);
  
  if (fullcontrol)
  {
    cmdbuf[0] = CREATE_OPCODE_FULL;
    if(write(r->fd, cmdbuf, 1) < 0)
    {
      perror("create_init():write():");
      return(-1);
    }
    r->mode = CREATE_MODE_FULL;
  }
  else
  {
    cmdbuf[0] = CREATE_OPCODE_SAFE;
    if(write(r->fd, cmdbuf, 1) < 0)
    {
      perror("create_init():write():");
      return(-1);
    }
    r->mode = CREATE_MODE_SAFE;

  }

  usleep(100000);

  return(0);
}

int
create_close(create_comm_t* r)
{
  //unsigned char cmdbuf[1];

  create_set_speeds(r, 0.0, 0.0);

  usleep(CREATE_DELAY_MODECHANGE_MS * 1e3);

  if(close(r->fd) < 0)
  {
    perror("create_close():close():");
    return(-1);
  }
  else
    return(0);
}

int
create_set_speeds(create_comm_t* r, double tv, double rv)
{
  unsigned char cmdbuf[5];
  int16_t tv_mm, rad_mm;

  //printf("tv: %.3lf rv: %.3lf\n", tv, rv);

  tv_mm = (int16_t)rint(tv * 1e3);
  tv_mm = MAX(tv_mm, -CREATE_TVEL_MAX_MM_S);
  tv_mm = MIN(tv_mm, CREATE_TVEL_MAX_MM_S);

  if(rv == 0)
  {
    // Special case: drive straight
    rad_mm = 0x8000;
  }
  else if(tv == 0)
  {
    // Special cases: turn in place
    if(rv > 0)
      rad_mm = 1;
    else
      rad_mm = -1;

    tv_mm = (int16_t)rint(CREATE_AXLE_LENGTH * fabs(rv) * 1e3);
  }
  else
  {
    // General case: convert rv to turn radius
    rad_mm = (int16_t)rint(tv_mm / rv);
    // The robot seems to turn very slowly with the above 
    rad_mm /= 2; 
    //printf("real rad_mm: %d\n", rad_mm);
    rad_mm = MAX(rad_mm, -CREATE_RADIUS_MAX_MM);
    rad_mm = MIN(rad_mm, CREATE_RADIUS_MAX_MM);
  }

  //printf("tv_mm: %d rad_mm: %d\n", tv_mm, rad_mm);

  cmdbuf[0] = CREATE_OPCODE_DRIVE;
  cmdbuf[1] = (unsigned char)(tv_mm >> 8);
  cmdbuf[2] = (unsigned char)(tv_mm & 0xFF);
  cmdbuf[3] = (unsigned char)(rad_mm >> 8);
  cmdbuf[4] = (unsigned char)(rad_mm & 0xFF);

  //printf("set_speeds: %X %X %X %X %X\n",
         //cmdbuf[0], cmdbuf[1], cmdbuf[2], cmdbuf[3], cmdbuf[4]);

  if(write(r->fd, cmdbuf, 5) < 0)
  {
    perror("create_set_speeds():write():");
    return(-1);
  }
  else
    return(0);
}

int
create_get_sensors(create_comm_t* r, int timeout)
{
  struct pollfd ufd[1];
  unsigned char cmdbuf[2];
  unsigned char databuf[CREATE_SENSOR_PACKET_SIZE];
  int retval;
  int numread;
  int totalnumread;
  //int i;

  cmdbuf[0] = CREATE_OPCODE_SENSORS;
  /* Zero to get all sensor data */
  cmdbuf[1] = 0;

  if(write(r->fd, cmdbuf, 2) < 0)
  {
    perror("create_get_sensors():write():");
    return(-1);
  }

  ufd[0].fd = r->fd;
  ufd[0].events = POLLIN;

  totalnumread = 0;
  while(totalnumread < sizeof(databuf))
  {
    retval = poll(ufd,1,timeout);

    if(retval < 0)
    {
      if(errno == EINTR)
        continue;
      else
      {
        perror("create_get_sensors():poll():");
        return(-1);
      }
    }
    else if(retval == 0)
    {
      printf("create_get_sensors: poll timeout\n");
      return(-1);
    }
    else
    {
      if((numread = read(r->fd,databuf+totalnumread,sizeof(databuf)-totalnumread)) < 0)
      {
        perror("create_get_sensors():read()");
        return(-1);
      }
      else
      {
        totalnumread += numread;
        /*printf("read %d bytes; buffer so far:\n", numread);
        for(i=0;i<totalnumread;i++)
          printf("%x ", databuf[i]);
        puts("");
        */
      }
    }
  }
  create_parse_sensor_packet(r, databuf, (size_t)totalnumread);
  return(0);
}

int
create_parse_sensor_packet(create_comm_t* r, unsigned char* buf, size_t buflen)
{
  unsigned char flag;
  int16_t signed_int;
  uint16_t unsigned_int;
  double dist, angle;
  int idx;

  if(buflen != CREATE_SENSOR_PACKET_SIZE)
  {
    puts("create_parse_sensor_packet():packet is wrong size");
    return(-1);
  }

  idx = 0;

  /* Bumps, wheeldrops */
  flag = buf[idx++];
  r->bumper_right = (flag >> 0) & 0x01;
  r->bumper_left = (flag >> 1) & 0x01;
  r->wheeldrop_right = (flag >> 2) & 0x01;
  r->wheeldrop_left = (flag >> 3) & 0x01;
  r->wheeldrop_caster = (flag >> 4) & 0x01;

  r->wall = buf[idx++] & 0x01;
  r->cliff_left = buf[idx++] & 0x01;
  r->cliff_frontleft = buf[idx++] & 0x01;
  r->cliff_frontright = buf[idx++] & 0x01;
  r->cliff_right = buf[idx++] & 0x01;
  r->virtual_wall = buf[idx++] & 0x01;

  flag = buf[idx++];
  r->overcurrent_sidebrush = (flag >> 0) & 0x01;
  r->overcurrent_vacuum = (flag >> 1) & 0x01;
  r->overcurrent_mainbrush = (flag >> 2) & 0x01;
  r->overcurrent_driveright = (flag >> 3) & 0x01;
  r->overcurrent_driveleft = (flag >> 4) & 0x01;

  // The next two bytes are unused (used to be dirt detector left and right)
  idx += 2;

  r->remote_opcode = buf[idx++];

  flag = buf[idx++];
  r->button_max = (flag >> 0) & 0x01;
  r->button_clean = (flag >> 1) & 0x01;
  r->button_spot = (flag >> 2) & 0x01;
  r->button_power = (flag >> 3) & 0x01;

  memcpy(&signed_int, buf+idx, 2);
  idx += 2;
  signed_int = (int16_t)ntohs((uint16_t)signed_int);
  dist = signed_int / 1e3;

  memcpy(&signed_int, buf+idx, 2);
  idx += 2;
  signed_int = (int16_t)ntohs((uint16_t)signed_int);
  angle = DTOR(signed_int);

  /* First-order odometric integration */
  r->oa = NORMALIZE(r->oa + angle);
  r->ox += dist * cos(r->oa);
  r->oy += dist * sin(r->oa);

  r->charging_state = buf[idx++];

  memcpy(&unsigned_int, buf+idx, 2);
  idx += 2;
  unsigned_int = ntohs(unsigned_int);
  r->voltage = unsigned_int / 1e3;

  memcpy(&signed_int, buf+idx, 2);
  idx += 2;
  signed_int = (int16_t)ntohs((uint16_t)signed_int);
  r->current = signed_int / 1e3;

  r->temperature = (char)(buf[idx++]);

  memcpy(&unsigned_int, buf+idx, 2);
  idx += 2;
  unsigned_int = ntohs(unsigned_int);
  r->charge = unsigned_int / 1e3;

  memcpy(&unsigned_int, buf+idx, 2);
  idx += 2;
  unsigned_int = ntohs(unsigned_int);
  r->capacity = unsigned_int / 1e3;

  return(0);
}


void
create_print(create_comm_t* r)
{
  printf("mode: %d\n", r->mode);
  printf("position: %.3lf %.3lf %.3lf\n", r->ox, r->oy, r->oa);
  printf("bumpers: l:%d r:%d\n", r->bumper_left, r->bumper_right);
  printf("wall: %d virtual wall: %d\n", r->wall, r->virtual_wall);
  printf("wheeldrops: c:%d l:%d r:%d\n", 
         r->wheeldrop_caster, r->wheeldrop_left, r->wheeldrop_right);
  printf("cliff: l:%d fl:%d fr:%d r:%d\n",
         r->cliff_left, r->cliff_frontleft, r->cliff_frontright, r->cliff_right);
  printf("overcurrent: dl:%d dr:%d mb:%d sb:%d v:%d\n",
         r->overcurrent_driveleft, r->overcurrent_driveright,
         r->overcurrent_mainbrush, r->overcurrent_sidebrush, r->overcurrent_vacuum);
  printf("dirt: l:%d r:%d\n", r->dirtdetector_left, r->dirtdetector_right);
  printf("remote opcode: %d\n", r->remote_opcode);
  printf("buttons: p:%d s:%d c:%d m:%d\n",
         r->button_power, r->button_spot, r->button_clean, r->button_max);
  printf("charging state: %d\n", r->charging_state);
  printf("battery: voltage:%.3lf current:%.3lf temp:%.3lf charge:%.3lf capacity:%.3f\n", 
         r->voltage, r->current, r->temperature, r->charge, r->capacity);

}

int create_set_song(create_comm_t* r, unsigned char songNumber, unsigned char songLength, unsigned char *notes, unsigned char *noteLengths)
{
  int size = 2*songLength+3;
  unsigned char cmdbuf[size];
  unsigned char i;

  cmdbuf[0] = CREATE_OPCODE_SONG;
  cmdbuf[1] = songNumber;
  cmdbuf[2] = songLength;

  for (i=0; i < songLength; i++)
  {
    cmdbuf[3+(2*i)] = notes[i];
    cmdbuf[3+(2*i)+1] = noteLengths[i];
  }

  if(write(r->fd, cmdbuf, size) < 0)
  {
    perror("create_set_song():write():");
    return(-1);
  }
  else
    return(0);
}

int 
create_play_song(create_comm_t *r, unsigned char songNumber)
{
  unsigned char cmdbuf[2];

  cmdbuf[0] = CREATE_OPCODE_PLAY;
  cmdbuf[1] = songNumber;

  if(write(r->fd, cmdbuf, 2) < 0)
  {
    perror("create_set_song():write():");
    return(-1);
  }
  else
    return(0);
}

int
create_vacuum(create_comm_t *r, int state)
{
  unsigned char cmdbuf[2];

  cmdbuf[0] = CREATE_OPCODE_MOTORS;
  cmdbuf[1] = state;

  if (write(r->fd, cmdbuf, 2) < 0)
  {
    perror("create_vacuum():write():");
    return -1;
  }

  return 0;
}

int
create_set_leds(create_comm_t *r, uint8_t dirt_detect, uint8_t max, uint8_t clean, uint8_t spot, uint8_t status, uint8_t power_color, uint8_t power_intensity )
{
  unsigned char cmdbuf[5];
  cmdbuf[0] = CREATE_OPCODE_LEDS;
  cmdbuf[1] = dirt_detect | max<<1 | clean<<2 | spot<<3 | status<<4;
  cmdbuf[2] = power_color;
  cmdbuf[3] = power_intensity;

  printf("Set LEDS[%d][%d][%d]\n",cmdbuf[1], cmdbuf[2], cmdbuf[3]);
  if (write(r->fd, cmdbuf, 4) < 0)
  {
    perror("create_set_leds():write():");
    return -1;
  }

  return 0;
}

int
create_run_demo(create_comm_t *r, uint8_t num)
{
  unsigned char cmdbuf[2];
  cmdbuf[0] = CREATE_OPCODE_DEMO;
  cmdbuf[1] = num;

  if (write(r->fd, cmdbuf, 2) < 0)
  {
    perror("create_run_demo():write():");
    return -1;
  }
  return 0;
}
