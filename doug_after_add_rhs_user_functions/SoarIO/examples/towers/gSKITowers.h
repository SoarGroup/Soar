
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


#endif //GSKI_TOWERS_HANOI_H

