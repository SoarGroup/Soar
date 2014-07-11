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
struct Connection_Sender_InterfaceStruct
{
    intptr_t n; /* So that we compiles under 'C' */
};
typedef Connection_Sender_InterfaceStruct* Connection_Receiver_Handle;

// A connection object that the client creates and owns
struct Connection_Receiver_InterfaceStruct
{
    intptr_t n; /* So that we compiles under 'C' */
};
typedef Connection_Receiver_InterfaceStruct* Connection_Sender_Handle;

// Values used to opaquely wrap objects.
// They're opaque as we just store them and return them to the kernel
// without otherwise accessing them.
struct Direct_AgentSML_Struct
{
    intptr_t n; /* So that we compiles under 'C' */
};
typedef Direct_AgentSML_Struct* Direct_AgentSML_Handle;

#ifdef __cplusplus
} // extern C
#endif

#endif  // SML_HANDLES_H
