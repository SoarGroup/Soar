/*
    This file is part of Sorts, an interface between Soar and ORTS.
    (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

    Sorts is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Sorts is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sorts; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/
#include "Sorts.h"

// DON'T put include guards around this
// it must be uniquely included for every cpp file that uses it,
// since what msg is defined as changes

// CLASS_TOKEN and ENABLE_DEBUG must be defined before this file is included

// OutputDefinitionsUnique: don't include the this ptr

#define msg cout << Sorts::frame << " " << CLASS_TOKEN << ": "

// the compiler should take out the if statement, since ENABLE_DEBUG is a
// constant.
#define dbg if(DEBUG_OUTPUT) cout << Sorts::frame << " " << CLASS_TOKEN << "[d]: "
