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
#include "cmd_settings.h"
#include "decider.h"
#include "decision_manipulation.h"
#include "exploration.h"
#include "misc.h"
#include "output_manager.h"
#include "print.h"
#include "production.h"
#include "soar_rand.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "rete.h"

#include <algorithm>
#include <vector>

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoDecide(std::vector<std::string>& argv, const std::string& pCmd)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (pCmd.empty())
    {
        thisAgent->command_params->decide_params->print_summary(thisAgent);
        return true;
    }
    soar_module::param* my_param = thisAgent->command_params->decide_params->get(pCmd.c_str());
    if (!my_param)
    {
            return SetError("Invalid decide command.  Use 'decide ?' to see a list of valid settings.");
    }
    if (my_param == thisAgent->command_params->decide_params->indifferent_selection_cmd)
    {
        return ParseIndifferentSelection(argv);
    }
    else if (my_param == thisAgent->command_params->decide_params->numeric_indifferent_mode_cmd)
    {
        return ParseNumericIndifferentMode(argv);
    }
    else if (my_param == thisAgent->command_params->decide_params->predict_cmd)
    {
        return ParsePredict(argv);
    }
    else if (my_param == thisAgent->command_params->decide_params->select_cmd)
    {
        return ParseSelect(argv);
    }
    else if ((my_param == thisAgent->command_params->decide_params->srand_cmd) || (my_param == thisAgent->command_params->decide_params->srand_bc_cmd))
    {
        return ParseSRand(argv);
    }
    else if ((my_param == thisAgent->command_params->decide_params->help_cmd) || (my_param == thisAgent->command_params->decide_params->qhelp_cmd))
    {
        thisAgent->command_params->decide_params->print_settings(thisAgent);
    }
    return false;
}
bool CommandLineInterface::ParseIndifferentSelection(std::vector< std::string >& argv)
{

    cli::Options opt;
    OptionsData optionsData[] =
    {
        // selection policies
        {'b', "boltzmann",            OPTARG_NONE},
        {'g', "epsilon-greedy",        OPTARG_NONE},
        {'f', "first",                OPTARG_NONE},
        {'l', "last",                OPTARG_NONE},
        //{'u', "random-uniform",        OPTARG_NONE},
        {'x', "softmax",            OPTARG_NONE},

        // selection parameters
        {'e', "epsilon",            OPTARG_NONE},
        {'t', "temperature",        OPTARG_NONE},

        // auto-reduction control
        {'a', "auto-reduce",        OPTARG_NONE},

        // selection parameter reduction
        {'p', "reduction-policy",    OPTARG_NONE},
        {'r', "reduction-rate",        OPTARG_NONE},

        // stats
        {'s', "stats",                OPTARG_NONE},

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
            return SetError("indifferent-selection takes only one option at a time.");
        }
        option = static_cast<char>(opt.GetOption());
    }

    switch (option)
    {
        case 0:
        default:
            // no options
            break;

        case 'b':
        case 'g':
        case 'f':
        case 'l':
        case 'x':
            // case: exploration policy takes no non-option arguments
            if (!opt.CheckNumNonOptArgs(1, 1))
            {
                return SetError(opt.GetError().c_str());
            }
            return DoIndifferentSelection(option);

        case 'e':
        case 't':
            // case: selection parameter can do zero or one non-option arguments
            if (!opt.CheckNumNonOptArgs(1, 2))
            {
                return SetError(opt.GetError().c_str());
            }

            if (opt.GetNonOptionArguments() == 1)
            {
                return DoIndifferentSelection(option);
            }

            return DoIndifferentSelection(option, &(argv[3]));

        case 'a':
            // case: auto reduction control can do zero or one non-option arguments
            if (!opt.CheckNumNonOptArgs(1, 2))
            {
                return SetError(opt.GetError().c_str());
            }

            if (opt.GetNonOptionArguments() == 1)
            {
                return DoIndifferentSelection(option);
            }

            return DoIndifferentSelection(option, &(argv[3]));

        case 'p':
            // case: reduction policy requires one or two non-option arguments
            if (!opt.CheckNumNonOptArgs(2, 3))
            {
                return SetError(opt.GetError().c_str());
            }

            if (opt.GetNonOptionArguments() == 2)
            {
                return DoIndifferentSelection(option, &(argv[3]));
            }

            return DoIndifferentSelection(option, &(argv[3]), &(argv[4]));

        case 'r':
            // case: reduction policy rate requires two or three arguments
            if (!opt.CheckNumNonOptArgs(3, 4))
            {
                return SetError(opt.GetError().c_str());
            }

            if (opt.GetNonOptionArguments() == 3)
            {
                return DoIndifferentSelection(option, &(argv[3]), &(argv[4]));
            }

            return DoIndifferentSelection(option, &(argv[3]), &(argv[4]), &(argv[5]));

        case 's':
            // case: stats takes no parameters
            if (!opt.CheckNumNonOptArgs(1, 1))
            {
                return SetError(opt.GetError().c_str());
            }

            return DoIndifferentSelection(option);
    }

    // bad: no option, but more than one argument
    if (argv.size() > 2)
    {
        return SetError("Too many args.");
    }

    // case: nothing = full configuration information
    return DoIndifferentSelection();

}
bool CommandLineInterface::ParseNumericIndifferentMode(std::vector< std::string >& argv)
{
    cli::Options opt;
    OptionsData optionsData[] =
    {
        {'a', "average",    OPTARG_NONE},
        {'a', "avg",        OPTARG_NONE},
        {'s', "sum",        OPTARG_NONE},
        {0, 0, OPTARG_NONE}
    };

    bool usesAvgNIM = true;
    bool query = true;

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
                usesAvgNIM = true;
                query = false;
                break;
            case 's':
                usesAvgNIM = false;
                query = false;
                break;
        }
    }

    // No additional arguments
    if (!opt.CheckNumNonOptArgs(1, 1))
    {
        return SetError(opt.GetError().c_str());
    }

    return DoNumericIndifferentMode(query, usesAvgNIM);
}
bool CommandLineInterface::ParsePredict(std::vector< std::string >& argv)
{
    // No arguments to predict next operator
    if (argv.size() != 2)
    {
        return SetError("predict takes no arguments.");
    }

    return DoPredict();
}
bool CommandLineInterface::ParseSelect(std::vector< std::string >& argv)
{
    // At most one argument to select the next operator
    if (argv.size() > 3)
    {
        return SetError("Syntax: decide select <id>");
    }

    if (argv.size() == 3)
    {
        return DoSelect(&(argv[2]));
    }

    return DoSelect();
}
bool CommandLineInterface::ParseSRand(std::vector< std::string >& argv)
{
    if (argv.size() < 3)
    {
        return DoSRand();
    }

    if (argv.size() > 3)
    {
        return SetError("Syntax: decide set-random-seed [seed]");
    }

    uint32_t seed = 0;
    sscanf(argv[2].c_str(), "%u", &seed);
    return DoSRand(&seed);
}
bool CommandLineInterface::DoIndifferentSelection(const char pOp, const std::string* p1, const std::string* p2, const std::string* p3)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    // show selection policy
    if (!pOp)
    {
        const char* policy_name = exploration_convert_policy(exploration_get_policy(thisAgent));

        if (m_RawOutput)
        {
            m_Result << policy_name;
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeString, policy_name);
        }

        return true;
    }

    // selection policy
    else if (pOp == 'b')
    {
        return exploration_set_policy(thisAgent, "boltzmann");
    }
    else if (pOp == 'g')
    {
        return exploration_set_policy(thisAgent, "epsilon-greedy");
    }
    else if (pOp == 'f')
    {
        return exploration_set_policy(thisAgent, "first");
    }
    else if (pOp == 'l')
    {
        return exploration_set_policy(thisAgent, "last");
    }
    /*else if ( pOp == 'u' )
    return exploration_set_policy( thisAgent, "random-uniform" );*/
    else if (pOp == 'x')
    {
        return exploration_set_policy(thisAgent, "softmax");
    }

    // auto-update control
    else if (pOp == 'a')
    {
        if (!p1)
        {
            bool setting = thisAgent->Decider->settings[DECIDER_AUTO_REDUCE];

            if (m_RawOutput)
            {
                m_Result << ((setting) ? ("on") : ("off"));
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, ((setting) ? ("on") : ("off")));
            }

            return true;
        }
        else
        {
            if (*p1 != "on" && *p1 != "off")
            {
                return SetError("Invalid parameter value.");
            }

            thisAgent->Decider->settings[DECIDER_AUTO_REDUCE] = (*p1 == "on");
            return true;
        }
    }

    // selection policy parameter
    else if (pOp == 'e')
    {
        if (!p1)
        {
            double param_value = exploration_get_parameter_value(thisAgent, "epsilon");
            std::string temp;
            to_string(param_value, temp);

            if (m_RawOutput)
            {
                m_Result << temp;
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeDouble, temp);
            }

            return true;
        }
        else
        {
            double new_val;
            if (!from_string(new_val, *p1))
            {
                return SetError("Invalid parameter value.");
            }

            if (!exploration_valid_parameter_value(thisAgent, "epsilon", new_val))
            {
                return SetError("Invalid parameter value.");
            }

            return exploration_set_parameter_value(thisAgent, "epsilon", new_val);
        }
    }
    else if (pOp == 't')
    {
        if (!p1)
        {
            double param_value = exploration_get_parameter_value(thisAgent, "temperature");
            std::string temp;
            to_string(param_value, temp);

            if (m_RawOutput)
            {
                m_Result << temp;
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeDouble, temp);
            }

            return true;
        }
        else
        {
            double new_val;
            if (!from_string(new_val, *p1))
            {
                return SetError("Invalid parameter value.");
            }

            if (!exploration_valid_parameter_value(thisAgent, "temperature", new_val))
            {
                return SetError("Invalid parameter value.");
            }

            return exploration_set_parameter_value(thisAgent, "temperature", new_val);
        }
    }

    // selection parameter reduction policy
    else if (pOp == 'p')
    {
        if (!p2)
        {
            if (!exploration_valid_parameter(thisAgent, p1->c_str()))
            {
                return SetError("Invalid parameter value.");
            }

            const char* policy_name = exploration_convert_reduction_policy(exploration_get_reduction_policy(thisAgent, p1->c_str()));

            if (m_RawOutput)
            {
                m_Result << policy_name;
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, policy_name);
            }

            return true;
        }
        else
        {
            if (!exploration_valid_reduction_policy(thisAgent, p1->c_str(), p2->c_str()))
            {
                return SetError("Invalid parameter value.");
            }

            return exploration_set_reduction_policy(thisAgent, p1->c_str(), p2->c_str());
        }
    }

    // selection parameter reduction rate
    else if (pOp == 'r')
    {
        if (!exploration_valid_parameter(thisAgent, p1->c_str()))
        {
            return SetError("Invalid exploration parameter.");
        }

        if (!exploration_valid_reduction_policy(thisAgent, p1->c_str(), p2->c_str()))
        {
            return SetError("Invalid exploration reduction policy.");
        }

        if (!p3)
        {
            double reduction_rate = exploration_get_reduction_rate(thisAgent, p1->c_str(), p2->c_str());
            std::string temp;
            to_string(reduction_rate, temp);

            if (m_RawOutput)
            {
                m_Result << temp;
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeDouble, temp);
            }

            return true;
        }
        else
        {
            double new_val;
            if (!from_string(new_val, *p3))
            {
                return SetError("Invalid parameter value.");
            }

            if (!exploration_valid_reduction_rate(thisAgent, p1->c_str(), p2->c_str(), new_val))
            {
                return SetError("Invalid parameter value.");
            }

            return exploration_set_reduction_rate(thisAgent, p1->c_str(), p2->c_str(), new_val);
        }
    }

    // stats
    else if (pOp == 's')
    {
        // used for output
        std::string temp, temp2, temp3;
        std::string temp4;
        double temp_value;

        temp = "Exploration Policy: ";
        temp += exploration_convert_policy(exploration_get_policy(thisAgent));

        if (m_RawOutput)
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp);
        }
        temp = "";

        temp = "Automatic Policy Parameter Reduction: ";
        temp += (thisAgent->Decider->settings[DECIDER_AUTO_REDUCE] ? ("on") : ("off"));

        if (m_RawOutput)
        {
            m_Result << temp << "\n\n";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp);
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, "");
        }
        temp = "";

        for (int i = 0; i < EXPLORATION_PARAMS; i++)
        {
            // value
            temp = exploration_convert_parameter(thisAgent, i);
            temp += ": ";
            temp_value = exploration_get_parameter_value(thisAgent, i);
            to_string(temp_value, temp4);
            temp += temp4;

            if (m_RawOutput)
            {
                m_Result << temp << "\n";
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp);
            }

            // reduction policy
            temp = exploration_convert_parameter(thisAgent, i);
            temp += " Reduction Policy: ";
            temp += exploration_convert_reduction_policy(exploration_get_reduction_policy(thisAgent, i));
            if (m_RawOutput)
            {
                m_Result << temp << "\n";
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp);
            }

            // rates
            temp2 = "";
            temp3 = "";
            for (int j = 0; j < EXPLORATION_REDUCTIONS; j++)
            {
                temp2 += exploration_convert_reduction_policy(j);
                if (j != (EXPLORATION_REDUCTIONS - 1))
                {
                    temp2 += "/";
                }

                temp_value = exploration_get_reduction_rate(thisAgent, i, j);
                to_string(temp_value, temp4);
                temp3 += temp4;
                if (j != (EXPLORATION_REDUCTIONS - 1))
                {
                    temp3 += "/";
                }
            }
            temp = exploration_convert_parameter(thisAgent, i);
            temp += " Reduction Rate (";
            temp += temp2;
            temp += "): ";
            temp += temp3;
            if (m_RawOutput)
            {
                m_Result << temp << "\n\n";
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, temp);
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, "");
            }

            temp = "";
        }

        return true;
    }

    return SetError("Unknown option.");
}

bool CommandLineInterface::DoNumericIndifferentMode(bool query, bool usesAvgNIM)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (query)
    {
        if (m_RawOutput)
        {
            m_Result << "Current numeric indifferent mode: ";

            switch (thisAgent->numeric_indifferent_mode)
            {
                default:
                case NUMERIC_INDIFFERENT_MODE_AVG:
                    m_Result << "Soar will average numeric preferences.";
                    break;
                case NUMERIC_INDIFFERENT_MODE_SUM:
                    m_Result << "Soar will use the sum of numeric preferences.";
                    break;
            }
        }
        else
        {
            std::stringstream modeString;
            modeString << static_cast< int >(thisAgent->numeric_indifferent_mode);
            AppendArgTagFast(sml_Names::kParamNumericIndifferentMode, sml_Names::kTypeInt, modeString.str());
        }
    }
    else // !query
    {
        thisAgent->numeric_indifferent_mode = usesAvgNIM ? NUMERIC_INDIFFERENT_MODE_AVG : NUMERIC_INDIFFERENT_MODE_SUM;
    }

    return true;
}

bool CommandLineInterface::DoPredict()
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    const char* prediction_result = predict_get(thisAgent);

    if (m_RawOutput)
    {
        m_Result << prediction_result;
    }
    else
    {
        AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, prediction_result);
    }

    return true;
}

bool CommandLineInterface::DoSelect(const std::string* pOp)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (!pOp)
    {
        const char* my_selection = select_get_operator(thisAgent);

        if (my_selection != NULL)
        {
            if (m_RawOutput)
            {
                m_Result << my_selection;
            }
            else
            {
                AppendArgTagFast(sml_Names::kOperator_ID, sml_Names::kTypeID, my_selection);
            }
        }
        else
        {
            if (m_RawOutput)
            {
                m_Result << "No operator selected.";
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, "No operator selected.");
            }
        }
    }
    else
    {
        select_next_operator(thisAgent, pOp->c_str());
        m_Result << "Operator " << (*pOp) << " will be selected.";
    }

    return true;
}

bool CommandLineInterface::DoSRand(uint32_t* pSeed)
{
    std::ostringstream lFeedback;
    if (pSeed)
    {
        SoarSeedRNG(*pSeed);
        lFeedback << "Random number generator seed set to " << (*pSeed);

    }
    else
    {
        SoarSeedRNG();
        lFeedback << "Random number generator seed set to new random value.";
    }

    if (m_RawOutput)
    {
        m_Result << lFeedback.str().c_str() << "\n";
    }
    else
    {
        AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, lFeedback.str().c_str());
    }

    return true;
}

