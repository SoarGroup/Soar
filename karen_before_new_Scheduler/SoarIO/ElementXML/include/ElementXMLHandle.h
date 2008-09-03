/////////////////////////////////////////////////////////////////
// ElementXML Handle file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
/////////////////////////////////////////////////////////////////

#ifndef ELEMENT_XML_HANDLE_H
#define ELEMENT_XML_HANDLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Define an ElementXML_Handle as a pointer to this empty structure to give users */
/* a typesafe way to pass ElementXML_Handle's in and out.  In fact it's a pointer */
/* to a class, but that's not the client's business--to the client this is an */
/* arbitrary handle.  If this is ever a problem, it can be typedef'd to a long. */
typedef struct ElementXML_InterfaceStructTag
{
	unsigned int n;	/* So that we compiles under 'C' */
} ElementXML_InterfaceStruct, *ElementXML_Handle ;

#ifdef __cplusplus
} // extern C
#endif

#endif	// ELEMENT_XML_HANDLE_H
