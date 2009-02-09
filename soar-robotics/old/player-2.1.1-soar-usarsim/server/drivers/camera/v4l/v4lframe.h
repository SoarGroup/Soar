//==========================================================================
//
//  Project:        libfg - Frame Grabber interface for Linux
//
//  Module:         Frame interface
//
//  Description:    Each frame captured by the FRAMEGRABBER returns a FRAME
//                  (defined here).  It contains the raw frame data, as well
//                  as information about the frame's size and format.
//
//  Author:         Gavin Baker <gavinb@antonym.org>
//
//  Homepage:       http://www.antonym.org/libfg
//
//--------------------------------------------------------------------------
//
//  libfg - Frame Grabber interface for Linux
//  Copyright (c) 2002 Gavin Baker
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
//  or obtain a copy from the GNU website at http://www.gnu.org/
//
//==========================================================================

#ifndef __V4LFRAME_H__
#define __V4LFRAME_H__

//==========================================================================
//  Types
//==========================================================================

#include <stddef.h>

//--------------------------------------------------------------------------
//
//  Type:           FRAME
//
//  Description:    Represents a single image in the output from the
//                  frame grabber.  Carries with it the dimensions,
//                  format and the data buffer.  The type of the data
//                  depends on the format flag (uses the VIDEO_* flags from
//                  Video4Linux), so RGB24 would be a triplet of chars,
//                  while RGB32 would be an int.
//
//--------------------------------------------------------------------------

#ifndef VIDEO_PALETTE_JPEG
#define VIDEO_PALETTE_JPEG 21
#endif

typedef struct
{
    int     width;
    int     height;
    int     depth;
    int     format;
    size_t  size;
    void*   data;

} FRAME;


typedef struct
{
    char    red;
    char    green;
    char    blue;
} FRAME_RGB;


//==========================================================================
//  Prototypes
//==========================================================================

//--------------------------------------------------------------------------

FRAME* frame_new( int width, int height, int format );

void frame_release( FRAME* fr );

void* frame_get_data( FRAME* fr );

int frame_get_size( FRAME* fr );

int frame_get_width( FRAME* fr );

int frame_get_height( FRAME* fr );

int frame_save( FRAME* fr, const char* filename );
    
//==========================================================================

#endif /*  __V4LFRAME_H__ */
