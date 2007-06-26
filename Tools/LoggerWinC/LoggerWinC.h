#pragma once

// FIXME: need to fix the SendMessage issue in this project. We undefine it because we use
// a function of the same name. This is not good when the windows function needs to
// be used. This file can probably be removed once portability.h is included by the cpp files.

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <memory.h>
#include <tchar.h>

#include "resource.h"
