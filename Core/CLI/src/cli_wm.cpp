/////////////////////////////////////////////////////////////////
// learn command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_AgentSML.h"
#include "sml_Names.h"
#include "sml_KernelSML.h"
#include "sml_Utils.h"

#include "agent.h"
#include "cli_CommandLineInterface.h"
#include "cmd_settings.h"
#include "decide.h"
#include "lexer.h"
#include "mem.h"
#include "misc.h"
#include "output_manager.h"
#include "parser.h"
#include "print.h"
#include "production.h"
#include "rete.h"
#include "slot.h"
#include "soar_TraceNames.h"
#include "symbol_manager.h"
#include "symbol.h"
#include "working_memory_activation.h"
#include "working_memory.h"
#include "xml.h"

#include <algorithm>

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoWM(std::vector<std::string>& argv, const std::string& pCmd)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (pCmd.empty())
    {
        thisAgent->command_params->wm_params->print_summary(thisAgent);
        return true;
    }
    soar_module::param* my_param = thisAgent->command_params->wm_params->get(pCmd.c_str());
    if (!my_param)
    {
            return SetError("Invalid wm command.  Use 'wm ?' to see a list of valid settings.");
    }
    if (my_param == thisAgent->command_params->wm_params->add_cmd)
    {
        return ParseWMEAdd(argv);
    }
    else if (my_param == thisAgent->command_params->wm_params->remove_cmd)
    {
        return ParseWMERemove(argv);
    }
    else if (my_param == thisAgent->command_params->wm_params->watch_cmd)
    {
        return ParseWMEWatch(argv);
    }
    else if (my_param == thisAgent->command_params->wm_params->wma_cmd)
    {
        return ParseWMA(argv);
    }
    else if ((my_param == thisAgent->command_params->wm_params->help_cmd) || (my_param == thisAgent->command_params->wm_params->qhelp_cmd))
    {
        thisAgent->command_params->wm_params->print_settings(thisAgent);
    }
    return false;
}

bool CommandLineInterface::ParseWMEAdd(std::vector< std::string >& argv)
{
    if (argv.size() < 5)
    {
        return SetError("Syntax: wm add id [^]attribute value [+]");
    }

    unsigned attributeIndex = (argv[3] == "^") ? 4 : 3;

    if (argv.size() < (attributeIndex + 2))
    {
        return SetError("Syntax: wm add id [^]attribute value [+]");
    }
    if (argv.size() > (attributeIndex + 3))
    {
        return SetError("Syntax: wm add id [^]attribute value [+]");
    }

    bool acceptable = false;
    if (argv.size() > (attributeIndex + 2))
    {
        if (argv[attributeIndex + 2] != "+")
        {
            return SetError("Syntax: wm add id [^]attribute value [+]");
        }
        acceptable = true;
    }

    return DoAddWME(argv[2], argv[attributeIndex], argv[attributeIndex + 1], acceptable);
}
bool CommandLineInterface::ParseWMERemove(std::vector< std::string >& argv)
{
    // Exactly one argument
    if (argv.size() < 3)
    {
        return SetError("Syntax: wm remove <time-tag>");
    }
    if (argv.size() > 3)
    {
        return SetError("Syntax: wm remove <time-tag>");
    }

    uint64_t timetag = 0;
    from_string(timetag, argv[2]);
    if (!timetag)
    {
        return SetError("<time-tag> must be positive");
    }

    return DoRemoveWME(timetag);
}
bool CommandLineInterface::ParseWMEWatch(std::vector< std::string >& argv)
{

    cli::Options opt;
    OptionsData optionsData[] =
    {
        {'a', "add-filter",        OPTARG_NONE},
        {'r', "remove-filter",    OPTARG_NONE},
        {'l', "list-filter",    OPTARG_NONE},
        {'R', "reset-filter",    OPTARG_NONE},
        {'t', "type",            OPTARG_REQUIRED},
        {0, 0, OPTARG_NONE}
    };

    cli::eWatchWMEsMode mode = cli::WATCH_WMES_LIST;
    cli::WatchWMEsTypeBitset type(0);

    for (;;)
    {
        if (!opt.ProcessOptions(argv, optionsData))
        {
            return SetError(opt.GetError().c_str());
        }
        ;
        if (opt.GetOption() == -1)
        {
            break;
        }

        switch (opt.GetOption())
        {
            case 'a':
                mode = cli::WATCH_WMES_ADD;
                break;
            case 'r':
                mode = cli::WATCH_WMES_REMOVE;
                break;
            case 'l':
                mode = cli::WATCH_WMES_LIST;
                break;
            case 'R':
                mode = cli::WATCH_WMES_RESET;
                break;
            case 't':
            {
                std::string typeString = opt.GetOptionArgument();
                if (typeString == "adds")
                {
                    type.set(cli::WATCH_WMES_TYPE_ADDS);
                }
                else if (typeString == "removes")
                {
                    type.set(cli::WATCH_WMES_TYPE_REMOVES);
                }
                else if (typeString == "both")
                {
                    type.set(cli::WATCH_WMES_TYPE_ADDS);
                    type.set(cli::WATCH_WMES_TYPE_REMOVES);
                }
                else
                {
                    return SetError("Invalid wme filter type, got: " + typeString);
                }
            }
            break;
            default:
                return SetError("Invalid argument for wm watch command.");
                break;
        }
    }

    if (mode == cli::WATCH_WMES_ADD || mode == cli::WATCH_WMES_REMOVE)
    {
        // type required
        if (type.none())
        {
            return SetError("Wme type required.");
        }

        // check for too few/many args
        if (opt.GetNonOptionArguments() > 4)
        {
            return SetError("Syntax: wm watch -[a|r]  -t <type>  <pattern>\nwm watch -[l|R] [-t <type>]");
        }
        if (opt.GetNonOptionArguments() < 4)
        {
            return SetError("Syntax: wm watch -[a|r]  -t <type>  <pattern>\nwm watch -[l|R] [-t <type>]");
        }

        int optind = opt.GetArgument() - opt.GetNonOptionArguments() + 1;
        return DoWatchWMEs(mode, type, &argv[optind], &argv[optind + 1], &argv[optind + 2]);
    }

    // no additional arguments
    if (!opt.CheckNumNonOptArgs(1, 1))
    {
        return SetError("Syntax: wm watch -[a|r]  -t <type>  <pattern>\nwm watch -[l|R] [-t <type>]");
    }

    return DoWatchWMEs(mode, type);

}
bool CommandLineInterface::ParseWMA(std::vector< std::string >& argv)
{

    cli::Options opt;
    OptionsData optionsData[] =
    {
        {'g', "get",        OPTARG_NONE},
        {'h', "history",    OPTARG_NONE},
        {'s', "set",        OPTARG_NONE},
        {'S', "stats",      OPTARG_NONE},
        {'t', "timers",     OPTARG_NONE},
        {0, 0, OPTARG_NONE} // null
    };

    char option = 0;

    for (;;)
    {
        if (!opt.ProcessOptions(argv, optionsData))
        {
            return SetError(opt.GetError().c_str());
        }

        if (opt.GetOption() == -1)
        {
            break;
        }

        if (option != 0)
        {
            return SetError("wm activation takes only one option at a time.");
        }

        option = static_cast<char>(opt.GetOption());
    }

    switch (option)
    {
        case 0:
        default:
            // no options
            break;

        case 'g':
            // case: get requires one non-option argument
        {
            if (!opt.CheckNumNonOptArgs(2, 2))
            {
                return SetError(opt.GetError().c_str());
            }

            return DoWMA(option, &(argv[3]));
        }

        case 'h':
            // case: history requires one non-option argument
        {
            if (!opt.CheckNumNonOptArgs(2, 2))
            {
                return SetError(opt.GetError().c_str());
            }

            return DoWMA(option, &(argv[3]));
        }

        case 's':
            // case: set requires two non-option arguments
        {
            if (!opt.CheckNumNonOptArgs(3, 3))
            {
                return SetError(opt.GetError().c_str());
            }

            return DoWMA(option, &(argv[3]), &(argv[4]));
        }

        case 'S':
            // case: stat can do zero or one non-option arguments
        {
            if (!opt.CheckNumNonOptArgs(1, 2))
            {
                return SetError(opt.GetError().c_str());
            }

            if (opt.GetNonOptionArguments() == 1)
            {
                return DoWMA('S');
            }

            return DoWMA(option, &(argv[3]));
        }

        case 't':
            // case: timer can do zero or one non-option arguments
        {
            if (!opt.CheckNumNonOptArgs(1, 2))
            {
                return SetError(opt.GetError().c_str());
            }

            if (opt.GetNonOptionArguments() == 1)
            {
                return DoWMA(option);
            }

            return DoWMA(option, &(argv[3]));
        }
    }

    // bad: no option, but more than one argument
    if (argv.size() > 2)
    {
        return SetError("Too many args.");
    }

    // case: nothing = full configuration information
    return DoWMA();

}
bool CommandLineInterface::DoAddWME(const std::string& id, std::string attribute, const std::string& value, bool acceptable)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    soar::Lexeme lexeme;

    // Get ID
    Symbol* pId = 0;
    if (!read_id_or_context_var_from_string(thisAgent, id.c_str(), &pId))
    {
        return SetError("Invalid identifier");
    }

    // skip optional '^', if present
    if (attribute[0] == '^')
    {
        attribute = attribute.substr(1);
    }

    // get attribute or '*'
    Symbol* pAttr = 0;
    if (attribute == "*")
    {
        pAttr = thisAgent->symbolManager->make_new_identifier('I', pId->id->level);
    }
    else
    {
        lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, attribute.c_str());

        switch (lexeme.type)
        {
            case STR_CONSTANT_LEXEME:
                pAttr = thisAgent->symbolManager->make_str_constant(lexeme.string());
                break;
            case INT_CONSTANT_LEXEME:
                pAttr = thisAgent->symbolManager->make_int_constant(lexeme.int_val);
                break;
            case FLOAT_CONSTANT_LEXEME:
                pAttr = thisAgent->symbolManager->make_float_constant(lexeme.float_val);
                break;
            case IDENTIFIER_LEXEME:
            case VARIABLE_LEXEME:
                pAttr = read_identifier_or_context_variable(thisAgent, &lexeme);
                if (!pAttr)
                {
                    return SetError("Invalid attribute.");
                }
                thisAgent->symbolManager->symbol_add_ref(pAttr);
                break;
            default:
                return SetError("Unknown attribute type.");
        }
    }

    // get value or '*'
    Symbol* pValue = 0;
    if (value == "*")
    {
        pValue = thisAgent->symbolManager->make_new_identifier('I', pId->id->level);
    }
    else
    {
        lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, value.c_str());
        switch (lexeme.type)
        {
            case STR_CONSTANT_LEXEME:
                pValue = thisAgent->symbolManager->make_str_constant(lexeme.string());
                break;
            case INT_CONSTANT_LEXEME:
                pValue = thisAgent->symbolManager->make_int_constant(lexeme.int_val);
                break;
            case FLOAT_CONSTANT_LEXEME:
                pValue = thisAgent->symbolManager->make_float_constant(lexeme.float_val);
                break;
            case IDENTIFIER_LEXEME:
            case VARIABLE_LEXEME:
                pValue = read_identifier_or_context_variable(thisAgent, &lexeme);
                if (!pValue)
                {
                    thisAgent->symbolManager->symbol_remove_ref(&pAttr);
                    return SetError("Invalid value.");
                }
                thisAgent->symbolManager->symbol_add_ref(pValue);
                break;
            default:
                thisAgent->symbolManager->symbol_remove_ref(&pAttr);
                return SetError("Unknown value type.");
        }
    }

    // now create and add the wme
    wme* pWme = make_wme(thisAgent, pId, pAttr, pValue, acceptable);

    thisAgent->symbolManager->symbol_remove_ref(&pWme->attr);
    thisAgent->symbolManager->symbol_remove_ref(&pWme->value);
    insert_at_head_of_dll(pWme->id->id->input_wmes, pWme, next, prev);

    if (wma_enabled(thisAgent))
    {
        wma_activate_wme(thisAgent, pWme);
    }

    add_wme_to_wm(thisAgent, pWme);

    /* There was very old code that stopped do_buffered_wm_and_ownership_changes
     * from being called if NO_TOP_LEVEL_REFS was true, which
     * is no longer the default setting.   If this call causes problems,
     * might want to try using DO_TOP_LEVEL_REF_CTS similarly.*/

    do_buffered_wm_and_ownership_changes(thisAgent);

    if (m_RawOutput)
    {
        m_Result << "Timetag: " << pWme->timetag;
    }
    else
    {
        std::stringstream timetagString;
        timetagString << pWme->timetag;
        AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, timetagString.str());
    }
    return true;
}

bool CommandLineInterface::DoRemoveWME(uint64_t timetag)
{
    wme* pWme = 0;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    for (pWme = thisAgent->all_wmes_in_rete; pWme != 0; pWme = pWme->rete_next)
    {
        if (pWme->timetag == timetag)
        {
            break;
        }
    }

    if (pWme)
    {
        Symbol* pId = pWme->id;

        // remove w from whatever list of wmes it's on
        for (wme* pWme2 = pId->id->input_wmes; pWme2 != 0; pWme2 = pWme2->next)
        {
            if (pWme == pWme2)
            {
                remove_from_dll(pId->id->input_wmes, pWme, next, prev);
                break;
            }
        }

        for (wme* pWme2 = pId->id->impasse_wmes; pWme2 != 0; pWme2 = pWme2->next)
        {
            if (pWme == pWme2)
            {
                remove_from_dll(pId->id->impasse_wmes, pWme, next, prev);
                break;
            }
        }

        for (slot* s = pId->id->slots; s != 0; s = s->next)
        {

            for (wme* pWme2 = s->wmes; pWme2 != 0; pWme2 = pWme2->next)
            {
                if (pWme == pWme2)
                {
                    remove_from_dll(s->wmes, pWme, next, prev);
                    break;
                }
            }

            for (wme* pWme2 = s->acceptable_preference_wmes; pWme2 != NIL; pWme2 = pWme2->next)
            {
                if (pWme == pWme2)
                {
                    remove_from_dll(s->acceptable_preference_wmes, pWme, next, prev);
                    break;
                }
            }
        }

        /* REW: begin 09.15.96 */
        if (pWme->gds)
        {
            if (pWme->gds->goal != 0)
            {
                gds_invalid_so_remove_goal(thisAgent, pWme);
                /* NOTE: the call to remove_wme_from_wm will take care of checking if
                GDS should be removed */
            }
        }
        /* REW: end   09.15.96 */

        // now remove w from working memory
        remove_wme_from_wm(thisAgent, pWme);

        /* This was previously using #ifndef NO_TOP_LEVEL_REFS, which is a macro constant that
         * no longer exists.  We now use DO_TOP_LEVEL_COND_REF_CTS.  Top level refcounting is now
         * also disabled by default so changing it to #ifdef DO_TOP_LEVEL_COND_REF_CTS would
         * change the current behavior.  Other uses of DO_TOP_LEVEL_COND_REF_CTS seem to only be used
         * when adding refcounts to top-state wme's, so I'm not sure why the old macro prevented
         * this entire call.  So, I'm just going to comment it out for now and preserve existing
         * behavior. */
        //#ifdef DO_TOP_LEVEL_COND_REF_CTS
        do_buffered_wm_and_ownership_changes(thisAgent);
        //#endif
    }

    return true;
}
bool CommandLineInterface::DoWMA(const char pOp, const std::string* pAttr, const std::string* pVal)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (!pOp)
    {
        std::string temp;
        char* temp2;

        if (m_RawOutput)
        {
            m_Result << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, "");
        }

        temp = "WMA activation: ";
        temp2 = thisAgent->WM->wma_params->activation->get_string();
        temp += temp2;
        delete temp2;
        if (m_RawOutput)
        {
            m_Result << temp << "\n\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, "");
        }

        temp = "Activation";
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }
        temp = "----------";
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }

        temp = "decay-rate: ";
        temp2 = thisAgent->WM->wma_params->decay_rate->get_string();
        temp += temp2;
        delete temp2;
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }

        temp = "petrov-approx: ";
        temp2 = thisAgent->WM->wma_params->petrov_approx->get_string();
        temp += temp2;
        delete temp2;
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }

        if (m_RawOutput)
        {
            m_Result << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, "");
        }


        temp = "Forgetting";
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }
        temp = "----------";
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }

        temp = "decay-thresh: ";
        temp2 = thisAgent->WM->wma_params->decay_thresh->get_string();
        temp += temp2;
        delete temp2;
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }

        temp = "forgetting: ";
        temp2 = thisAgent->WM->wma_params->forgetting->get_string();
        temp += temp2;
        delete temp2;
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }

        temp = "forget-wme: ";
        temp2 = thisAgent->WM->wma_params->forget_wme->get_string();
        temp += temp2;
        delete temp2;
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }

        temp = "fake-forgetting: ";
        temp2 = thisAgent->WM->wma_params->fake_forgetting->get_string();
        temp += temp2;
        delete temp2;
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }

        if (m_RawOutput)
        {
            m_Result << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, "");
        }

        temp = "Performance";
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }
        temp = "-----------";
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }

        temp = "timers: ";
        temp2 = thisAgent->WM->wma_params->timers->get_string();
        temp += temp2;
        delete temp2;
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }

        temp = "max-pow-cache: ";
        temp2 = thisAgent->WM->wma_params->max_pow_cache->get_string();
        temp += temp2;
        delete temp2;
        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str());
        }

        //

        if (m_RawOutput)
        {
            m_Result << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, "");
        }

        return true;
    }
    else if (pOp == 'g')
    {
        soar_module::param* my_param = thisAgent->WM->wma_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid activation setting.");
        }

        char* temp2 = my_param->get_string();
        std::string output(temp2);
        delete temp2;

        if (m_RawOutput)
        {
            m_Result << output;
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, output.c_str());
        }

        return true;
    }
    else if (pOp == 'h')
    {
        uint64_t timetag;
        if (!from_string(timetag, *pAttr) || (timetag == 0))
        {
            return SetError("Invalid timetag.");
        }

        wme* pWme = NULL;
        agent* thisAgent = m_pAgentSML->GetSoarAgent();

        for (pWme = thisAgent->all_wmes_in_rete; pWme; pWme = pWme->rete_next)
        {
            if (pWme->timetag == timetag)
            {
                break;
            }
        }

        if (pWme)
        {
            std::string output;
            wma_get_wme_history(thisAgent, pWme, output);

            if (m_RawOutput)
            {
                m_Result << output;
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, output.c_str());
            }
        }

        return true;
    }
    else if (pOp == 's')
    {
        soar_module::param* my_param = thisAgent->WM->wma_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid activation setting.");
        }

        if (!my_param->validate_string(pVal->c_str()))
        {
            return SetError("Invalid value for activation setting.");
        }

        bool result = my_param->set_string(pVal->c_str());

        // since parameter name and value have been validated,
        // this can only mean the parameter is protected
        if (!result)
        {
            SetError("ERROR: this parameter is protected while WMA is on.");
        }

        return result;
    }
    else if (pOp == 'S')
    {
        if (!pAttr)
        {
            std::string output;
            char* temp2;

            output = "Forgotten WMEs: ";
            temp2 = thisAgent->WM->wma_stats->forgotten_wmes->get_string();
            output += temp2;
            delete temp2;

            if (m_RawOutput)
            {
                m_Result << output << "\n";
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, output.c_str());
            }
        }
        else
        {
            soar_module::statistic* my_stat = thisAgent->WM->wma_stats->get(pAttr->c_str());
            if (!my_stat)
            {
                return SetError("Invalid statistic.");
            }

            char* temp2 = my_stat->get_string();
            std::string output(temp2);
            delete temp2;

            if (m_RawOutput)
            {
                m_Result << output;
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, output.c_str());
            }
        }

        return true;
    }
    else if (pOp == 't')
    {
        if (!pAttr)
        {
            struct foo: public soar_module::accumulator< soar_module::timer* >
            {
                private:
                    bool raw;
                    cli::CommandLineInterface* this_cli;
                    std::ostringstream& m_Result;

                    foo& operator= (const foo&)
                    {
                        return *this;
                    }

                public:
                    foo(bool m_RawOutput, cli::CommandLineInterface* new_cli, std::ostringstream& m_Result): raw(m_RawOutput), this_cli(new_cli), m_Result(m_Result) {};


                    void operator()(soar_module::timer* t)
                    {
                        std::string output(t->get_name());
                        output += ": ";

                        char* temp = t->get_string();
                        output += temp;
                        delete temp;

                        if (raw)
                        {
                            m_Result << output << "\n";
                        }
                        else
                        {
                            this_cli->AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, output.c_str());
                        }
                    }
            } bar(m_RawOutput, this, m_Result);

            thisAgent->WM->wma_timers->for_each(bar);
        }
        else
        {
            // check attribute name
            soar_module::timer* my_timer = thisAgent->WM->wma_timers->get(pAttr->c_str());
            if (!my_timer)
            {
                return SetError("Invalid timer.");
            }

            char* temp2 = my_timer->get_string();
            std::string output(temp2);
            delete temp2;

            if (m_RawOutput)
            {
                m_Result << output;
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, output.c_str());
            }
        }

        return true;
    }

    return SetError("Unknown option.");
}
int RemoveWme(agent* thisAgent, wme* pWme)
{
    //    wme *w, *w2;
    //    Symbol *id;
    //    slot *s;

    //    w = (wme *) the_wme;

    Symbol* pId = pWme->id;

    // remove w from whatever list of wmes it's on
    wme* pWme2;
    for (pWme2 = pId->id->input_wmes; pWme2 != NIL; pWme2 = pWme2->next)
        if (pWme == pWme2)
        {
            break;
        }

    if (pWme2)
    {
        remove_from_dll(pId->id->input_wmes, pWme, next, prev);
    }

    for (pWme2 = pId->id->impasse_wmes; pWme2 != NIL; pWme2 = pWme2->next)
        if (pWme == pWme2)
        {
            break;
        }

    if (pWme2)
    {
        remove_from_dll(pId->id->impasse_wmes, pWme, next, prev);
    }

    slot* s;
    for (s = pId->id->slots; s != NIL; s = s->next)
    {

        for (pWme2 = s->wmes; pWme2 != NIL; pWme2 = pWme2->next)
            if (pWme == pWme2)
            {
                break;
            }

        if (pWme2)
        {
            remove_from_dll(s->wmes, pWme, next, prev);
        }

        for (pWme2 = s->acceptable_preference_wmes; pWme2 != NIL; pWme2 = pWme2->next)
            if (pWme == pWme2)
            {
                break;
            }

        if (pWme2)
        {
            remove_from_dll(s->acceptable_preference_wmes, pWme, next, prev);
        }
    }

#ifdef USE_CAPTURE_REPLAY
    // TODO: ommitted
#endif // USE_CAPTURE_REPLAY

    /* REW: begin 09.15.96 */
    if (pWme->gds)
    {
        if (pWme->gds->goal != NIL)
        {
            gds_invalid_so_remove_goal(thisAgent, pWme);
            /* NOTE: the call to remove_wme_from_wm will take care of checking if
            GDS should be removed */
        }
    }
    /* REW: end   09.15.96 */

    // now remove w from working memory
    remove_wme_from_wm(thisAgent, pWme);

    /* REW: begin 28.07.96 */
    /* See AddWme for description of what's going on here */

    if (thisAgent->current_phase != INPUT_PHASE)
    {
#ifndef NO_TIMING_STUFF
        thisAgent->timers_kernel.start();
#ifndef KERNEL_TIME_ONLY
        thisAgent->timers_phase.start();
#endif // KERNEL_TIME_ONLY
#endif // NO_TIMING_STUFF

        /* do_buffered_wm_and_ownership_changes(); */

#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        thisAgent->timers_phase.stop();
        thisAgent->timers_decision_cycle_phase[thisAgent->current_phase].update(thisAgent->timers_phase);
#endif // KERNEL_TIME_ONLY
        thisAgent->timers_kernel.stop();
        thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
        thisAgent->timers_kernel.start();
#endif // NO_TIMING_STUFF
    }

/* This was previously using #ifndef NO_TOP_LEVEL_REFS, which is a macro constant that
 * no longer exists.  We now use DO_TOP_LEVEL_COND_REF_CTS.  Top level refcounting is now
 * also disabled by default so changing it to #ifdef DO_TOP_LEVEL_COND_REF_CTS would
 * change the current behavior.  Other uses of DO_TOP_LEVEL_COND_REF_CTS seem to only be used
 * when adding refcounts to top-state wme's, so I'm not sure why the old macro prevented
 * this entire call.  So, I'm just going to comment it out for now and preserve existing
 * behavior. */
//#ifdef DO_TOP_LEVEL_COND_REF_CTS
    do_buffered_wm_and_ownership_changes(thisAgent);
//#endif // DO_TOP_LEVEL_COND_REF_CTS

    return 0;
}

bool read_wme_filter_component(agent* thisAgent, const char* s, Symbol** sym)
{
    soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, const_cast<char*>(s));
    if (lexeme.type == IDENTIFIER_LEXEME)
    {
        if ((*sym = thisAgent->symbolManager->find_identifier(lexeme.id_letter, lexeme.id_number)) == NIL)
        {
            return false;          /* Identifier does not exist */
        }
    }
    else
    {
        *sym = make_symbol_for_lexeme(thisAgent, &lexeme, false);
    }
    // Added by voigtjr because if this function can
    // legally return success with *sym == 0, my logic in AddWmeFilter will be broken.
    assert(*sym);
    return true;
}

int AddWMEFilter(agent* thisAgent, const char* pIdString, const char* pAttrString, const char* pValueString, bool adds, bool removes)
{
    Symbol* pId = 0;
    if (!read_wme_filter_component(thisAgent, pIdString, &pId))
    {
        return -1;
    }

    Symbol* pAttr = 0;
    if (!read_wme_filter_component(thisAgent, pAttrString, &pAttr))
    {
        thisAgent->symbolManager->symbol_remove_ref(&pId);
        return -2;
    }

    Symbol* pValue = 0;
    if (!read_wme_filter_component(thisAgent, pValueString, &pValue))
    {
        thisAgent->symbolManager->symbol_remove_ref(&pId);
        thisAgent->symbolManager->symbol_remove_ref(&pAttr);
        return -3;
    }

    /* check to see if such a filter has already been added: */
    cons* c;
    wme_filter* existing_wf;
    for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest)
    {

        existing_wf = static_cast<wme_filter*>(c->first);

        // check for duplicate
        if ((existing_wf->adds == adds)
                && (existing_wf->removes == removes)
                && (existing_wf->id == pId)
                && (existing_wf->attr == pAttr)
                && (existing_wf->value == pValue))
        {
            thisAgent->symbolManager->symbol_remove_ref(&pId);
            thisAgent->symbolManager->symbol_remove_ref(&pAttr);
            thisAgent->symbolManager->symbol_remove_ref(&pValue);
            return -4; // Filter already exists
        }
    }

    wme_filter* wf = static_cast<wme_filter*>(thisAgent->memoryManager->allocate_memory(sizeof(wme_filter), MISCELLANEOUS_MEM_USAGE));
    wf->id = pId;
    wf->attr = pAttr;
    wf->value = pValue;
    wf->adds = adds;
    wf->removes = removes;

    /* Rather than add refs for the new filter symbols and then remove refs
    * for the identical symbols created from the string parameters, skip
    * the two nullifying steps altogether and just return immediately
    * after pushing the new filter:
    */
    push(thisAgent, wf, thisAgent->wme_filter_list);
    return 0;
}

int RemoveWMEFilter(agent* thisAgent, const char* pIdString, const char* pAttrString, const char* pValueString, bool adds, bool removes)
{
    Symbol* pId = 0;
    if (!read_wme_filter_component(thisAgent, pIdString, &pId))
    {
        return -1;
    }

    Symbol* pAttr = 0;
    if (!read_wme_filter_component(thisAgent, pAttrString, &pAttr))
    {
        thisAgent->symbolManager->symbol_remove_ref(&pId);
        return -2;
    }

    Symbol* pValue = 0;
    if (!read_wme_filter_component(thisAgent, pValueString, &pValue))
    {
        thisAgent->symbolManager->symbol_remove_ref(&pId);
        thisAgent->symbolManager->symbol_remove_ref(&pAttr);
        return -3;
    }

    cons* c;
    cons** prev_cons_rest = &thisAgent->wme_filter_list;
    for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest)
    {
        wme_filter* wf = static_cast<wme_filter*>(c->first);

        // check for duplicate
        if ((wf->adds == adds)
                && (wf->removes == removes)
                && (wf->id == pId)
                && (wf->attr == pAttr)
                && (wf->value == pValue))
        {
            *prev_cons_rest = c->rest;
            thisAgent->symbolManager->symbol_remove_ref(&pId);
            thisAgent->symbolManager->symbol_remove_ref(&pAttr);
            thisAgent->symbolManager->symbol_remove_ref(&pValue);
            thisAgent->memoryManager->free_memory(wf, MISCELLANEOUS_MEM_USAGE);
            free_cons(thisAgent, c);
            return 0; /* assume that AddWMEFilter did not add duplicates */
        }
        prev_cons_rest = &(c->rest);
    }
    assert(!c);
    thisAgent->symbolManager->symbol_remove_ref(&pId);
    thisAgent->symbolManager->symbol_remove_ref(&pAttr);
    thisAgent->symbolManager->symbol_remove_ref(&pValue);
    return -4;
}

bool ResetWMEFilters(agent* thisAgent, bool adds, bool removes)
{
    cons* c, *next_cons;
    bool didRemoveSome = false;
    cons** prev_cons_rest = &thisAgent->wme_filter_list;
    for (c = thisAgent->wme_filter_list; c != NIL; c = next_cons)
    {
        next_cons = c->rest;
        wme_filter* wf = static_cast<wme_filter*>(c->first);
        if ((adds && wf->adds) || (removes && wf->removes))
        {
            *prev_cons_rest = c->rest;
            thisAgent->outputManager->printa_sf(thisAgent, "Removed: (%y ^%y %y) ", wf->id, wf->attr, wf->value);
            thisAgent->outputManager->printa_sf(thisAgent,  "%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
            thisAgent->symbolManager->symbol_remove_ref(&wf->id);
            thisAgent->symbolManager->symbol_remove_ref(&wf->attr);
            thisAgent->symbolManager->symbol_remove_ref(&wf->value);
            thisAgent->memoryManager->free_memory(wf, MISCELLANEOUS_MEM_USAGE);
            free_cons(thisAgent, c);
            didRemoveSome = true;
        }
        prev_cons_rest = &(next_cons);
    }
    return didRemoveSome;
}

void ListWMEFilters(agent* thisAgent, bool adds, bool removes)
{
    cons* c;
    for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest)
    {
        wme_filter* wf = static_cast<wme_filter*>(c->first);

        if ((adds && wf->adds) || (removes && wf->removes))
        {
            thisAgent->outputManager->printa_sf(thisAgent, "wme filter: (%y ^%y %y) ", wf->id, wf->attr, wf->value);
            thisAgent->outputManager->printa_sf(thisAgent,  "%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
        }
    }
}

bool CommandLineInterface::DoWatchWMEs(const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString, const std::string* pAttributeString, const std::string* pValueString)
{
    int ret = 0;
    bool retb = false;
    switch (mode)
    {
        case WATCH_WMES_ADD:
            if (!pIdString || !pAttributeString || !pValueString)
            {
                return SetError("ID/Attribute/Value filter expected, one or more missing.");
            }
            ret = AddWMEFilter(m_pAgentSML->GetSoarAgent(), pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
            if (ret == -1)
            {
                return SetError("Invalid id, got: " + *pIdString);
            }
            if (ret == -2)
            {
                return SetError("Invalid attribute, got: " + *pAttributeString);
            }
            if (ret == -3)
            {
                return SetError("Invalid value, got: " + *pValueString);
            }
            if (ret == -4)
            {
                return SetError("That WME filter already exists.");
            }
            break;

        case WATCH_WMES_REMOVE:
            if (!pIdString || !pAttributeString || !pValueString)
            {
                return SetError("ID/Attribute/Value filter expected, one or more missing.");
            }
            ret = RemoveWMEFilter(m_pAgentSML->GetSoarAgent(), pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
            if (ret == -1)
            {
                return SetError("Invalid id, got: " + *pIdString);
            }
            if (ret == -2)
            {
                return SetError("Invalid attribute, got: " + *pAttributeString);
            }
            if (ret == -3)
            {
                return SetError("Invalid value, got: " + *pValueString);
            }
            if (ret == -4)
            {
                return SetError("The specified WME filter was not found.");
            }
            break;

        case WATCH_WMES_LIST:
            if (type.none())
            {
                type.flip();
            }

            ListWMEFilters(m_pAgentSML->GetSoarAgent(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
            break;

        case WATCH_WMES_RESET:
            if (type.none())
            {
                type.flip();
            }

            retb = ResetWMEFilters(m_pAgentSML->GetSoarAgent(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));

            if (!retb)
            {
                return SetError("The specified WME filter was not found.");
            }
            break;

        default:
            return SetError("Invalid mode.");
    }

    return true;
}
