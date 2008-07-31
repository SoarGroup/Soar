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

/*
 * Simple implementation of zlib's compressBound(). Seems that it was not
 * always in the library, as it's not available on all systems with zlib.
 * This implementation follows the docs in the 1.1.4 version of zlib.h:
 *
 *     ....destLen is the total size of the destination buffer, which must 
 *     be at least 0.1% larger than sourceLen plus 12 bytes...
 */

#include <math.h>

unsigned long 
compressBound(unsigned long sourceLen)
{
  return(((int)ceil(sourceLen * 1.001)) + 12);
}
