/********************************************************************
	created:	2001/09/12
	created:	12:9:2001   21:10
	filename: 	c:\dev\soar-84\kernel++port\luaenum.cpp
	file path:	c:\dev\soar-84\kernel++port
	file base:	luaenum
	file ext:	cpp
	author:		Jens Wessling
	
	purpose:	
*********************************************************************/
#include "LuaEnum.h"


int MakeEnum(const char *enum_name, ... ){
   va_list marker;
   const char *i;

   va_start(marker, i);  // Init variable arguments
   i = va_arg(marker, const char *);
   i = va_arg(marker, const char *);
   i = va_arg(marker, const char *);
   va_end(marker);

   return 0;
}
