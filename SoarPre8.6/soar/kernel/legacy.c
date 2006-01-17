#include "soarkernel.h"
#include "soar_core_api.h"
#include "soar_ecore_api.h"

/* Depreciated:
 * Use: 
 */

/* Depreciated:
 * Use: soar_cCreateAgent
 */
agent *create_soar_agent(char *name)
{
    soar_cCreateAgent(name);
    return (agent *) soar_cGetAgentByName(name);
}

/* Depreciated:
 * Use: soar_cDestroyAgent
 */
void destroy_soar_agent(agent * d)
{
    soar_cDestroyAgentByAddress(d);
}

/* Depreciated:
 * Use: soar_cReInitSoar
 */
void reinitialize_soar(void)
{
    soar_cReInitSoar();
}

/* Depreciated:
 * Use: soar_cInitializeSoar
 */
void init_soar(void)
{
    soar_cInitializeSoar();
}

/* Depreciated:
 * Use: soar_cAddInputFunction
 */
void add_input_function(agent * a, soar_callback_fn f,
                        soar_callback_data cb_data, soar_callback_free_fn free_fn, char *name)
{
    soar_cAddInputFunction(a, f, cb_data, free_fn, name);
}

/* Depreciated:
 * Use: soar_cRemoveInputFunction
 */
void remove_input_function(agent * a, char *name)
{
    soar_cRemoveInputFunction(a, name);
}

/* Depreciated:
 * Use: soar_cAddOutputFunction
 */
void add_output_function(agent * a, soar_callback_fn f,
                         soar_callback_data cb_data, soar_callback_free_fn free_fn, char *output_link_name)
{
    soar_cAddOutputFunction(a, f, cb_data, free_fn, output_link_name);

}

/* Depreciated:
 * Use: soar_cRemoveOutputFunction
 */
void remove_output_function(agent * a, char *name)
{

    soar_cRemoveOutputFunction(a, name);
}

/* Depreciated:
 * Use: soar_cSetSysparam
 * moved from init_soar.c
 */
/*
void set_sysparam( int param_number, long new_val ) {
  soar_cSetSysparam( param_number, new_val );
}
*/

/* Depreciated:
 * Use: soar_ecExplainChunkTrace
 * moved from explain.c
 */
void explain_trace_named_chunk(char *name)
{
    soar_ecExplainChunkTrace(name);
}

/* Depreciated:
 * Use: soar_ecExplainChunkConditionList
 * moved from explain.c
 */

void explain_cond_list(char *name)
{
    soar_ecExplainChunkConditionList(name);
}

/* Depreciated:
 * Use: soar_ecExplainChunk
 * moved from explain.c
 */

void explain_chunk(char *name, int cond_number)
{
    soar_ecExplainChunkCondition(name, cond_number);
}

/* Depreciated:
 * Use: soar_cTestAllMonitorableCallbacks
 * moved from callback.c
 */
void soar_test_all_monitorable_callbacks(soar_callback_agent a)
{
    soar_cTestAllMonitorableCallbacks(a);
}

/* Depreciated:
 * Use: soar_cRemoveAllCallbacksForEvent
 * moved from callback.c
 */
void soar_remove_all_callbacks_for_event(soar_callback_agent a, SOAR_CALLBACK_TYPE ct)
{
    soar_cRemoveAllCallbacksForEvent(a, ct);
}

/* Depreciated:
 * Use: soar_cRemoveAllMonitorableCallbacks
 * moved from callback.c
 */
void soar_remove_all_monitorable_callbacks(soar_callback_agent a)
{

    soar_cRemoveAllMonitorableCallbacks(a);
}

/* Depreciated:
 * Use: soar_cListAllCallbacksForEvent
 * moved from callback.c
 */
void soar_list_all_callbacks_for_event(soar_callback_agent a, SOAR_CALLBACK_TYPE ct)
{

    soar_cListAllCallbacksForEvent(a, ct);
}

/* Depreciated:
 * Use: soar_cListAllCallbacks
 * moved from callback.c
 */

void soar_list_all_callbacks(soar_callback_agent a, bool monitorable_only)
{

    soar_cListAllCallbacks(a, monitorable_only);
}

/* Depreciated:
 * Use: soar_cCallbackNameToEnum
 * moved from callback.c
 */
SOAR_CALLBACK_TYPE soar_callback_name_to_enum(char *name, bool monitorable_only)
{

    return soar_cCallbackNameToEnum(name, monitorable_only);
}

/* Depreciated:
 * Use: soar_ecPrintReteStatistics
 * moved from rete.c
 */

void print_rete_statistics(void)
{

    soar_ecPrintReteStatistics();
}

/* Depreciated:
 * Use: soar_ecPrintMemoryStatistics
 * moved from rete.c
 */

void print_memory_statistics(void)
{

    soar_ecPrintMemoryStatistics();
}

/* Depreciated:
 * Use: soar_ecPrintMemoryPoolStatistics
 * moved from rete.c
 */
void print_memory_pool_statistics(void)
{

    soar_ecPrintMemoryPoolStatistics();
}

/* Depreciated:
 * Use: soar_ecPrintInternalSymbols
 * moved from symtab.c
 */
void print_internal_symbols(void)
{

    soar_ecPrintInternalSymbols();
}

void print_memories(int n, byte type[])
{
    soar_ecPrintMemories(n, (int *) type);
}
