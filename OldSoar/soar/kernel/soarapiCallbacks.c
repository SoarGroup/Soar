#include "soarkernel.h"
#include "soarapi.h"

void cb_soarResult_AppendResult(agent * the_agent, soar_callback_data data, soar_call_data call_data)
{
    the_agent = the_agent;
    appendSoarResultResult((soarResult *) data, "%s", (char *) call_data);
}
