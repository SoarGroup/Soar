//
//  portable-dirent.h
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef portable_dirent_c
#define portable_dirent_c

#ifndef _MSC_VER
#include <dirent.h>
#else
#include "msdirent.h"
#endif

#endif /* portable_dirent_c */
