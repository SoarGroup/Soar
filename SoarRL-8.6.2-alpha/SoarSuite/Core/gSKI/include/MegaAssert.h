/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

// -------------------------------------------------------------------
//	MODULE:		megaassert.h
//
//	PURPOSE:	
//
// CREATE:		2001/06/26
//
// (c) 2001 Soar Technology.  All Rights Reserved.
//
// -------------------------------------------------------------------
//#include <string>
//#include <vector>
#include <stdarg.h>

//extern "C" int  mega_assert();
//extern void DoStackTrace ( LPTSTR szString  ,DWORD  dwSize, DWORD  dwNumSkip  );

#ifndef __MEGA_ASSERT_H
#define __MEGA_ASSERT_H

#if defined _WIN32 && defined(HAS_MEGA_ASSERT)
extern "C" 
bool __declspec(dllexport) 
     mega_assert( bool        b, 
                  const char *      msg, 
                  int         line_num, 
                  const char *      msg2, 
                  bool        &ignore_always, 
                  const char *      expr);
#endif 
//      if( !exp && mega_assert( (int)(exp), __LINE__, __FILE__, ignoreAlways, #exp, description) ) {         

#if defined (_DEBUG) && defined (_WIN32) && defined(HAS_MEGA_ASSERT)
#define MegaAssert( exp, description)																													\
{																																								   \
static bool ignoreAlways = false;																														\
   if(!ignoreAlways ) {																																      \
		if( !(exp) && mega_assert( (exp)? true: false, description, __LINE__, __FILE__, ignoreAlways, #exp) ) {					\
         _asm {																																				\
            int 3																																				\
         }																																						\
      }																																							\
   }																																								\
}

#else
#include <assert.h>
#define MegaAssert( exp, description ) assert (exp && description)
#endif

#endif
