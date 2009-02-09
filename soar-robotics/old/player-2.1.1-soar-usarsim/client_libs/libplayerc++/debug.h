/*
 *  Copyright (C) 2005
 *     Brad Kratochvil
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

 /*
 * $Id: debug.h 4100 2007-07-10 09:01:53Z thjc $
 *
 *   a collection of debugging macros
 */

#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

#if HAVE_CONFIG_H
  #include "config.h"
#endif

#include <iostream>

/** Debugging Macros
 *  \section debug Debug
 *  \brief macros used for debugging
 *
 * These macros can be turned on/off through defining of DEBUG_LEVEL as one
 * of the following:
 * - NONE
 * - LOW
 * - MEDIUM
 * - HIGH
 */

#define NONE   0
#define LOW    1
#define MEDIUM 2
#define HIGH   3

/** \def LOG(x)
 *  \brief write output to std::clog
 */
#if DEBUG_LEVEL < LOW
#define LOG(x)
#else
#define LOG(x) std::clog << x << std::endl
#endif

/** \def PRINT(x)
 *  \brief output name and value of expression
 */
#if DEBUG_LEVEL < MEDIUM
#define PRINT(x)
#else
#define PRINT(x) std::cerr << x << std::endl
#endif

/** \def EVAL(x)
 *  \brief evaluate a variable
 */
#if DEBUG_LEVEL < HIGH
#define EVAL(x)
#else
#define EVAL(x) \
  std::cerr << #x" = " << (x) << std::endl
#endif

#endif


