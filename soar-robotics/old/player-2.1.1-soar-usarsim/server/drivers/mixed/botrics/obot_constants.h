/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000-2003
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

/*
 * $Id: obot_constants.h 3308 2005-12-09 01:25:10Z gerkey $
 *
 * Relevant constants for the so-called "Trogdor" robots, by Botrics.
 * These values are taken from the 'cerebellum' module of CARMEN; thanks to
 * the authors of that module.
 */

#include <math.h>

#define OBOT_DEFAULT_PORT "/dev/usb/ttyUSB1"

// might need to define a longer delay to wait for acks
#define OBOT_DELAY_US 50000

// time between consecutive publishes
#define OBOT_PUBLISH_INTERVAL 0.1

/************************************************************************/
/* Physical constants, in meters, radians, seconds (unless otherwise noted) */
#define OBOT_AXLE_LENGTH    0.317
#define OBOT_WHEEL_DIAM     0.10795  /* 4.25 inches */
#define OBOT_WHEEL_CIRCUM   (OBOT_WHEEL_DIAM * M_PI)
#define OBOT_TICKS_PER_REV  11600.0
#define OBOT_M_PER_TICK     (OBOT_WHEEL_CIRCUM / OBOT_TICKS_PER_REV)
/* the new internal PID loop runs every 1.9375ms (we think) */
#define OBOT_PID_FREQUENCY  (1/1.9375e-3)
#define OBOT_MAGIC_TIMING_CONSTANT 1.0
#define OBOT_MPS_PER_TICK  (OBOT_M_PER_TICK * OBOT_PID_FREQUENCY / \
                                OBOT_MAGIC_TIMING_CONSTANT)
#define OBOT_WIDTH 0.45
#define OBOT_LENGTH 0.45
#define OBOT_POSE_X 0.0
#define OBOT_POSE_Y 0.0
#define OBOT_POSE_A 0.0
#define OBOT_NOMINAL_VOLTAGE 48.0

/* assuming that the counts can use the full space of a signed 32-bit int */
#define OBOT_MAX_TICS 2147483648U

/* for safety */
//#define OBOT_MAX_WHEELSPEED   1.0
#define OBOT_MAX_WHEELSPEED   5.0

/* to account for our bad low-level PID motor controller */
#define OBOT_MIN_WHEELSPEED_TICKS 5

/************************************************************************/
/* Comm protocol values */
#define OBOT_ACK   6 // if command acknowledged
#define OBOT_NACK 21 // if garbled message

#define OBOT_INIT1 253 // The init commands are used in sequence(1,2,3)
#define OBOT_INIT2 252 // to initialize a link to a cerebellum.
#define OBOT_INIT3 251 // It will then blink green and start accepting other 
                  // commands.

#define OBOT_DEINIT 250

#define OBOT_SET_VELOCITIES 118 // 'v'(left_vel, right_vel) as 16-bit signed ints
#define OBOT_SET_ACCELERATIONS 97 // 'a'(left_accel, right_accel) as 16-bit unsigned ints
#define OBOT_ENABLE_VEL_CONTROL  101 // 'e'()
#define OBOT_DISABLE_VEL_CONTROL 100 // 'd'()
#define OBOT_GET_ODOM            111 // 'o'()->(left_count, right_count, left_vel, right_vel)
#define OBOT_GET_VOLTAGE          98 // 'b'()->(batt_voltage)
#define OBOT_STOP                115 // 's'()  [shortcut for set_velocities(0,0)]
#define OBOT_KILL                107 // 'k'()  [shortcut for disable_velocity_control]
#define OBOT_HEARTBEAT           104 // 'h'() sends keepalive
/************************************************************************/


