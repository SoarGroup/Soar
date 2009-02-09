/*
 *  RTK2 : A GUI toolkit for robotics
 *  Copyright (C) 2001  Andrew Howard  ahoward@usc.edu
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
 * Desc: Rtk canvas functions
 * Author: Andrew Howard
 * CVS: $Id: rtk_canvas_movie.c 2686 2005-02-03 23:44:17Z gerkey $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>

#include "rtk.h"
#include "rtkprivate.h"

#if ENABLE_AVCODEC
#include "avcodec.h"
#endif

#define Y_RGB16(x) ((0.257 *  RTK_R_RGB16(x)) + (0.504 *  RTK_G_RGB16(x)) + (0.098 *  RTK_B_RGB16(x)) + 16)
#define V_RGB16(x) ((0.439 *  RTK_R_RGB16(x)) - (0.368 *  RTK_G_RGB16(x)) - (0.071 *  RTK_B_RGB16(x)) + 128)
#define U_RGB16(x) (-(0.148 *  RTK_R_RGB16(x)) - (0.291 *  RTK_G_RGB16(x)) + (0.439 *  RTK_B_RGB16(x)) + 128)

// Color space conversion macros
#define Y_RGB(r, g, b) ((0.257 * r) + (0.504 * g) + (0.098 * b) + 16)
#define V_RGB(r, g, b) ((0.439 * r) - (0.368 * g) - (0.071 * b) + 128)
#define U_RGB(r, g, b) (-(0.148 * r) - (0.291 * g) + (0.439 * b) + 128)


// Start movie capture.
int rtk_canvas_movie_start(rtk_canvas_t *canvas,
                           const char *filename, double fps, double speed)
{
#ifdef ENABLE_AVCODEC
  int i;
  
  // find the mpeg1 video encoder
  canvas->movie_codec = avcodec_find_encoder(CODEC_ID_MPEG1VIDEO);
  if (!canvas->movie_codec)
  {
    PRINT_ERR("codec not found");
    return -1;
  }
    
  canvas->movie_context = avcodec_alloc_context();
  assert(canvas->movie_context);
    
  canvas->movie_context->bit_rate = 400000;

  // Size must be a multiple of two
  canvas->movie_context->width = canvas->sizex - canvas->sizex % 2;  
  canvas->movie_context->height = canvas->sizey - canvas->sizey % 2;  

  // Frames per second
  canvas->movie_context->frame_rate = 25 * FRAME_RATE_BASE;

  // Number of frames between stills
  canvas->movie_context->gop_size = 10; 

  // Open the codec
  if (avcodec_open(canvas->movie_context, canvas->movie_codec) < 0)
  {
    PRINT_ERR("could not open codec");
    return -1;
  }

  // Open the output file
  canvas->movie_file = fopen(filename, "w");
  if (!canvas->movie_file)
  {
    PRINT_ERR1("could not open %s", filename);
    return -1;
  }

  // Compute the color-conversion LUT: RGB16 565 to YUV24 888
  for (i = 0; i < 0x10000; i++)
  {
    canvas->movie_lut[i][0] = Y_RGB16(i);
    canvas->movie_lut[i][1] = U_RGB16(i);
    canvas->movie_lut[i][2] = V_RGB16(i);
  }

  canvas->movie_fps = fps;
  canvas->movie_speed = speed;
  canvas->movie_frame = 0;
  canvas->movie_time = 0;
  canvas->mpeg_time = 0;
  printf("movie capture: opening [%s]\n", filename);

  return 0;

#else

  PRINT_ERR("movie support disabled at compile time");
  return -1;
  
#endif
}


// Start movie capture.
void rtk_canvas_movie_frame(rtk_canvas_t *canvas)
{
#ifdef ENABLE_AVCODEC
  AVPicture picture;
  UINT8 *outbuf, *picture_buf;
  int i, out_size, size, outbuf_size;
  uint16_t *image;
  int x, y;
  uint16_t *pix;
  uint8_t *pr, *pg, *pb;
  uint8_t *py, *pu, *pv;
  int copies;

  if (!canvas->movie_context)
    return;

  printf("movie capture: writing frame [%04d]\r", canvas->movie_frame++);
  fflush(stdout);

  // Compute the movie time.
  canvas->movie_time += 1.0 / canvas->movie_fps / canvas->movie_speed;

  // We may have to skip some frames to get the right movie speed.
  if (canvas->movie_time < canvas->mpeg_time)
    return;
  
  // alloc image and output buffer
  outbuf_size = 1000000;
  outbuf = (UINT8*) malloc(outbuf_size);
  size = canvas->movie_context->width * canvas->movie_context->height;
  picture_buf = (UINT8*) malloc((size * 3) / 2); /* size for YUV 420 */
    
  picture.data[0] = picture_buf;
  picture.data[1] = picture.data[0] + size;
  picture.data[2] = picture.data[1] + size / 4;
  picture.linesize[0] = canvas->movie_context->width;
  picture.linesize[1] = canvas->movie_context->width / 2;
  picture.linesize[2] = canvas->movie_context->width / 2;

  // Get an image in canonical 16-bit RGB format
  image = rtk_canvas_get_image_rgb16(canvas, canvas->movie_context->width,
                                     canvas->movie_context->height);
  assert(image);

  pix = image;

  py = picture.data[0];
  pu = picture.data[1];
  pv = picture.data[2];

  // Put image into YUV planar format
  for (y = 0; y < canvas->movie_context->height; y++)
  {
    for (x = 0; x < canvas->movie_context->width; x++)
    {
      *py = canvas->movie_lut[*pix][0]; py++;
      if (x % 2 == 0 && y % 2 == 0)
      {
        *pu = canvas->movie_lut[*pix][1]; pu++;
        *pv = canvas->movie_lut[*pix][2]; pv++;
      }
      *pix++;
    }
  }

  // Encode the image.  We may encode the same image multiple times in
  // order to get the correct movie speed.
  copies = 0;
  while (canvas->movie_time > canvas->mpeg_time)
  {
    out_size = avcodec_encode_video(canvas->movie_context, outbuf, outbuf_size, &picture);
    fwrite(outbuf, 1, out_size, canvas->movie_file);
    canvas->mpeg_time += 1.0 / 25;
    copies++;
  }

  free(picture_buf);
  free(outbuf);
  free(image);

  return;
#endif
}


// Stop movie capture.
void rtk_canvas_movie_stop(rtk_canvas_t *canvas)
{
#ifdef ENABLE_AVCODEC
  uint8_t outbuf[4];

  if (!canvas->movie_context)
    return;

  // add sequence end code to have a real mpeg file
  outbuf[0] = 0x00;
  outbuf[1] = 0x00;
  outbuf[2] = 0x01;
  outbuf[3] = 0xb7;
  fwrite(outbuf, 1, 4, canvas->movie_file);
  fclose(canvas->movie_file);

  avcodec_close(canvas->movie_context);
  free(canvas->movie_context);
  canvas->movie_context = NULL;

  printf("\nmovie capture: done\n");
  
  return;
#endif
}
