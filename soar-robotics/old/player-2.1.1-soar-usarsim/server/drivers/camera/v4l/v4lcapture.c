//==========================================================================
//
//  Project:        libfg - Frame Grabber interface for Linux
//
//  Module:         Capture client implementation
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

#include "v4lcapture.h"

#include <string.h>

//--------------------------------------------------------------------------
//
//  Function:       fg_open
//
//  Description:    Opens and initialises the frame grabber device with
//                  some reasonable default values, and queries for all
//                  capabilities.
//
//  Requires:       1.  The Video4Linux subsystem to be properly initialised
//                  2.  Either a valid device name (such as "/dev/bttv") or
//                      NULL to use the default.
//                  
//  Promises:       1.  To open the video device
//                  2.  Initialise the video device to reasonable defaults
//                  3.  To not leak any memory by freeing upon any error
//                  4.  To return NULL if any initialisation fails
//                  5.  To return a valid FRAMEGRABBER instance that can
//                      be used immediately for grabbing
//
//--------------------------------------------------------------------------

FRAMEGRABBER* fg_open( const char* dev )
{
  FRAMEGRABBER* fg = malloc( sizeof( FRAMEGRABBER ) );
  int i;

  // Use default device if none specified
  if ( dev != NULL )
  {
    fg->device = strdup( dev );
  }
  else
  {
    fg->device = strdup( FG_DEFAULT_DEVICE );
  }

  // Open the video device
  fg->fd = open( fg->device, O_RDWR );
  if ( fg->fd == -1 )
  {
    perror( "fg_open(): open video device failed" );
    free( fg->device );
    free( fg );
    return NULL;
  }

  // Make sure child processes don't inherit video (close on exec)
  fcntl( fg->fd, F_SETFD, FD_CLOEXEC );

  // For n-ary buffering
  fg->cur_frame = -1;

  // Get the device capabilities
  if( ioctl( fg->fd, VIDIOCGCAP, &(fg->caps) ) < 0 )
  {
    perror( "fg_open(): query capabilities failed" );
    free( fg->device );
    free( fg );
    return NULL;
  }

  // Read info for all input sources
  fg->source = 0;
  fg->sources = malloc( sizeof( struct video_channel )*fg->caps.channels );
  for ( i = 0; i < fg->caps.channels; i++ )
  {
    fg->sources[i].channel = i;
    ioctl( fg->fd, VIDIOCGCHAN, &(fg->sources[i]) );

    // Read info about tuner
    // TODO: multiple tuners?
    if (fg->sources[i].tuners > 0)
    {
      fg->tuner.tuner = 0;
      if ( ioctl( fg->fd, VIDIOCGTUNER, &(fg->tuner) ) < 0 )
      {
        perror( "fg_open(): warning: cannot get tuner info (not present?)" );
      }
    }
  }

  /* MOVE or REMOVE; we may not know the valid palette yet
  // Set default picture attributes (50%)
  fg->picture.brightness = FG_50PC;
  fg->picture.hue        = FG_50PC;
  fg->picture.colour     = FG_50PC;
  fg->picture.contrast   = FG_50PC;
  fg->picture.whiteness  = FG_50PC;
  fg->picture.depth      = 32;
  fg->picture.palette    = VIDEO_PALETTE_RGB32;

  if ( ioctl( fg->fd, VIDIOCSPICT, &(fg->picture) ) < 0 )
  {
    perror( "fg_open(): warning: set picture attributes failed" );
    free( fg->device );
    free( fg->sources );
    free( fg );
    return NULL;
  }
  */

  // Get frame buffer info
  if ( ioctl( fg->fd, VIDIOCGFBUF, &(fg->fbuffer) ) < 0 )
  {
    perror( "fg_open(): get framebuffer failed; ignoring" );
    /* VIDIOCGBUF ioctl fails on some webcams, this can be safely ignored
    free( fg->device );
    free( fg->sources );
    free( fg );
    return NULL;
    */
  }

  // Get the memory buffer info
  if ( ioctl( fg->fd, VIDIOCGMBUF, &(fg->mbuf) ) < 0 )
  {
    perror( "fg_open(): get memory buffer" );
    free( fg->device );
    free( fg->sources );
    free( fg );
    return NULL;
  }
  
  // set the default max_buffer
  fg->max_buffer = fg->mbuf.frames;

  // Memory map the video buffer
  fg->mb_map = mmap( NULL,                    // Start addr
                     fg->mbuf.size,           // Buffer length
                     PROT_READ | PROT_WRITE,  // Protections
                     MAP_SHARED,              // Mapping flags
                     fg->fd,                  // File handle
                     0 );                     // Offset

  if ( MAP_FAILED == fg->mb_map )
  {
    perror( "fg_open(): mmap buffer" );
    free( fg->device );
    free( fg->sources );
    free( fg );
    return NULL;
  }

  return fg;
}

//--------------------------------------------------------------------------

void fg_close(FRAMEGRABBER* fg)
{
    munmap( fg->mb_map, fg->mbuf.size);     
    close( fg->fd );

    // Make sure we free all memory (backwards!)
    free( fg->device );
    free( fg->sources );
    free( fg );
}

//--------------------------------------------------------------------------

int fg_set_capture_window( FRAMEGRABBER* fg,
                           int x, int y, int width, int height )
{
    // Read default fields value
    if ( ioctl( fg->fd, VIDIOCGWIN, &(fg->window) ) < 0 )
    {
        perror( "fg_set_capture_window(): get window failed" );
        return -1;
    }

    // Set user's desired window
    fg->window.x = x;
    fg->window.y = y;
    fg->window.width  = width;
    fg->window.height = height;

/*
    // Try setting the values
    if ( ioctl( fg->fd, VIDIOCSWIN, &(fg->window) ) < 0 )
    {
        perror( "fg_set_capture_window(): set window failed" );
        return -1;
    }

    // Read back the real values set
    if ( ioctl( fg->fd, VIDIOCGWIN, &(fg->window) ) < 0 )
    {
        perror( "fg_set_capture_window(): get window failed" );
        return -1;
    }
*/

    return 0;
}

//--------------------------------------------------------------------------

int fg_enable_capture( FRAMEGRABBER* fg, int flag )
{
    if ( ioctl( fg->fd, VIDIOCCAPTURE, (flag>0) ) < 0 )
    {
        perror( "fg_enable_capture(): capture control failed" );
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------

int fg_set_format( FRAMEGRABBER* fg, int fmt )
{
    // This is only meaningful when we actually do a capture
    fg->picture.palette = fmt;

    return 0;
}

//--------------------------------------------------------------------------

int fg_set_source( FRAMEGRABBER* fg, int ch )
{
    if ( ch > fg->caps.channels )
    {
        fprintf( stderr, "fg_set_source(): Invalid channel number!\n" );
        return -1;
    }

    fg->source = ch;

    if ( ioctl( fg->fd, VIDIOCSCHAN, &(fg->sources[fg->source]) ) < 0 )
    {
        perror( "fg_set_source(): set channel failed" );
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------

int fg_set_source_norm( FRAMEGRABBER* fg, int norm )
{
    fg->sources[fg->source].norm = norm;

    if ( ioctl( fg->fd, VIDIOCSCHAN, &(fg->sources[fg->source]) ) < 0 )
    {
        perror( "fg_set_source_norm(): set channel/norm failed" );
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------

int fg_get_source_count( FRAMEGRABBER* fg )
{
    return fg->caps.channels;
}

//--------------------------------------------------------------------------

char* fg_get_source_name( FRAMEGRABBER* fg, int ch )
{
    if ( ch > fg->caps.channels )
    {
        fprintf( stderr, "fg_get_source_name(): Invalid channel number!\n" );
        return NULL;
    }

    return fg->sources[ch].name;
}

//--------------------------------------------------------------------------

int fg_get_source_type( FRAMEGRABBER* fg, int ch )
{
    if ( ch > fg->caps.channels )
    {
        fprintf( stderr, "fg_get_source_type(): Invalid channel number!\n" );
        return -1;
    }

    return fg->sources[ch].type;
}


//--------------------------------------------------------------------------

int fg_set_channel( FRAMEGRABBER* fg, float freq )
{
    // The LOW flag means freq in 1/16 MHz, not 1/16 kHz
    int scale = ( fg->tuner.flags & VIDEO_TUNER_LOW ) ? 16000 : 16;
    int val = (int)( freq * scale );

    if ( ioctl( fg->fd, VIDIOCSFREQ, &val ) < 0 )
    {
        perror( "fg_set_channel(): failed to tune channel" );
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------

float fg_get_channel( FRAMEGRABBER* fg )
{
    // The LOW flag means freq in 1/16 MHz, not 1/16 kHz
    int scale = ( fg->tuner.flags & VIDEO_TUNER_LOW ) ? 16 : 16000;
    int val = 0;
    float freq = 0;

    if ( ioctl( fg->fd, VIDIOCGFREQ, &val ) < 0 )
    {
        perror( "fg_get_channel(): failed to query channel" );
        return -1;
    }

    freq = val / scale;

    return 0;
}

//--------------------------------------------------------------------------

FRAME* fg_grab( FRAMEGRABBER* fg )
{
    return fg_grab_frame( fg,
                          frame_new( fg->window.width,
                                     fg->window.height,
                                     fg->picture.palette ) );
}

//--------------------------------------------------------------------------

FRAME* fg_grab_frame( FRAMEGRABBER* fg, FRAME* fr )
{
    int capture_frame = fg->cur_frame + 1;
    int i;
    int insize, count;
    unsigned char * buf;

    //----------------------
    // Very first time only
    //----------------------

    if ( fg->mbuf.frames > 1 && fg->cur_frame == -1 )
    {
        fg->cur_frame = 1;

        // Set up capture parameters
        fg->mmap.format = fg->picture.palette;
        fg->mmap.frame  = fg->cur_frame;
        fg->mmap.width  = fg->window.width;
        fg->mmap.height = fg->window.height;

        // Start capture
        if ( ioctl( fg->fd, VIDIOCMCAPTURE, &(fg->mmap) ) < 0 )
        {
            perror( "fg_grab(): failed to capture initial frame" );
            return NULL;
        }
    }

    //----------------------------
    // Start capturing next frame
    //----------------------------

    // Wrap counter
    if ( capture_frame >= fg->mbuf.frames || capture_frame > fg->max_buffer)
    {
        capture_frame = 0;
    }
    
    // Set up capture parameters
    fg->mmap.format = fg->picture.palette;
    fg->mmap.frame  = capture_frame;
    fg->mmap.width  = fg->window.width;
    fg->mmap.height = fg->window.height;

    // Start capture
    if ( ioctl( fg->fd, VIDIOCMCAPTURE, &(fg->mmap) ) < 0 )
    {
        perror( "fg_grab(): failed to capture frame" );
        return NULL;
    }

    //--------------------
    // Save current frame
    //--------------------

    // Wait for end of frame
    if ( ioctl( fg->fd, VIDIOCSYNC, &(fg->cur_frame) ) < 0 )
    {
        perror( "fg_grab(): failed to sync frame" );
        return NULL;
    }

    if ((fg->picture.palette) == VIDEO_PALETTE_JPEG)
    {
        buf = (unsigned char *)(fg->mb_map + fg->mbuf.offsets[fg->cur_frame]);
	insize = frame_get_size(fr) - sizeof(int);
	count = insize - 1;
	for (i = 1024; i < count; i++)
	{
	    if (buf[i] == 0xff) if (buf[i + 1] == 0xd9)
	    {
		insize = i + 10;
		break;
	    }
	}
	memcpy(fr->data, &insize, sizeof(int));
	memcpy(((unsigned char *)(fr->data)) + sizeof(int), buf, insize);
    } else
    {
	// Save video buffer into our own memory
	memcpy( fr->data,
        	fg->mb_map + fg->mbuf.offsets[fg->cur_frame],
        	frame_get_size( fr ) );
    }

    // Move along to the next one
    fg->cur_frame = capture_frame;

    return fr;
}

//--------------------------------------------------------------------------

FRAME* fg_grab_read( FRAMEGRABBER* fg )
{
    FRAME* fr = frame_new( fg->window.width,
                           fg->window.height,
                           fg->picture.palette );

    read( fg->fd,
          fr->data,
          frame_get_size( fr ) );

    return fr;
}

int fg_read(FRAMEGRABBER * fg, FRAME * fr)
{
    int count;

    if ((fg->picture.palette) == VIDEO_PALETTE_JPEG)
    {
	count = read(fg->fd, ((unsigned char *)(fr->data)) + sizeof(int), frame_get_size(fr) - sizeof(int));
	memcpy(fr->data, &count, sizeof(int));
    } else count = read(fg->fd, fr->data, frame_get_size(fr));
    return count;
}

//--------------------------------------------------------------------------

int fg_set_brightness( FRAMEGRABBER* fg, int br )
{
    if ( ioctl( fg->fd, VIDIOCGPICT, &(fg->picture) ) < 0 )
    {
        perror( "fg_set_brightness(): get attribute failed" );
        return -1;
    }

    fg->picture.brightness = br;

    if ( ioctl( fg->fd, VIDIOCSPICT, &(fg->picture) ) < 0 )
    {
        perror( "fg_set_brightness(): set attribute failed" );
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------

int fg_set_hue( FRAMEGRABBER* fg, int hue )
{
    if ( ioctl( fg->fd, VIDIOCGPICT, &(fg->picture) ) < 0 )
    {
        perror( "fg_set_hue(): get attribute failed" );
        return -1;
    }

    fg->picture.hue = hue;

    if ( ioctl( fg->fd, VIDIOCSPICT, &(fg->picture) ) < 0 )
    {
        perror( "fg_set_hue(): set attribute failed" );
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------

int fg_set_colour( FRAMEGRABBER* fg, int clr )
{
    if ( ioctl( fg->fd, VIDIOCGPICT, &(fg->picture) ) < 0 )
    {
        perror( "fg_set_colour(): get attribute failed" );
        return -1;
    }

    fg->picture.colour = clr;

    if ( ioctl( fg->fd, VIDIOCSPICT, &(fg->picture) ) < 0 )
    {
        perror( "fg_set_colour(): set attribute failed" );
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------

int fg_set_contrast( FRAMEGRABBER* fg, int ct )
{
    if ( ioctl( fg->fd, VIDIOCGPICT, &(fg->picture) ) < 0 )
    {
        perror( "fg_set_contrast(): get attribute failed" );
        return -1;
    }

    fg->picture.contrast = ct;

    if ( ioctl( fg->fd, VIDIOCSPICT, &(fg->picture) ) < 0 )
    {
        perror( "fg_set_contrast(): set attribute failed" );
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------

FRAME* fg_new_compatible_frame( FRAMEGRABBER* fg )
{
    return frame_new( fg->window.width,
                      fg->window.height,
                      fg->picture.palette );
}

//--------------------------------------------------------------------------

#define FROM_PC(n)      (n*100/65535)
#define TEST_FLAG(v,f)  (((v)|(f))?"Yes":"___")

void fg_dump_info(FRAMEGRABBER* fg)
{
  int i;
  int type = fg->caps.type;

  if ( fg->fd < 1 )
  {
    fprintf( stderr, "fg_dump_info(): device not open/initialised!" );
    return;
  }

  // Dump out the contents of the capabilities structure
  printf( "\nFrame Grabber Details\n" );
  printf( "=====================\n" );

  printf( "  device    = %s\n", fg->device );
  printf( "  fd handle = 0x%08xd\n", fg->fd );

  // Capabilities
  printf( "  caps.name      = %s\n", fg->caps.name );
  printf( "  caps.channels  = %d\n", fg->caps.channels );
  printf( "  caps.audio chs = %d\n", fg->caps.audios );
  printf( "  caps.maxwidth  = %d\n", fg->caps.maxwidth );
  printf( "  caps.maxheight = %d\n", fg->caps.maxheight );
  printf( "  caps.minwidth  = %d\n", fg->caps.minwidth );
  printf( "  caps.minheight = %d\n", fg->caps.minheight );

  printf( "  caps.type = \n" );
  printf( "\t%s:    VID_TYPE_CAPTURE"
          "\tCan capture to memory\n",
          TEST_FLAG(type, VID_TYPE_CAPTURE) );
  printf( "\t%s:    VID_TYPE_TUNER"
          "\t\tHas a tuner of some form\n",
          TEST_FLAG( type, VID_TYPE_TUNER ) );
  printf( "\t%s:    VID_TYPE_TELETEXT"
          "\tHas teletext capability\n",
          TEST_FLAG( type, VID_TYPE_TELETEXT ) );
  printf( "\t%s:    VID_TYPE_OVERLAY"
          "\tCan overlay images onto the frame buffer\n",
          TEST_FLAG( type, VID_TYPE_OVERLAY ) );
  printf( "\t%s:    VID_TYPE_CHROMAKEY"
          "\tOverlay is Chromakeyed\n",
          TEST_FLAG( type, VID_TYPE_CHROMAKEY ) );
  printf( "\t%s:    VID_TYPE_CLIPPING"
          "\tOverlay clipping is supported\n",
          TEST_FLAG( type, VID_TYPE_CLIPPING ) );
  printf( "\t%s:    VID_TYPE_FRAMERAM"
          "\tOverlay overwrites frame buffer memory\n",
          TEST_FLAG( type, VID_TYPE_FRAMERAM ) );
  printf( "\t%s:    VID_TYPE_SCALES"
          "\t\tThe hardware supports image scaling\n",
          TEST_FLAG( type, VID_TYPE_SCALES ) );
  printf( "\t%s:    VID_TYPE_MONOCHROME"
          "\tImage capture is grey scale only\n",
          TEST_FLAG( type, VID_TYPE_MONOCHROME ) );
  printf( "\t%s:    VID_TYPE_SUBCAPTURE"
          "\tCapture can be of only part of the image\n",
          TEST_FLAG( type, VID_TYPE_SUBCAPTURE ) );

  // Dump input sources
  for ( i = 0; i < fg_get_source_count(fg); i++ )
  {
    printf( "  sources[%d].name = %s\n", i, fg_get_source_name(fg, i ) );
    printf( "  sources[%d].norm = ...\n", i );

    // Tuner info
    // TODO: multiple tuners?
    if (fg->sources[i].tuners > 0)
      printf( "  tuner[0].name = %s\n", fg->tuner.name );
  }

  // Capture window
  printf( "  window.x         = %u\n", fg->window.x );
  printf( "  window.y         = %u\n", fg->window.y );
  printf( "  window.width     = %u\n", fg->window.width );
  printf( "  window.height    = %u\n", fg->window.height );
  printf( "  window.chromakey = 0x%08x\n", fg->window.chromakey );
  printf( "  window.flags     = %u\n", fg->window.flags );

  // Picture
  printf( "  picture.brightness = %u%%\n", FROM_PC(fg->picture.brightness ));
  printf( "  picture.hue        = %u%%\n", FROM_PC(fg->picture.hue ));
  printf( "  picture.colour     = %u%%\n", FROM_PC(fg->picture.colour ));
  printf( "  picture.contrast   = %u%%\n", FROM_PC(fg->picture.contrast ));
  printf( "  picture.whiteness  = %u%%\n", FROM_PC(fg->picture.whiteness ));
  printf( "  picture.depth      = %u\n", fg->picture.depth );
  printf( "  picture.palette    = %u\n", fg->picture.palette );

  // mmap

  // Dump out frame buffer setup
	printf( "  fbuffer.base         = 0x%p\n", fg->fbuffer.base );
  printf( "  fbuffer.width        = %u\n", fg->fbuffer.width );
	printf( "  fbuffer.height       = %u\n", fg->fbuffer.height );
	printf( "  fbuffer.depth        = %u\n", fg->fbuffer.depth );
	printf( "  fbuffer.bytesperline = %u\n", fg->fbuffer.bytesperline );

  // Dump memory buffer info
  printf( "  mbuf.size   = %u\n", fg->mbuf.size );
  printf( "  mbuf.frames = %u\n", fg->mbuf.frames );
  for ( i = 0; i < fg->mbuf.frames; i++ )
  {
    printf( "  mbuf.offsets[%u] = %u\n", i, fg->mbuf.offsets[i] );
  }

  printf( "  mb_map      = 0x%p\n", (fg->mb_map) );
}

//==========================================================================
