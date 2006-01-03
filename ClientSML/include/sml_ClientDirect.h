/////////////////////////////////////////////////////////////////
// Client SML Direct header.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Provides methods used to proved optimized
// direct access to kernel sml when using
// an embedded connection and doing I/O
// (which is where performance is most critical).
//
/////////////////////////////////////////////////////////////////
#ifndef SML_CLIENT_DIRECT_H
#define SML_CLIENT_DIRECT_H

#ifndef SML_DISABLE_DIRECT_METHODS
// Defining this allows us to make direct calls to kernel sml
#define SML_DIRECT
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for these handles
// (Actual declarations are in KernelSMLDirect.h)
struct Direct_WME_StructTag ;
typedef struct Direct_WME_StructTag *Direct_WME_Handle ;

struct Direct_WMObject_StructTag ;
typedef struct Direct_WMObject_StructTag *Direct_WMObject_Handle ;

struct Direct_Agent_StructTag ;
typedef struct Direct_Agent_StructTag *Direct_Agent_Handle ;

struct Direct_WorkingMemory_StructTag ;
typedef struct Direct_WorkingMemory_StructTag *Direct_WorkingMemory_Handle ;

#ifdef __cplusplus
} // extern C
#endif

#endif //SML_CLIENT_DIRECT_H
