// OSspecific.h: interface for the OSspecific functions
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OSSPECIFIC_H__562D8D52_1008_4747_892F_6938B394006E__INCLUDED_)
#define AFX_OSSPECIFIC_H__562D8D52_1008_4747_892F_6938B394006E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SocketHeader.h"	// For SOCKET definition

bool InitializeOperatingSystemSocketLibrary() ;
bool TerminateOperatingSystemSocketLibrary() ;
bool MakeSocketNonBlocking(SOCKET hSock) ;
bool SleepMillisecs(long msecs) ;

// Map certain functions depending on the OS
#ifdef _WIN32
#define STRICMP	   stricmp
#define VSNSPRINTF _vsnprintf
#else
#define STRICMP    strcasecmp
#define VSNSPRINTF vsnprintf
#endif

#endif // !defined(AFX_OSSPECIFIC_H__562D8D52_1008_4747_892F_6938B394006E__INCLUDED_)
