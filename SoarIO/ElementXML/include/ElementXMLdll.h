/////////////////////////////////////////////////////////////////
// ElementXMLdll: Defines the entry point for this DLL.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This library is responsible for representing an XML document as an object (actually a tree of objects).
//
// A client can send a stream of XML data which this class parses to create the object representation of the XML.
// Or the client can call to this library directly, creating the object representation without ever producing the actual
// XML output (this is just for improved efficiency when the client and the Soar kernel are embedded in the same process).
//
// This class will not support the full capabilities of XML which is now a complex language.
// It will support just the subset that is necessary for SML (Soar Markup Language) which is intended to be its primary customer.
/////////////////////////////////////////////////////////////////

#ifndef ELEMENTXMLDLL_H
#define ELEMENTXMLDLL_H

#ifndef unused
#define unused(x) (void)(x)
#endif

#endif	// ELEMENTXMLDLL_H
