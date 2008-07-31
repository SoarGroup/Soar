/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003  
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
 * A simple example of how to write a driver that will be built as a
 * shared object.
 */

// ONLY if you need something that was #define'd as a result of configure 
// (e.g., HAVE_CFMAKERAW), then #include <config.h>, like so:
/*
#if HAVE_CONFIG_H
  #include <config.h>
#endif
*/

#include <unistd.h>
#include <string.h>
#include <stdarg.h>


#include <libplayercore/playercore.h>
#include <libplayercore/error.h>
#define MSG_MAX 1024

// Function for printing and logging errors.
void NewErrorPrint(int msgType, int level, const char *file, int line, const char *fmt, ...)
{
  va_list ap;

  //if (level <= msgLevel)
  {
    va_start(ap, fmt);
    fprintf(stderr, "New error print: ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
  }
  if (msgFile)
  {
    char msgBuf[MSG_MAX];
    va_start(ap, fmt);
    vsnprintf(msgBuf, MSG_MAX, fmt, ap);
    va_end(ap);
    fprintf(msgFile, "New error print: %s:%d %s", file, line, msgBuf);
  }
  
  return;
}

/* need the extern to avoid C++ name-mangling  */
extern "C" {
  int player_driver_init(DriverTable* table)
  {
    PLAYER_WARN("Log plugin initialiseing");
    ErrorPrint = NewErrorPrint;
    PLAYER_WARN("Log plugin done");
    return(0);
  }
}

