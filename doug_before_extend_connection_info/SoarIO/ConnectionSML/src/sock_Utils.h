// Utils.h: interface for the Utils functions
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UTILS_H__1B273E85_FDE2_494F_98BB_E64C0B02982A__INCLUDED_)
#define AFX_UTILS_H__1B273E85_FDE2_494F_98BB_E64C0B02982A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace sock {

// Makes sure pDest is null terminated, which strncpy doesn't guarantee
extern char* SafeStrncpy(char* pDest, char const* pSrc, size_t count) ;

}

#endif // !defined(AFX_UTILS_H__1B273E85_FDE2_494F_98BB_E64C0B02982A__INCLUDED_)
