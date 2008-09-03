/////////////////////////////////////////////////////////////////
// SML Handles file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This file just defines a few "handle" types.  These are actually pointers
// to objects created in one DLL and passed to another.  They are all opaque--that
// is the module receiving them never does anything with the pointers other than
// hand them back to the original DLL at the right time.
//
// The only reason for defining them is so that we can have some level
// of type safety on the interfaces.  If they ever prove to be a problem,
// typedef them to just be an int.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_HANDLES_H
#define SML_HANDLES_H

#ifdef __cplusplus
extern "C" {
#endif

// A connection object that the kernel creates and owns
// In practice probably the same class as Connection_Client
// but doesn't have to be and we shouldn't mix them up.
typedef struct Connection_Sender_InterfaceStructTag
{
	unsigned int n;	/* So that we compiles under 'C' */
} Connection_Sender_InterfaceStruct, *Connection_Sender_Handle ;

// A connection object that the client creates and owns
typedef struct Connection_Receiver_InterfaceStructTag
{
	unsigned int n;	/* So that we compiles under 'C' */
} Connection_Receiver_InterfaceStruct, *Connection_Receiver_Handle ;

// Values used to opaquely wrap gSKI objects.
// They're opaque as we just store them and return them to the kernel
// without otherwise accessing them.
typedef struct Direct_WME_StructTag
{
	unsigned int n;	/* So that we compiles under 'C' */
} Direct_WME_Struct, *Direct_WME_Handle ;

typedef struct Direct_WMObject_StructTag
{
	unsigned int n;	/* So that we compiles under 'C' */
} Direct_WMObject_Struct, *Direct_WMObject_Handle ;

typedef struct Direct_Agent_StructTag
{
	unsigned int n;	/* So that we compiles under 'C' */
} Direct_Agent_Struct, *Direct_Agent_Handle ;

typedef struct Direct_WorkingMemory_StructTag
{
	unsigned int n;	/* So that we compiles under 'C' */
} Direct_WorkingMemory_Struct, *Direct_WorkingMemory_Handle ;

#ifdef __cplusplus
} // extern C
#endif

#endif	// SML_HANDLES_H
