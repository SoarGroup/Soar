/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2008
 *     Brian Gerkey - Player
 *     Klaas Gadeyne - clock_gettime replacement function
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

#include <sys/time.h>

/* This replacement function originally written by Klass Gadeyne
   for the Orocos Project */
int clock_gettime(int clk_id /*ignored*/, struct timespec *tp)
{
    struct timeval now;
    int rv = gettimeofday(&now, NULL);
    if (rv != 0) 
    {
        tp->tv_sec = 0;
        tp->tv_nsec = 0;
        return rv;
    }
    tp->tv_sec = now.tv_sec;
    tp->tv_nsec = now.tv_usec * 1000;
    return 0;
}
