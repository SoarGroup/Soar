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
#define OM_BUFFER_SIZE 7000   /* --- size of output buffer for a calls to print routines --- */

void Output_Manager::printa_sf(agent* pSoarAgent, const char* format, ...)
{
    va_list args;
    char buf[OM_BUFFER_SIZE+1];
    buf[OM_BUFFER_SIZE] = 0;

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
        char *buf = 0;
        buf = new char[OM_BUFFER_SIZE+1];

        va_start(args, format);
        vsnprint_sf(m_defaultAgent, buf, OM_BUFFER_SIZE, format, args);
        va_end(args);
        printa(m_defaultAgent, buf);
        delete [] buf;
    }
}

size_t Output_Manager::vsnprint_sf(agent* thisAgent, char* dest, size_t dest_size, const char* format, va_list pargs)
{
    if (!dest_size) return 0;
    char* ch = dest;
    Symbol* sym;
    size_t buffer_left = dest_size;

    /* Apparently nested variadic calls need to have their argument list copied here.
     * If windows has issues with va_copy, might be able to just comment out that line
     * or use args = pargs.  Supposedly, way VC++ handles va_list doesn't need for it
     * to be copied. */
    va_list args;
    va_copy(args, pargs);

    while (true)
    {
        /* MToDo | Need safer way to copy this */
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
            if (ch2)
            {
                buffer_left = om_strcpy(&ch, ch2, buffer_left);
            }
            format += 2;
        } else if (*(format + 1) == 'y')
        {
            sym = va_arg(args, Symbol*);
            if (sym)
            {
                buffer_left = om_sym_to_string(sym, true, &ch, buffer_left);
            } else {
                buffer_left = om_charcpy(&ch, '#', buffer_left);
            }
            format += 2;
        } else if (*(format + 1) == 'i')
        {
            buffer_left = om_snprintf(&ch, buffer_left, SNPRINTF(ch, buffer_left, "%lld", va_arg(args, int64_t)));
            format += 2;
        } else if (*(format + 1) == 'u')
        {
            buffer_left = om_snprintf(&ch, buffer_left, SNPRINTF(ch, buffer_left, "%llu", va_arg(args, uint64_t)));
            format += 2;
        } else if (*(format + 1) == 't')
        {
            test t = va_arg(args, test);
            if (t)
            {
                buffer_left = test_to_string(t, &ch, buffer_left);
            } else {
                buffer_left = om_charcpy(&ch, '#', buffer_left);
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
                        buffer_left = test_to_string(t, &ch, buffer_left);
                        buffer_left = om_strcpy(&ch, " [o", buffer_left);
                        buffer_left = om_snprintf(&ch, buffer_left, SNPRINTF(ch, buffer_left, "%llu", t->identity));
                        sym = thisAgent->variablizationManager->get_ovar_for_o_id(t->identity);
                        buffer_left = om_sym_to_string(sym, true, &ch, buffer_left);
                        buffer_left = om_charcpy(&ch, ']', buffer_left);
                    } else {
                        buffer_left = test_to_string(t, &ch, buffer_left);
                        buffer_left = om_strcpy(&ch, " [o0]", buffer_left);
                    }
                } else {
                    buffer_left = om_strcpy(&ch, "{ ", buffer_left);
                    for (cons *c = t->data.conjunct_list; c != NIL; c = c->rest)
                    {
                        ct = static_cast<test>(c->first);
                        assert(ct);
                        if (t->identity)
                        {
                            buffer_left = test_to_string(t, &ch, buffer_left);
                            buffer_left = om_strcpy(&ch, " [o", buffer_left);
                            buffer_left = om_snprintf(&ch, buffer_left, SNPRINTF(ch, buffer_left, "%llu", t->identity));
                            sym = thisAgent->variablizationManager->get_ovar_for_o_id(t->identity);
                            buffer_left = om_sym_to_string(sym, true, &ch, buffer_left);
                            buffer_left = om_charcpy(&ch, ']', buffer_left);
                        } else {
                            buffer_left = test_to_string(t, &ch, buffer_left);
                            buffer_left = om_strcpy(&ch, " [o0]", buffer_left);
                        }
                        buffer_left = om_charcpy(&ch, ' ', buffer_left);
                    }
                    buffer_left = om_charcpy(&ch, '}', buffer_left);
                }
            } else {
                buffer_left = om_charcpy(&ch, '#', buffer_left);
            }
            format += 2;
        } else if (*(format + 1) == 'l')
        {
            condition* lc = va_arg(args, condition*);
            if (lc)
            {
                buffer_left = condition_to_string(thisAgent, lc, &ch, buffer_left);
            } else {
                buffer_left = om_charcpy(&ch, '#', buffer_left);
            }
            format += 2;
        } else if (*(format + 1) == 'a')
        {
            action* la = va_arg(args, action *);
            if (la)
            {
                buffer_left = this->action_to_string(thisAgent, la, &ch, buffer_left);

            } else {
                buffer_left = om_charcpy(&ch, '#', buffer_left);
            }
            format += 2;
        } else if (*(format + 1) == 'n')
        {
            list* la = va_arg(args, list *);
            if (la)
            {
                buffer_left = this->rhs_value_to_string(thisAgent, funcall_list_to_rhs_value(la), &ch, buffer_left);

            } else {
                buffer_left = om_charcpy(&ch, '#', buffer_left);
            }
            format += 2;
        } else if (*(format + 1) == 'r')
        {
            char* la = va_arg(args, char *);
            if (la)
            {
                buffer_left = this->rhs_value_to_string(thisAgent, la, &ch, buffer_left, NULL );

            } else {
                buffer_left = om_charcpy(&ch, '#', buffer_left);
            }
            format += 2;
        } else if (*(format + 1) == 'p')
        {
            preference* lp = va_arg(args, preference *);
            if (lp)
            {
                buffer_left = pref_to_string(thisAgent, lp, &ch, buffer_left);

            } else {
                buffer_left = om_charcpy(&ch, '#', buffer_left);
            }
            format += 2;
        } else if (*(format + 1) == 'w')
        {
            wme* lw = va_arg(args, wme *);
            if (lw)
            {
                buffer_left = wme_to_string(thisAgent, lw, &ch, buffer_left);
            } else {
                buffer_left = om_charcpy(&ch, '#', buffer_left);
            }
            format += 2;
        } else if (*(format + 1) == 'd')
        {
            buffer_left = om_snprintf(&ch, buffer_left, SNPRINTF(ch, buffer_left, "%d", va_arg(args, int)));
            format += 2;
        } else if (*(format + 1) == 'f')
        {
            if (thisAgent->output_settings->printer_output_column != 1)
            {
                buffer_left = om_charcpy(&ch, '\n', buffer_left);
            }
            format += 2;
        } else if (*(format + 1) == '1')
        {
            condition* temp = va_arg(args, condition *);
            if (temp)
            {
                buffer_left = condition_list_to_string(thisAgent, temp, &ch, buffer_left);
            }
                format += 2;
        } else if (*(format + 1) == '2')
        {
            action* temp = va_arg(args, action *);
            if (temp)
            {
                buffer_left = action_list_to_string(thisAgent, temp, &ch, buffer_left);
            }
                format += 2;
        } else if (*(format + 1) == '3')
        {
            cons* temp = va_arg(args, cons*);
            if (temp)
            {
                buffer_left = condition_cons_to_string(thisAgent, temp, &ch, buffer_left);
            }
                format += 2;
        } else if (*(format + 1) == '4')
        {
            buffer_left = cond_actions_to_string(thisAgent, va_arg(args, condition*), va_arg(args, action*), &ch, buffer_left);
            format += 2;
        } else if (*(format + 1) == '5')
        {
            buffer_left = cond_prefs_to_string(thisAgent, va_arg(args, condition*), va_arg(args, preference*), &ch, buffer_left);
            format += 2;
        } else if (*(format + 1) == '6')
        {
            buffer_left = cond_results_to_string(thisAgent, va_arg(args, condition*), va_arg(args, preference*), &ch, buffer_left);
            format += 2;
        } else if (*(format + 1) == '7')
        {
            instantiation* temp = va_arg(args, instantiation*);
            if (temp)
            {
                buffer_left = instantiation_to_string(thisAgent, temp, &ch, buffer_left);
            }
            format += 2;
        } else if (*(format + 1) == '8')
        {
            buffer_left = WM_to_string(thisAgent, &ch, buffer_left);
            format += 2;
        } else if (*(format + 1) == 'c')
        {
            char c = static_cast<char>(va_arg(args, int));
            buffer_left = om_snprintf(&ch, buffer_left, SNPRINTF(ch, dest_size - (ch - dest), "%c", c));
            format += 2;
        } else
        {
            buffer_left = om_charcpy(&ch, '\n', buffer_left);
            buffer_left = om_charcpy(&ch, *(format++), buffer_left);
        }
    }
    va_end(args);
    return buffer_left;
}

size_t Output_Manager::sprinta_sf(agent* thisAgent, char* dest, size_t dest_size, const char* format, ...)
{
    if (!dest_size) return 0;
    size_t buffer_left;
    va_list args;
    va_start(args, format);
    buffer_left = vsnprint_sf(thisAgent, dest, dest_size, format, args);
    va_end(args);
    return buffer_left;
}

/* Same as above but changes the destination pointer to point to the next character to write to.  Used
 * by other output manager functions to build up a string incrementally */

size_t Output_Manager::sprinta_sf_internal(agent* thisAgent, char* &dest, size_t dest_size, const char* format, ...)
{
    if (!dest_size) return 0;
    size_t buffer_left;
    va_list args;
    va_start(args, format);
    buffer_left = vsnprint_sf(thisAgent, dest, dest_size, format, args);
    va_end(args);
    dest = *(dest + (dest - buffer_left));
    return buffer_left;
}

size_t Output_Manager::sprint_sf(char* &dest, size_t dest_size, const char* format, ...)
{
    if (!dest_size) return 0;
    size_t buffer_left = dest_size;
    if (m_defaultAgent)
    {
        size_t buffer_left;
        va_list args;
        va_start(args, format);
        buffer_left = vsnprint_sf(m_defaultAgent, dest, dest_size, format, args);
        va_end(args);
    }
    return buffer_left;
}

void Output_Manager::debug_print(TraceMode mode, const char* msg)
{
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent)
    {
        std::cout << msg;
        return;
    }

    char buf[OM_BUFFER_SIZE+1];
    buf[OM_BUFFER_SIZE] = 0;
    strncpy(buf, mode_info[mode].prefix, OM_BUFFER_SIZE);
    int s = strlen(buf);
    strncpy((buf + s), msg, OM_BUFFER_SIZE - s);
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
    char buf[OM_BUFFER_SIZE+1];
    buf[OM_BUFFER_SIZE] = 0;
    strncpy(buf, mode_info[mode].prefix, OM_BUFFER_SIZE);
    int s = strlen(buf);
    va_start(args, format);
    vsnprint_sf(m_defaultAgent, (buf+s), OM_BUFFER_SIZE - s, format, args);
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
    char buf[OM_BUFFER_SIZE+1];
    buf[OM_BUFFER_SIZE] = 0;

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
    char buf[OM_BUFFER_SIZE+1];
    buf[OM_BUFFER_SIZE] = 0;

    strncpy(buf, mode_info[mode].prefix, OM_BUFFER_SIZE);
    int s = strlen(buf);
    va_start(args, format);
    vsnprint_sf(m_defaultAgent, (buf+s), OM_BUFFER_SIZE - s, format, args);
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
