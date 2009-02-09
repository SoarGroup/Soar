/* $Author: gerkey $
 * $Name$
 * $Id: playerjpeg.h 3018 2005-08-31 17:43:51Z gerkey $
 * $Source$
 * $Log$
 * Revision 1.1  2005/08/31 17:43:51  gerkey
 * created libplayerjpeg
 *
 * Revision 1.2  2004/11/22 23:10:17  gerkey
 * made libjpeg optional in libplayerpacket
 *
 * Revision 1.1  2004/09/17 18:09:05  inspectorg
 * *** empty log message ***
 *
 * Revision 1.1  2004/09/10 05:34:14  natepak
 * Added a JpegStream driver
 *
 * Revision 1.2  2003/12/30 20:49:49  srik
 * Added ifdef flags for compatibility
 *
 * Revision 1.1.1.1  2003/12/30 20:39:19  srik
 * Helicopter deployment with firewire camera
 *
 */

#ifndef _JPEG_H_
#define _JPEG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

int 
jpeg_compress(char *dst, char *src, int width, int height, int dstsize, int quality);

void
jpeg_decompress(unsigned char *dst, int dst_size, unsigned char *src, int src_size);

void
jpeg_decompress_from_file(unsigned char *dst, char *file, int size, int *width, int *height);

#ifdef __cplusplus
}
#endif

#endif
