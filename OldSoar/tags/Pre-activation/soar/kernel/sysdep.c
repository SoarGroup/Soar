#include "soarkernel.h"

/* ===================================================================
   
                        Signal Handling

   Setup things so control_c_handler() gets control whenever the program
   receives a SIGINT (e.g., from a ctrl-c at the keyboard).  The handler
   just sets the stop_soar flag.
=================================================================== */

char *c_interrupt_msg = "*** Ctrl-C Interrupt ***";

/* AGR 581  The variable the_signal is not used at all, so I thought I
   would remove it.  Unfortunately, the signal command at the end of this
   function requires a function name that has a single integer parameter.
   It's probably some unix thing.  So I left the variable in the parameter
   list and instead changed the calling functions to use a parameter.
   94.11.15 (although this was done a month or two earlier--this comment
   was placed here in retrospect.)  */

void control_c_handler(int the_signal)
{

/* Windows 3.1 can't do ^C handling */
#ifndef _WINDOWS

    cons *c;
    agent *the_agent;

    for (c = all_soar_agents; c != NIL; c = c->rest) {
        the_agent = ((agent *) c->first);
        the_agent->stop_soar = TRUE;
        the_agent->reason_for_stopping = c_interrupt_msg;
    }

    /* --- reinstall this signal handler -- some brain-damaged OS's uninstall
       it after delivering the signal --- */
    signal(SIGINT, control_c_handler);

#endif

    the_signal = the_signal;
}

void setup_signal_handling(void)
{

#ifndef _WINDOWS
    if (signal(SIGINT, control_c_handler) == SIG_ERR) {
        fprintf(stderr, "setup_signal_handling: unable to install signal handler.\n");
        fprintf(stderr, "                       Ctrl-C will not interrupt Soar.\n");
    }
#endif                          /* _WINDOWS */

}
