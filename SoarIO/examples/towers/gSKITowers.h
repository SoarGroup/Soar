/////////////////////////////////////////////////////////////////
// gSKITowers class file.
//
// Author: Devvan Stokes, University of Michigan
// Date  : October 2004
//
// These classes define the disks, towers (or pegs), and the 
// Towers of Hanoi game world that contains them
//
/////////////////////////////////////////////////////////////////
#ifndef GSKI_TOWERS_HANOI_H
#define GSKI_TOWERS_HANOI_H

#include "Towers.h"

//Debugger directives
#include "TgD.h"
#include "tcl.h"
#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>
#define TGD_SLEEP Sleep
#else
#include <unistd.h>
#define TGD_SLEEP usleep
#endif

class HanoiWorld;
class TowerInputLinkProfile;


#endif //GSKI_TOWERS_HANOI_H

