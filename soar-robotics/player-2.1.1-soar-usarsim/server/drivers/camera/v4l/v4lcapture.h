//==========================================================================
//
//  Project:        libfg - Frame Grabber interface for Linux
//
//  Module:         Capture client interface
//
//  Description:    Provides a high-level C interface for controlling frame
//                  grabber and TV tuner cards.  Uses the Video 4 Linux API
//                  (currently v1) and thus supports any V4L supported
//                  device.
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

#ifndef __V4LCAPTURE__H_
#define __V4LCAPTURE__H_


#ifdef __cplusplus
extern "C"
{
#endif


#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/videodev.h>

#include "v4lframe.h"

//==========================================================================
//  Definitions
//==========================================================================

#ifndef VIDEO_PALETTE_JPEG
#define VIDEO_PALETTE_JPEG 21
#endif

// Standard device for fg_open()
#define FG_DEFAULT_DEVICE       "/dev/video0"

// Normal capture size
#define FG_DEFAULT_WIDTH        640
#define FG_DEFAULT_HEIGHT       480

// Percentage of a ushort
#define FG_PERCENT(n)           ((n)*65535/100)
#define FG_50PC                 FG_PERCENT(50)

// Default input sources
#define FG_SOURCE_TV            0
#define FG_SOURCE_COMPOSITE     1
#define FG_SOURCE_SVIDEO        2


//--------------------------------------------------------------------------
//
//  Type:           FRAMEGRABBER
//
//  Description:    Represents all information about a frame grabber
//                  device.  Returned by fg_open(), and used as the first
//                  parameter for all other fg_*() calls.
//
//--------------------------------------------------------------------------
typedef struct
{
    char*                       device;     // Device name, eg. "/dev/video"
    int                         fd;         // File handle for open device
    struct video_capability     caps;       // Capabilities
    struct video_channel*       sources;    // Input sources (eg. TV, SVideo)
    int                         source;     // Currently selected source
    struct video_tuner          tuner;      // TV or Radio tuner
    struct video_window         window;     // Capture window
    struct video_picture        picture;    // Picture controls (eg. bright)
    struct video_mmap           mmap;       // Memory-mapped info
    struct video_buffer         fbuffer;    // Frame buffer
    struct video_mbuf           mbuf;       // Memory buffer #frames, offsets
    void*                       mb_map;     // Memory-mapped buffer
    int                         cur_frame;  // Currently capuring frame no.
    int							max_buffer; // Maximum number of frames to buffer

} FRAMEGRABBER;



//==========================================================================
//  Prototypes
//==========================================================================


//--------------------------------------------------------------------------
//
//  Function:       fg_open
//
//  Description:    Opens and initialises the frame grabber device with
//                  some reasonable default values, and queries for all
//                  capabilities.
//
//  Parameters:     char*   dev     Device name to open, eg. "/dev/video2"
//                                  or NULL for "/dev/video".
//
//  Returns:        FRAMEGRABBER*   The open framegrabber device, or
//                                  NULL in the case of an error.
//
//--------------------------------------------------------------------------

FRAMEGRABBER* fg_open( const char *dev );

int fg_enable_capture( FRAMEGRABBER* fg, int flag );

//--------------------------------------------------------------------------
//
//  Function:       fg_close
//
//  Description:    Closes an open framegrabber device, and releases all
//                  memory allocated to it.
//
//--------------------------------------------------------------------------

void fg_close( FRAMEGRABBER* fg );


//--------------------------------------------------------------------------
//
//  Function:       fg_grab
//
//  Description:    Reads a frame from the capture device, allocating
//                  a new FRAME instance and returning it.
//                  Note that this is a *blocking* read, and thus will
//                  wait until the next frame is ready.
//                  The caller is responsible for doing a frame_release()
//                  when done with the frame (to free memory).
//
//  Returns:        FRAME*      The most recently captured frame
//                  NULL        On error
//
//  Notes:          This function blocks!
//
//--------------------------------------------------------------------------

FRAME* fg_grab( FRAMEGRABBER* fg );

int fg_read(FRAMEGRABBER * fg, FRAME * fr);

//--------------------------------------------------------------------------
//
//  Function:       fg_grab_frame
//
//  Description:    Reads a frame from the capture device, using the
//                  existing frame storage as passed in.  Returns the
//                  same instance, with the contents of the last frame.
//                  Note that this is a *blocking* read, and thus will
//                  wait until the next frame is ready.
//
//  Parameters:     FRAME*      An existing frame
//
//  Returns:        FRAME*      The most recently captured frame
//                  NULL        On error
//
//  Notes:          This function blocks!
//                  The size *must* be correct!
//
//--------------------------------------------------------------------------

FRAME* fg_grab_frame( FRAMEGRABBER* fg, FRAME* fr );


//--------------------------------------------------------------------------
//
//  Function:       fg_set_source
//
//  Description:    Specifies the number of the video source to be used
//                  for the input signal.  For example, tuner, composite
//                  or S/Video signal.
//
//  Parameters:     int src     Source id (eg. FG_SOURCE_SVIDEO)
//
//  Returns:        0           On success
//                  -1          Failure
//
//--------------------------------------------------------------------------

int fg_set_source( FRAMEGRABBER* fg, int src );


//--------------------------------------------------------------------------
//
//  Function:       fg_set_source_norm
//
//  Description:    Specifies the video signal norm (eg. PAL, NTSC, SECAM)
//                  for the current input source.
//
//  Parameters:     int norm    Signal norm (eg. VIDEO_MODE_PAL)
//
//  Returns:        0           On success
//                  -1          Failure
//
//--------------------------------------------------------------------------

int fg_set_source_norm( FRAMEGRABBER* fg, int norm );


//--------------------------------------------------------------------------
//
//  Function:       fg_get_source_count
//
//  Description:    Returns the number of input sources available.
//
//  Returns:        >0          Sources (can be used in fg_set_source)
//
//--------------------------------------------------------------------------

int fg_get_source_count( FRAMEGRABBER* fg );


//--------------------------------------------------------------------------
//
//  Function:       fg_get_source_name
//
//  Description:    Returns a user-friendly name corresponding to the
//                  supplied channel number.
//
//  Parameters:     int src     Source id (eg. FG_SOURCE_TV)
//
//  Returns:        char*       Name, like "Television"
//
//--------------------------------------------------------------------------

char* fg_get_source_name( FRAMEGRABBER* fg, int src );


//--------------------------------------------------------------------------
//
//  Function:       fg_set_channel
//
//  Description:    Sets the TV tuner to the specified frequency.
//
//  Parameters:     float freq  Tuner frequency, in MHz
//
//  Returns:        0           Success, tuned in
//                  -1          Failure
//
//--------------------------------------------------------------------------

int fg_set_channel( FRAMEGRABBER* fg, float freq );


//--------------------------------------------------------------------------
//
//  Function:       fg_get_channel
//
//  Description:    Queries the current frequency of the TV tuner.
//
//  Returns:        float       The frequency in MHz
//
//--------------------------------------------------------------------------

float fg_get_channel( FRAMEGRABBER* fg );


//--------------------------------------------------------------------------
//
//  Function:       fg_set_format
//
//  Description:    Specifies the capture format to use.  Must be one of
//                  the VIDEO_PALETTE_* flags.
//
//  Notes:          Currently only RGB32 and RGB24 are properly supported.
//
//  Returns:        0           Success
//
//--------------------------------------------------------------------------

int fg_set_format( FRAMEGRABBER* fg, int fmt );

//--------------------------------------------------------------------------
//
//  Function:       fg_set_capture_window
//
//  Description:    Specifies a sub-window of the input source to capture.
//
//  Parameters:     int         x           }
//                  int         y           }  A window that is smaller than
//                  int         width       } or equal to the capture window
//                  int         height      }
//
//  Returns:        0           Success
//                  -1          Failure
//
//--------------------------------------------------------------------------

int fg_set_capture_window( FRAMEGRABBER* fg,
                           int x, int y, int width, int height );


//--------------------------------------------------------------------------
//
//  Function:       fg_set_brightness
//
//  Description:    Sets the picture brightness to the specified value.
//
//  Parameters:     int         br          Brightness (integer value)
//
//  Returns:        0           Success
//                  -1          Failure
//
//--------------------------------------------------------------------------

int fg_set_brightness( FRAMEGRABBER* fg, int br );


//--------------------------------------------------------------------------
//
//  Function:       fg_set_contrast
//
//  Description:    Sets the picture contrast to the specified value.
//
//  Parameters:     int         ct          Contrast (integer value)
//
//  Returns:        0           Success
//                  -1          Failure
//
//--------------------------------------------------------------------------

int fg_set_contrast( FRAMEGRABBER* fg, int ct );

int fg_set_hue( FRAMEGRABBER* fg, int hue );
int fg_set_colour( FRAMEGRABBER* fg, int clr );

//--------------------------------------------------------------------------
//
//  Function:       fg_new_compatible_frame
//
//  Description:    Returns a newly allocated frame that is compatible with
//                  the current frame grabber settings; that is, the window
//                  width and height, and the capture format.  This frame
//                  must be deleted by the caller with frame_release().
//
//  Returns:        FRAME*      A new frame
//
//--------------------------------------------------------------------------

FRAME* fg_new_compatible_frame( FRAMEGRABBER* fg );


//--------------------------------------------------------------------------
//
//  Function:       fg_dump_info
//
//  Description:    Dumps to the console on stdout all the status
//                  information available for the framegrabber.
//
//--------------------------------------------------------------------------

void fg_dump_info( FRAMEGRABBER* fg );


//==========================================================================

#ifdef __cplusplus
}
#endif

#endif /* __CAPTURE__H_ */
