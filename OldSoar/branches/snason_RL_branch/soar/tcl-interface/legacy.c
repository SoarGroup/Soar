#include "soarkernel.h"
#include "soarapi.h"
#include "soar_core_api.h"

agent *create_soar_agent(char *name)
{
    soar_cCreateAgent(name);
    return soar_cGetAgentByName(name);
}

void destroy_soar_agent(agent * d)
{
    printf("Legacy destroy agent called on agent %d\n", d->id);
    soar_cDestroyAgentByAddress(d);
}

void reinitialize_soar(void)
{
    soar_cReInitSoar();
}

void init_soar(void)
{
    soar_cInitializeSoar();
}

void add_input_function(agent * a, soar_callback_fn f,
                        soar_callback_data cb_data, soar_callback_free_fn free_fn, char *name)
{
    soar_cAddInputFunction(a, f, cb_data, free_fn, name);
}

void remove_input_function(agent * a, char *name)
{
    soar_cRemoveInputFunction(a, name);
}

void add_output_function(agent * a, soar_callback_fn f,
                         soar_callback_data cb_data, soar_callback_free_fn free_fn, char *output_link_name)
{
    soar_cAddOutputFunction(a, f, cb_data, free_fn, output_link_name);

}

void remove_output_function(agent * a, char *name)
{

    soar_cRemoveOutputFunction(a, name);
}
