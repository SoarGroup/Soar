/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
    print.cpp
------------------------------------------------------------------ */

#include "output_manager.h"

#include "agent.h"
#include "callback.h"
#include "ebc.h"
#include "instantiation.h"
#include "preference.h"
#include "print.h"
#include "production_reorder.h"
#include "rete.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "soar_instance.h"
#include "symbol.h"
#include "test.h"
#include "working_memory.h"
#include "xml.h"

#include <iostream>
#include <cstdarg>

void Output_Manager::printa_sf(agent* pSoarAgent, const char* format, ...)
{
    va_list args;
    std::string buf;

    va_start(args, format);
    vsnprint_sf(pSoarAgent, buf, format, args);
    va_end(args);
    printa(pSoarAgent, buf.c_str());
}
void Output_Manager::printa(agent* pSoarAgent, const char* msg)
{
    if (pSoarAgent)
    {
//        xml_generate_message(pSoarAgent, const_cast<char*>(msg));
        if (!pSoarAgent->output_settings->print_enabled) return;
        if (pSoarAgent->output_settings->callback_mode)
        {
            soar_invoke_callbacks(pSoarAgent, PRINT_CALLBACK, static_cast<soar_call_data>(const_cast<char*>(msg)));
        }
        if (stdout_mode)
        {
            fputs(msg, stdout);
        }

        update_printer_columns(pSoarAgent, msg);
    }
}
/* A way to do variadic printing with std::strings that might be worth using,
 * though we might just want to something less likely to cause crashes because
 * of accidental type mismatches */
//std::string out( std::string format, ... )
//{
//    va_list args, args_copy ;
//    va_start( args, format ) ;
//    va_copy( args_copy, args ) ;
//
//    const auto sz = std::vsnprintf( nullptr, 0, format.c_str(), args ) + 1 ;
//
//    try
//    {
//        std::string result( sz, ' ' ) ;
//        std::vsnprintf( &result.front(), sz, format.c_str(), args_copy ) ;
//
//        va_end(args_copy) ;
//        va_end(args) ;
//
//        // do whatever else with result
//
//        return result ;
//    }
//
//    catch( const std::bad_alloc& )
//    {
//        va_end(args_copy) ;
//        va_end(args) ;
//        throw ;
//    }
//}

void Output_Manager::print_sf(const char* format, ...)
{
    if (m_defaultAgent)
    {
        va_list args;
        std::string buf;

        va_start(args, format);
        vsnprint_sf(m_defaultAgent, buf, format, args);
        va_end(args);
        printa(m_defaultAgent, buf.c_str());
    }
}

void Output_Manager::sprinta_sf(agent* thisAgent, std::string &destString, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprint_sf(thisAgent, destString, format, args);
    va_end(args);
    return;
}

/* Same as above but changes the destination pointer to point to the next character to write to.  Used
 * by other output manager functions to build up a string incrementally */

size_t Output_Manager::sprinta_sf_cstr(agent* thisAgent, char* dest, size_t dest_size, const char* format, ...)
{
    if (!dest_size) return 0;
    va_list args;
    std::string buf;

    va_start(args, format);
    vsnprint_sf(m_defaultAgent, buf, format, args);
    va_end(args);

    return om_strncpy(dest, buf.c_str(), dest_size, buf.length());
}

void Output_Manager::sprint_sf(std::string &destString, const char* format, ...)
{
    if (m_defaultAgent)
    {
        va_list args;
        va_start(args, format);
        vsnprint_sf(m_defaultAgent, destString, format, args);
        va_end(args);
    }
}

size_t Output_Manager::sprint_sf_cstr(char* dest, size_t dest_size, const char* format, ...)
{
    if (!dest_size) return 0;
    std::string buf;

    if (m_defaultAgent)
    {
        va_list args;
        va_start(args, format);
        vsnprint_sf(m_defaultAgent, buf, format, args);
        va_end(args);
    }
    return om_strncpy(dest, buf.c_str(), dest_size, buf.length());
}
void Output_Manager::debug_print(TraceMode mode, const char* msg)
{
    if (!is_trace_enabled(mode)) return;

    if (!m_defaultAgent)
    {
        std::cout << msg;
        return;
    }

    std::string buf;
    buffer_start_fresh_line(m_defaultAgent, buf);
    buf.append(mode_info[mode].prefix);
    buf.append(msg);
    printa(m_defaultAgent, buf.c_str());
}

void Output_Manager::debug_print_sf(TraceMode mode, const char* format, ...)
{
    if (!is_trace_enabled(mode)) return;
    if (!m_defaultAgent)
    {
        std::cout << format;
        return;
    }

    va_list args;
    std::string buf;
    buffer_start_fresh_line(m_defaultAgent, buf);
    buf.append(mode_info[mode].prefix);

    va_start(args, format);
    vsnprint_sf(m_defaultAgent, buf, format, args);
    va_end(args);
    printa(m_defaultAgent, buf.c_str());
}

void Output_Manager::debug_print_sf_noprefix(TraceMode mode, const char* format, ...)
{
    if (!is_trace_enabled(mode)) return;
    if (!m_defaultAgent)
    {
        std::cout << format;
        return;
    }

    va_list args;
    std::string buf;

    va_start(args, format);
    vsnprint_sf(m_defaultAgent, buf, format, args);
    va_end(args);

    printa(m_defaultAgent, buf.c_str());
}

void Output_Manager::debug_print_header(TraceMode mode, Print_Header_Type whichHeaders, const char* format, ...)
{
    if (!is_trace_enabled(mode)) return;
    if (!m_defaultAgent)
    {
        std::cout << format;
        return;
    }

    std::string buf;
    buffer_start_fresh_line(m_defaultAgent, buf);
    if ((whichHeaders == PrintBoth) || (whichHeaders == PrintBefore))
        buf.append("=========================================================\n");
    buf.append(mode_info[mode].prefix);

    va_list args;

    va_start(args, format);
    vsnprint_sf(m_defaultAgent, buf, format, args);
    va_end(args);

    if ((whichHeaders == PrintBoth) || (whichHeaders == PrintAfter))
        buf.append("=========================================================\n");

    printa(m_defaultAgent, buf.c_str());
}

void Output_Manager::buffer_start_fresh_line(agent* thisAgent, std::string &destString)
{
    if (!thisAgent)
    {
        std::cout << std::endl;
        return;
    }

    if (destString.empty())
    {
        if ((global_printer_output_column != 1) || (thisAgent->output_settings->printer_output_column != 1))
        {
            destString.append("\n");
        }
    } else {
        if (destString.back() != '\n')
        {
            destString.append("\n");
        }
    }
}

void Output_Manager::vsnprint_sf(agent* thisAgent, std::string &destString, const char* format, va_list pargs)
{
    Symbol* sym;
    test t, ct;
    char ch = 0;
    char* ch2 = 0;
	int next_column, indent_amount, next_position, i=0;
	size_t m;
    std::string sf = format;

    va_list args;
    va_copy(args, pargs);

    m = sf.length();
    while (i<m)
    {
        ch = sf.at(i);

        if (ch == '%')
        {
            i++;
            if (i<m)
            {
                ch = sf.at(i);
                switch(ch)
                {
                    case 's':
                    {
                        ch2 = va_arg(args, char *);
                        if (ch2)
                        {
                            destString += ch2;
                        }
                    }
                    break;

                    case 'y':
                    {
                        sym = va_arg(args, Symbol*);
                        if (sym)
                        {
                            destString += sym->to_string(true);
                        } else {
                            destString += '#';
                        }
                    }
                    break;

                    case 'd':
                    {
                        destString += std::to_string(va_arg(args, int64_t));
                    }
                    break;

                    case 'u':
                    {
                        destString += std::to_string(va_arg(args, uint64_t));
                    }
                    break;

                    case 'f':
                    {
                        destString += std::to_string(va_arg(args, double));
                    }
                    break;

                    case 't':
                    {
                        t = va_arg(args, test);
                        if (t)
                        {
                            test_to_string(t, destString);
                        } else {
                            destString += '#';
                        }
                    }
                    break;

                    case '-':
                    case '=':
                    {
                        indent_amount = 0;
                        next_position = (this->get_printer_output_column(thisAgent) + destString.length());
                        for (next_column = 0; next_column < MAX_COLUMNS; next_column++)
                        {
                            if (next_position < m_column_indent[next_column]) {
                                indent_amount = (m_column_indent[next_column] - next_position);
                                break;
                            }
                        }
                        if (indent_amount > 0) {
                            if (ch == '-')
                            {
                                destString.append(indent_amount , ' ');
                            } else {
                                destString.append(indent_amount , '.');
                            }
                        }
                    }
                    break;

                    case 'g':
                    {
                        t = va_arg(args, test);
                        ct = NULL;
                        if (t)
                        {
                            if (t->type != CONJUNCTIVE_TEST)
                            {
                                if (t->identity)
                                {
                                    if (t->type != EQUALITY_TEST)
                                    {
                                        destString += test_type_to_string(t->type);
                                    }
                                    destString += "[";
                                    destString += std::to_string(t->identity);
                                    destString += "]";
                                } else {
                                    test_to_string(t, destString);
                                }
                            } else {
                                destString += "{ ";
                                bool isFirst = true;
                                for (cons *c = t->data.conjunct_list; c != NIL; c = c->rest)
                                {
                                    ct = static_cast<test>(c->first);
                                    assert(ct);
                                    if (ct->identity)
                                    {
                                        if (ct->type != EQUALITY_TEST)
                                        {
                                            destString += test_type_to_string(ct->type);
                                        }
                                        if (!isFirst) destString += ' '; else isFirst = false;
                                        destString += "[";
                                        destString += std::to_string(ct->identity);
                                        destString += "]";
                                    } else {
                                        test_to_string(ct, destString);
                                    }
                                    destString += ' ';
                                }
                                destString += '}';
                            }
                        } else {
                            destString += '#';
                        }
                    }
                    break;

                    case 'l':
                    {
                        condition* lc = va_arg(args, condition*);
                        if (lc)
                        {
                            condition_to_string(thisAgent, lc, destString);
                        } else {
                            destString += '#';
                        }
                    }
                    break;

                    case 'a':
                    {
                        action* la = va_arg(args, action *);
                        if (la)
                        {
                            this->action_to_string(thisAgent, la, destString);

                        } else {
                            destString += '#';
                        }
                    }
                    break;

                    case 'n':
                    {
                        cons* la = va_arg(args, cons *);
                        if (la)
                        {
                            this->rhs_value_to_string(funcall_list_to_rhs_value(la), destString);

                        } else {
                            destString += '#';
                        }
                    }
                    break;

                    case 'r':
                    {
                        char* la = va_arg(args, char *);
                        if (la)
                        {
                            this->rhs_value_to_string(la, destString, NULL );

                        } else {
                            destString += '#';
                        }
                    }
                    break;

                    case 'p':
                    {
                        preference* lp = va_arg(args, preference *);
                        if (lp)
                        {
                            pref_to_string(thisAgent, lp, destString);

                        } else {
                            destString += '#';
                        }
                    }
                    break;

                    case 'w':
                    {
                        wme* lw = va_arg(args, wme *);
                        if (lw)
                        {
                            wme_to_string(thisAgent, lw, destString);
                        } else {
                            destString += '#';
                        }
                    }
                    break;

                    case 'e':
                    {
                        if (thisAgent->output_settings->printer_output_column != 1)
                        {
                            destString += '\n';
                        }
                    }
                    break;

                    case '1':
                    {
                        condition* temp = va_arg(args, condition *);
                        if (temp)
                        {
                            condition_list_to_string(thisAgent, temp, destString);
                        }
                    }
                    break;

                    case '2':
                    {
                        action* temp = va_arg(args, action *);
                        if (temp)
                        {
                            action_list_to_string(thisAgent, temp, destString);
                        }
                    }
                    break;

                    case '3':
                    {
                        cons* temp = va_arg(args, cons*);
                        if (temp)
                        {
                            condition_cons_to_string(thisAgent, temp, destString);
                        }
                    }
                    break;

                    case '4':
                    {
                        cond_actions_to_string(thisAgent, va_arg(args, condition*), va_arg(args, action*), destString);
                    }
                    break;

                    case '5':
                    {
                        cond_prefs_to_string(thisAgent, va_arg(args, condition*), va_arg(args, preference*), destString);
                    }
                    break;

                    case '6':
                    {
                        cond_results_to_string(thisAgent, va_arg(args, condition*), va_arg(args, preference*), destString);
                    }
                    break;

                    case '7':
                    {
                        instantiation* temp = va_arg(args, instantiation*);
                        if (temp)
                        {
                            instantiation_to_string(thisAgent, temp, destString);
                        }
                    }
                    break;

                    case 'c':
                    {
                        destString += static_cast<char>(va_arg(args, int));
                    }
                    break;

                    case '%':
                    {
                        destString += '%';
                    }
                    break;

                    default:
                    {
                        destString += '%';
                        destString += ch;
                        //i = m; //get out of loop
                    }
                }
            }
        }
        else
        {
            destString += ch;
        }
        i++;
    }
    va_end(args);
}

