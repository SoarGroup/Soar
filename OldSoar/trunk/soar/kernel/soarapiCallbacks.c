/* This block of code needs to be removed and the warnings dealt with */
#ifdef _MSC_VER
#pragma message("Disabling compiler warnings 4115 4100 at top of file!")
#pragma warning(disable : 4115 4100)
#endif

#include "soarkernel.h"
#include "soarapi.h"




void cb_soarResult_AppendResult( agent *the_agent, soar_callback_data data,
				 soar_call_data call_data ) {

  appendSoarResultResult( (soarResult *)data, "%s",
			  (char *) call_data );
}
