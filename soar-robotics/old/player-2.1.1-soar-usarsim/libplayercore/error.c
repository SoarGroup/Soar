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

/***************************************************************************
 * Desc: Error handling macros
 * Author: Andrew Howard
 * Date: 13 May 2002
 * CVS: $Id: error.c 4419 2008-03-16 08:41:58Z thjc $
 **************************************************************************/

#include <stdarg.h>

#include <libplayercore/error.h>

// User-selected msg level: 0 for the most important messages (always
// printed); 9 for the least important.
int msgLevel;

// File for logging messages
FILE *msgFile;
void (*ErrorPrint)(int msgType, int level, const char *file, int line, const char *fmt, ...) = DefaultErrorPrint;


// Initialize error logging
void 
ErrorInit(int _msgLevel)
{
  msgLevel = _msgLevel;
  msgFile = fopen(".player", "a+");
}

#define MSG_MAX 1024

// Function for printing and logging errors.
void DefaultErrorPrint(int msgType, int level, const char *file, int line, const char *fmt, ...)
{
  va_list ap;

  if (level <= msgLevel)
  {
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
  }
  if (msgFile)
  {
    char msgBuf[MSG_MAX];
    va_start(ap, fmt);
    vsnprintf(msgBuf, MSG_MAX, fmt, ap);
    va_end(ap);
    fprintf(msgFile, "%s:%d %s", file, line, msgBuf);
  }
  
  return;
}
