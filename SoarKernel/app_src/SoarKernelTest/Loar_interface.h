/********************************************************************
	created:	2001/09/04
	created:	4:9:2001   23:18
	filename: 	c:\dev\soar-84\port\loar_interface.h
	file path:	c:\dev\soar-84\port
	file base:	loar_interface
	file ext:	h
	author:		Jens Wessling
	
	purpose:	
*********************************************************************/

extern "C" {
#include "lua.h"
#include "lualib.h"
}

lua_State *init_Lua_Soar(void);
