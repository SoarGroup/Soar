/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) <insert dates here>
 *     <insert author's name(s) here>
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
 * Fallback implementation of round(), which is not available everywhere
 * (e.g., older versions of uclibc) - BPG
 */

#include <math.h>

double 
round(double x)
{
  if(x > 0.0)
  {
    if((ceil(x) - x) <= (x - floor(x)))
      return(ceil(x));
    else
      return(floor(x));
  }
  else if(x < 0.0)
  {
    if((x - floor(x)) <= (ceil(x) - x))
      return(floor(x));
    else
      return(ceil(x));
  }
  else
    return(x);
}
