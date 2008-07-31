/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000
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
 * Desc: Some useful macros
 * $Id: playercommon.h 4100 2007-07-10 09:01:53Z thjc $
 */

#ifndef _PLAYERCOMMON_H
#define _PLAYERCOMMON_H

/* one of the following will define PATH_MAX */
#include <limits.h>
#include <sys/param.h>

#define MAX_FILENAME_SIZE PATH_MAX

/*
 * the following macros can be used to get the first char, short, or int
 * out of an unstructured buffer (such as a void* that contains a struct
 * of unknown type).
 */
#define GETFIRSTUINT8(x)  (*((uint8_t*)x))
#define GETFIRSTINT8(x)   (*((int8_t*)x))
#define GETFIRSTUINT16(x) (*((uint16_t*)x))
#define GETFIRSTINT16(x)  (*((int16_t*)x))
#define GETFIRSTUINT32(x) (*((uint32_t*)x))
#define GETFIRSTINT32(x)  (*((int32_t*)x))

#define LOBYTE(w) ((uint8_t) (w & 0xFF))
#define HIBYTE(w) ((uint8_t) ((w >> 8) & 0xFF))
#define MAKEUINT16(lo, hi) ((((uint16_t) (hi)) << 8) | ((uint16_t) (lo)))

////////////////////////////////////////////////////////////////////////////////
// Maths stuff

#ifndef M_PI
	#define M_PI        3.14159265358979323846
#endif

// Convert radians to degrees
#ifndef RTOD
#define RTOD(r) ((r) * 180 / M_PI)
#endif

// Convert degrees to radians
#ifndef DTOR
#define DTOR(d) ((d) * M_PI / 180)
#endif

// Normalize angle to domain -pi, pi
#ifndef NORMALIZE
#define NORMALIZE(z) atan2(sin(z), cos(z))
#endif

#ifndef BOOL
	#define BOOL int
#endif

#ifndef __cplusplus
  #define true 1
  #define false 0
#endif

#ifndef TRUE
	#define TRUE true
#endif

#ifndef FALSE
	#define FALSE false
#endif

////////////////////////////////////////////////////////////////////////////////
// Array checking macros

// Macro for returning array size
//
#ifndef ARRAYSIZE
	// Note that the cast to int is used to suppress warning about
	// signed/unsigned mismatches.
	#define ARRAYSIZE(x) (int) (sizeof(x) / sizeof(x[0]))
#endif

// Macro for checking array bounds
#define ASSERT_INDEX(index, array) \
    assert((size_t) (index) >= 0 && (size_t) (index) < sizeof(array) / sizeof(array[0]));


////////////////////////////////////////////////////////////////////////////////
// Misc useful stuff

#ifndef MIN
  #define MIN(a,b) ((a < b) ? (a) : (b))
#endif
#ifndef MAX
  #define MAX(a,b) ((a > b) ? (a) : (b))
#endif

#include <assert.h>
#define VERIFY(m) assert(m)


#endif
