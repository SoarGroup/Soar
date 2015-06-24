//
//  portable-dirent.h
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef portable_dirent_c
#define portable_dirent_c

#ifdef _WIN32
#include "msdirent.h"
#else
#include <dirent.h>
#endif

#endif /* portable_dirent_c */
