/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
             output_manager_print.cpp

   @brief output_manager_print.cpp provides many functions to
   print Soar data structures.  Many were originally written
   for debugging purposes and are only fount in print commands.

------------------------------------------------------------------ */

#include "rhs.h"
#include "print.h"
#include "agent.h"
#include "instantiations.h"
#include "rete.h"
#include "reorder.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "output_manager.h"
#include "output_manager_db.h"
#include "prefmem.h"
#include "wmem.h"
#include "soar_instance.h"
#include "test.h"
#include "variablization_manager.h"

#include <iostream>

/* This is far too large, but we're setting it to this until we fix a bug we previously had
 * with strncpy corrupting memory. */
#define OM_BUFFER_SIZE 70000   /* --- size of output buffer for a calls to print routines --- */

void Output_Manager::printa_sf(agent* pSoarAgent, const char* format, ...)
{
    va_list args;
    char buf[OM_BUFFER_SIZE];

    va_start(args, format);
    vsnprint_sf(pSoarAgent, buf, OM_BUFFER_SIZE, format, args);
    va_end(args);
    printa(pSoarAgent, buf);
}

void Output_Manager::printa(agent* pSoarAgent, const char* msg)
{
    if (pSoarAgent)
    {
        if (!pSoarAgent->output_settings->print_enabled) return;
        if (pSoarAgent->output_settings->callback_mode)
        {
            soar_invoke_callbacks(pSoarAgent, PRINT_CALLBACK, static_cast<soar_call_data>(const_cast<char*>(msg)));
        }
        if (pSoarAgent->output_settings->stdout_mode)
        {
            fputs(msg, stdout);
        }

        update_printer_columns(pSoarAgent, msg);

        if (db_mode)
        {
            m_db->print_db(trace_msg, mode_info[No_Mode].prefix, msg);
        }
    }
}

void Output_Manager::printa_database(TraceMode mode, agent* pSoarAgent, MessageType msgType, const char* msg)
{
    soar_module::sqlite_statement*   target_statement = NIL;

    if (((msgType == trace_msg) && mode_info[mode].enabled) ||
            ((msgType == debug_msg) && mode_info[mode].enabled))
    {
        m_db->print_db(msgType, mode_info[mode].prefix, msg);
    }
}

void Output_Manager::print_sf(const char* format, ...)
{
    if (m_defaultAgent)
    {
        va_list args;
        char buf[OM_BUFFER_SIZE];

        va_start(args, format);
        vsnprint_sf(m_defaultAgent, buf, OM_BUFFER_SIZE, format, args);
        va_end(args);
        printa(m_defaultAgent, buf);
    }
}

void Output_Manager::vsnprint_sf(agent* thisAgent, char* dest, size_t dest_size, const char* format, va_list pargs)
{
    char* ch = dest;
    Symbol* sym;

    /* Apparently nested variadic calls need to have their argument list copied here.
     * If windows has issues with va_copy, might be able to just comment out that line
     * or use args = pargs.  Supposedly, way VC++ handles va_list doesn't need for it
     * to be copied. */
    va_list args;
    va_copy(args, pargs);

    while (true)
    {
        /* --- copy anything up to the first "%" --- */
        while ((*format != '%') && (*format != 0))
        {
            *(ch++) = *(format++);
        }
        if (*format == 0)
        {
            break;
        }
        if (*(format + 1) == 's')
        {
            char *ch2 = va_arg(args, char *);
            if (ch2 && strlen(ch2))
            {
                strcpy(ch, ch2);
                while (*ch) ch++;
            }
            format += 2;
        } else if (*(format + 1) == 'y')
        {
            sym = va_arg(args, Symbol*);
            if (sym)
            {
                (sym)->to_string(true, ch, dest_size - (ch - dest));
                while (*ch) ch++;
            } else {
                *(ch++) = '#';
            }
            format += 2;
        } else if (*(format + 1) == 'o')
        {
            test t = va_arg(args, test);
            test ct = NULL;
            sym = NULL;
            if (t)
            {
                if (t->type != CONJUNCTIVE_TEST)
                {
                    if (t->identity)
                    {
                        sym = thisAgent->variablizationManager->get_ovar_for_o_id(t->identity);
                        sym->to_string(true, ch, dest_size - (ch - dest));
                        while (*ch) ch++;
                    } else {
                        *(ch++) = '#';
                    }
                } else {
                    strcpy(ch, "{ ");
                    ch += 2;
                    for (cons *c = t->data.conjunct_list; c != NIL; c = c->rest)
                    {
                        ct = static_cast<test>(c->first);
                        if (ct && ct->identity)
                        {
                            sym = thisAgent->variablizationManager->get_ovar_for_o_id(ct->identity);
                            sym->to_string(true, ch, dest_size - (ch - dest));
                            while (*ch) ch++;
                        } else {
                            *(ch++) = '#';
                        }
                        *(ch++) = ' ';
                    }
                    *(ch++) = '}';;
                }
            } else {
                *(ch++) = '#';
            }
            format += 2;
        } else if (*(format + 1) == 'i')
        {
            SNPRINTF(ch, dest_size - (ch - dest), "%lld", va_arg(args, int64_t));
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 'u')
        {
            SNPRINTF(ch, dest_size - (ch - dest), "%llu", va_arg(args, uint64_t));
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 't')
        {
            test t = va_arg(args, test);
            if (t)
            {
                test_to_string(t, ch, dest_size - (ch - dest) );
                while (*ch) ch++;
            } else {
                *(ch++) = '#';
            }
            format += 2;
        } else if (*(format + 1) == 'g')
        {
            test t = va_arg(args, test);
            test ct = NULL;
            if (t)
            {
                if (t->type != CONJUNCTIVE_TEST)
                {
                    if (t->identity)
                    {
                        identity_to_string(thisAgent, t, ch, dest_size - (ch - dest) );
                        while (*ch) ch++;
                    } else {
                        *(ch++) = '#';
                    }
                } else {
                    strcpy(ch, "{ ");
                    ch += 2;
                    for (cons *c = t->data.conjunct_list; c != NIL; c = c->rest)
                    {
                        ct = static_cast<test>(c->first);
                        if (ct && ct->identity)
                        {
                            identity_to_string(thisAgent, ct, ch, dest_size - (ch - dest) );
                            while (*ch) ch++;
                        } else {
                            *(ch++) = '#';
                        }
                        *(ch++) = ' ';
                    }
                    *(ch++) = '}';;
                }
            } else {
                *(ch++) = '#';
            }
            format += 2;
        } else if (*(format + 1) == 'l')
        {
            condition* lc = va_arg(args, condition*);
            if (lc)
            {
                condition_to_string(thisAgent, lc, ch, dest_size - (ch - dest) );
                while (*ch) ch++;
            } else {
                *(ch++) = '#';
            }
            format += 2;
        } else if (*(format + 1) == 'a')
        {
            action* la = va_arg(args, action *);
            if (la)
            {
                action_to_string(thisAgent, la, ch, dest_size - (ch - dest) );
                while (*ch) ch++;
            } else {
                *(ch++) = '#';
            }
            format += 2;
        } else if (*(format + 1) == 'p')
        {
            preference* lp = va_arg(args, preference *);
            if (lp)
            {
                pref_to_string(thisAgent, lp, ch, dest_size - (ch - dest) );
                while (*ch) ch++;
            } else {
                *(ch++) = '#';
            }
            format += 2;
        } else if (*(format + 1) == 'w')
        {
            wme* lw = va_arg(args, wme *);
            if (lw)
            {
            wme_to_string(thisAgent, lw, ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            } else {
                *(ch++) = '#';
            }
            format += 2;
        } else if (*(format + 1) == 'd')
        {
            SNPRINTF(ch, dest_size - (ch - dest), "%d", va_arg(args, int));
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 'f')
        {
            if (thisAgent->output_settings->printer_output_column != 1)
            {
                *(ch++) = '\n';
            }
            format += 2;
        } else if (*(format + 1) == '1')
        {
            condition_list_to_string(thisAgent, va_arg(args, condition *), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == '2')
        {
            action_list_to_string(thisAgent, va_arg(args, action *), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == '3')
        {
            condition_cons_to_string(thisAgent, va_arg(args, cons*), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == '4')
        {
            cond_actions_to_string(thisAgent, va_arg(args, condition*), va_arg(args, action*), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == '5')
        {
            cond_prefs_to_string(thisAgent, va_arg(args, condition*), va_arg(args, preference*), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == '6')
        {
            cond_results_to_string(thisAgent, va_arg(args, condition*), va_arg(args, preference*), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == '7')
        {
            instantiation_to_string(thisAgent, va_arg(args, instantiation*), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == '8')
        {
            WM_to_string(thisAgent, ch, dest_size - (ch - dest));
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 'c')
        {
            char c = static_cast<char>(va_arg(args, int));
            SNPRINTF(ch, dest_size - (ch - dest), "%c", c);
            while (*ch) ch++;
            format += 2;
        } else
        {
            *(ch++) = *(format++);
        }
    }
    *ch = 0;
}

void Output_Manager::sprinta_sf(agent* thisAgent, char* dest, size_t dest_size, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprint_sf(thisAgent, dest, dest_size, format, args);
    va_end(args);
}

void Output_Manager::sprint_sf(char* dest, size_t dest_size, const char* format, ...)
{
    if (m_defaultAgent)
    {
        va_list args;
        va_start(args, format);
        vsnprint_sf(m_defaultAgent, dest, dest_size, format, args);
        va_end(args);
    }
}

void Output_Manager::debug_print(TraceMode mode, const char* msg)
{
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent)
    {
        std::cout << msg;
        return;
    }

    char buf[OM_BUFFER_SIZE];
    strcpy(buf, mode_info[mode].prefix);
    int s = strlen(buf);
    strcpy((buf + s), msg);
    printa(m_defaultAgent, buf);
}

void Output_Manager::debug_print_sf(TraceMode mode, const char* format, ...)
{
    if (!debug_mode_enabled(mode)) return;
    if (!m_defaultAgent)
    {
        std::cout << format;
        return;
    }

    va_list args;
    char buf[OM_BUFFER_SIZE];

    strcpy(buf, mode_info[mode].prefix);
    int s = strlen(buf);
    va_start(args, format);
    vsnprint_sf(m_defaultAgent, (buf+s), OM_BUFFER_SIZE, format, args);
    va_end(args);
    printa(m_defaultAgent, buf);
}

void Output_Manager::debug_print_sf_noprefix(TraceMode mode, const char* format, ...)
{
    if (!debug_mode_enabled(mode)) return;
    if (!m_defaultAgent)
    {
        std::cout << format;
        return;
    }

    va_list args;
    char buf[OM_BUFFER_SIZE];

    va_start(args, format);
    vsnprint_sf(m_defaultAgent, buf, OM_BUFFER_SIZE, format, args);
    va_end(args);

    printa(m_defaultAgent, buf);

}

void Output_Manager::debug_print_header(TraceMode mode, Print_Header_Type whichHeaders, const char* format, ...)
{
    if (!debug_mode_enabled(mode)) return;
    if (!m_defaultAgent)
    {
        std::cout << format;
        return;
    }

    if ((whichHeaders == PrintBoth) || (whichHeaders == PrintBefore))
        debug_print(mode, "=========================================================\n");
    va_list args;
    char buf[OM_BUFFER_SIZE];

    strcpy(buf, mode_info[mode].prefix);
    int s = strlen(buf);
    va_start(args, format);
    vsnprint_sf(m_defaultAgent, (buf+s), OM_BUFFER_SIZE, format, args);
    va_end(args);
    if (strlen(buf) > s)
    {
        printa(m_defaultAgent, buf);
    }
    if ((whichHeaders == PrintBoth) || (whichHeaders == PrintAfter))
        debug_print(mode, "=========================================================\n");
}

void Output_Manager::debug_start_fresh_line(TraceMode mode)
{
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent)
    {
        std::cout << std::endl;
        return;
    }

    if ((global_printer_output_column != 1) || (m_defaultAgent->output_settings->printer_output_column != 1))
    {
        printa(m_defaultAgent, "\n");
    }

}
