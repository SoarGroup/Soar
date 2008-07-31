/* 
 *  PlayerViewer
 *  Copyright (C) Andrew Howard 2002
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
/***************************************************************************
 * Desc: Public strutures, functions
 * Author: Andrew Howard
 * Date: 14 May 2002
 * CVS: $Id: error.h 836 2002-08-17 04:59:55Z inspectorg $
 ***************************************************************************/

#ifndef ERROR_H
#define ERROR_H

// Message macros
#define PRINT_MSG(m)          printf("\r"m"\n")
#define PRINT_MSG1(m, a)      printf("\r"m"\n", a)
#define PRINT_MSG2(m, a, b)   printf("\r"m"\n", a, b)

// Warning macros
#define PRINT_WARN(msg)        printf("playerv warning : " msg "\n")
#define PRINT_WARN1(msg, a)    printf("playerv warning : " msg "\n", a)
#define PRINT_WARN2(msg, a, b) printf("playerv warning : " msg "\n", a, b)

// Error macros
#define PRINT_ERR(msg)         printf("playerv : error in "__FILE__"\n  " msg "\n")
#define PRINT_ERR1(msg, a)     printf("playerv : error in "__FILE__"\n  " msg "\n", a)
#define PRINT_ERR2(msg, a, b)  printf("playerv : error in "__FILE__"\n  " msg "\n", a, b)
#define PRINT_ERR3(msg, a, b, c)  printf("playerv : error in "__FILE__"\n  " msg "\n", a, b, c)
#define PRINT_ERRNO(msg)       printf("playerv : error in "__FILE__"\n  " msg "\n", \
                                      strerror(errno))
#define PRINT_ERRNO1(msg, a)   printf("playerv error : %s \n  " msg "\n", \
                                       a, strerror(errno))

#endif
