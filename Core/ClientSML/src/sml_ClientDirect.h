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
struct Direct_AgentSML_Struct ;
typedef struct Direct_AgentSML_Struct* Direct_AgentSML_Handle ;

#ifdef __cplusplus
} // extern C
#endif

#endif //SML_CLIENT_DIRECT_H
