/////////////////////////////////////////////////////////////////
// ElementJSON Handle file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
/////////////////////////////////////////////////////////////////

#ifndef ELEMENT_JSON_HANDLE_H
#define ELEMENT_JSON_HANDLE_H

#include "portability.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Define an ElementJSON_Handle as a pointer to this empty structure to give users */
/* a typesafe way to pass ElementJSON_Handle's in and out.  In fact it's a pointer */
/* to a class, but that's not the client's business--to the client this is an */
/* arbitrary handle.  If this is ever a problem, it can be typedef'd to a long. */
struct ElementJSON_InterfaceStruct
{
    intptr_t n; /* So that we compiles under 'C' */
};
typedef ElementJSON_InterfaceStruct* ElementJSON_Handle ;

#ifdef __cplusplus
} // extern C
#endif

#endif  // ELEMENT_JSON_HANDLE_H
