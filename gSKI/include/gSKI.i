/* File: gSKI.i
        This file helps create the various scripting language interfaces to
        to the gSKI project using the SWIG interface generation tool.
*/
%module jSKI

%{
#include "gSKI_Enumerations.h"
#include "gSKI_Structures.h"
#include "gSKI.h"
#include "gSKI_Stub.h"
#include "gSKI_Events.h"
%}

/* Just trying things with the basic header file */
/* Note: Ordering appears to be quite important below 
        bad ordering can lead to improper name spaces for
        some of the code inside the wrapper.cxx file.
*/
%include "gSKI_Structures.h"
%include "gSKI.h"
%include "gSKI_Stub.h"
%include "gSKI_Events.h"
%include "gSKI_Enumerations.h"
%include "IgSKI_Iterator.h"
%include "IgSKI_Release.h"
%include "IgSKI_KernelFactory.h"
%include "IgSKI_Kernel.h"
%include "IgSKI_AgentManager.h"
%include "IgSKI_Agent.h"
%include "IgSKI_InputProducer.h"
%include "IgSKI_OutputProcessor.h"
%include "IgSKI_Wme.h"
%include "IgSKI_WMObject.h"
%include "IgSKI_WorkingMemory.h"
%include "IgSKI_Symbol.h"
%include "IgSKI_InputLink.h"
%include "IgSKI_OutputLink.h"
