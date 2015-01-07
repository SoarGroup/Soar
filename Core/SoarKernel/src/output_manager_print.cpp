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
//#define PRINT_BUFSIZE 70000   /* --- size of output buffer for a calls to print routines --- */

void Output_Manager::printa_sf(agent* pSoarAgent, const char* format, ...)
{
    va_list args;
    char buf[PRINT_BUFSIZE];

    va_start(args, format);
    vsnprint_sf(pSoarAgent, buf, PRINT_BUFSIZE, format, args);
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
        char buf[PRINT_BUFSIZE];

        va_start(args, format);
        vsnprint_sf(m_defaultAgent, buf, PRINT_BUFSIZE, format, args);
        va_end(args);
        printa(m_defaultAgent, buf);
    }
}

void Output_Manager::vsnprint_sf(agent* thisAgent, char* dest, size_t dest_size, const char* format, va_list args)
{
    char* ch = dest;

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
        /* --- handle the %-thingy --- */
        /* the size of the remaining buffer (after ch) is
            the difference between the address of ch and
            the address of the beginning of the buffer
         */
        if (*(format + 1) == 's')
        {
            char *ch2 = va_arg(args, char *);
            if (ch2 && strlen(ch2))
            {
                //SNPRINTF(ch, count - (ch - dest), "%s", va_arg(args, char *));
                strcpy(ch, ch2);
                while (*ch) ch++;
            }
            format += 2;
        } else if (*(format + 1) == 'y')
        {
            Symbol* sym = va_arg(args, Symbol*);
            if (sym)
            {
                (sym)->to_string(true, ch, dest_size - (ch - dest));
                while (*ch) ch++;
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
            test_to_string(va_arg(args, test_info *), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 'l')
        {
            condition_to_string(thisAgent, va_arg(args, condition*), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 'a')
        {
            action_to_string(thisAgent, va_arg(args, action *), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 'p')
        {
            pref_to_string(thisAgent, va_arg(args, preference *), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 'w')
        {
            wme_to_string(thisAgent, va_arg(args, wme *), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
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
            WM_to_string(thisAgent, ch, dest_size - (ch - dest), true);
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

    if (!m_defaultAgent) return;

    char buf[PRINT_BUFSIZE];
    strcpy(buf, mode_info[mode].prefix);
    int s = strlen(buf);
    strcpy((buf + s), msg);
    printa(m_defaultAgent, buf);
}

void Output_Manager::debug_print_sf(TraceMode mode, const char* format, ...)
{
    if (!debug_mode_enabled(mode)) return;
    if (!m_defaultAgent) return;

    va_list args;
    char buf[PRINT_BUFSIZE];

    strcpy(buf, mode_info[mode].prefix);
    int s = strlen(buf);
    va_start(args, format);
    vsnprint_sf(m_defaultAgent, (buf+s), PRINT_BUFSIZE, format, args);
    va_end(args);
    printa(m_defaultAgent, buf);
}

void Output_Manager::debug_print_sf_noprefix(TraceMode mode, const char* format, ...)
{
    if (!debug_mode_enabled(mode)) return;
    if (!m_defaultAgent) return;

    va_list args;
    char buf[PRINT_BUFSIZE];

    va_start(args, format);
    vsnprint_sf(m_defaultAgent, buf, PRINT_BUFSIZE, format, args);
    va_end(args);

    printa(m_defaultAgent, buf);

}

void Output_Manager::debug_print_header(TraceMode mode, Print_Header_Type whichHeaders, const char* format, ...)
{
    if (!debug_mode_enabled(mode)) return;
    if (!m_defaultAgent) return;

    if ((whichHeaders == PrintBoth) || (whichHeaders == PrintBefore))
        debug_print(mode, "=========================================================\n");
    va_list args;
    char buf[PRINT_BUFSIZE];

    strcpy(buf, mode_info[mode].prefix);
    int s = strlen(buf);
    va_start(args, format);
    vsnprint_sf(m_defaultAgent, (buf+s), PRINT_BUFSIZE, format, args);
    va_end(args);
    printa(m_defaultAgent, buf);
    if ((whichHeaders == PrintBoth) || (whichHeaders == PrintAfter))
        debug_print(mode, "=========================================================\n");
}

void Output_Manager::debug_start_fresh_line(TraceMode mode)
{
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;

    if ((global_printer_output_column != 1) || (m_defaultAgent->output_settings->printer_output_column != 1))
    {
        printa(m_defaultAgent, "\n");
    }

}
