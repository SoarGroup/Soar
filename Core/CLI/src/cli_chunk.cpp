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
#include "ebc.h"
#include "ebc_timers.h"
#include "explanation_memory.h"

#include "print.h"
#include "ebc_settings.h"
#include "output_manager.h"

#include <cstdio>

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoChunk(const std::string* pArg1, const std::string* pArg2, const std::string* pArg3, const std::string* pArg4, bool doRemove, bool doClear)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempStringStream;
    std::string tempString;

    if (!pArg1)
    {
        thisAgent->explanationBasedChunker->print_chunking_summary();
        return true;
    }
    /* Single command argument */
    soar_module::param* my_param = thisAgent->explanationBasedChunker->ebc_params->get(pArg1->c_str());
    if (!my_param)
    {
        /* Command was not a valid ebc_param name, so it must be a single word command */
        /* Check if it's one of the four chunk enable commands.  (Means no ebc_param name
         * can be named enabled, on off, all disabled, none only all-except*/
        if(thisAgent->explanationBasedChunker->ebc_params->chunk_in_states->validate_string(pArg1->c_str()))
        {
            thisAgent->explanationBasedChunker->ebc_params->chunk_in_states->set_string(pArg1->c_str());
            tempStringStream << "Learns rules in states: " << pArg1->c_str() << "\n";
            PrintCLIMessage(&tempStringStream);
            thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent, NULL);
            return true;
        } else {
            return SetError("Invalid chunking command.  Use 'chunk ?' to see a list of valid settings.");
        }
    }
    if (my_param == thisAgent->explanationBasedChunker->ebc_params->stats_cmd)
    {
        thisAgent->explanationMemory->print_global_stats();
    }
    else if ((my_param == thisAgent->explanationBasedChunker->ebc_params->always_cmd) ||
        (my_param == thisAgent->explanationBasedChunker->ebc_params->never_cmd) ||
        (my_param == thisAgent->explanationBasedChunker->ebc_params->flagged_cmd) ||
        (my_param == thisAgent->explanationBasedChunker->ebc_params->unflagged_cmd))
    {
        thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent, static_cast<soar_module::boolean_param*>(my_param));
        return true;
    }
    else if ((my_param == thisAgent->explanationBasedChunker->ebc_params->timers_cmd) && !pArg2)
    {
        double lEBC_total = thisAgent->explanationBasedChunker->ebc_timers->ebc_total->value();
        double lKernel_total = thisAgent->timers_total_kernel_time.get_sec();

        struct foo: public soar_module::accumulator< soar_module::timer* >
        {
            private:
                bool raw;
                double m_total;

                cli::CommandLineInterface* this_cli;
                std::ostringstream& m_Result;

                foo& operator=(const foo&)
                {
                    return *this;
                }

            public:
                foo(bool m_RawOutput, double pEBC_total, cli::CommandLineInterface* new_cli, std::ostringstream& m_Result): raw(m_RawOutput), m_total(pEBC_total), this_cli(new_cli), m_Result(m_Result) {};

                void operator()(soar_module::timer* t)
                {
                    std::string output(t->get_name());
                    std::string output_value;
                    std::snprintf(&output_value[0], output_value.capacity(), "%.2f sec", t->value());
                    std::string percentage;

                    bool isChunkSubTimer = (output != std::string("1.00 Instantiation creation"));
                    bool isEBCTimer = (output == std::string("2.13 EBC Total"));
                    output += ":";
                    if (!isChunkSubTimer || !m_total)
                    {
                        this_cli->PrintCLIMessage_Justify(output.c_str(), output_value.c_str(), 70);
                    }
                    else if (isEBCTimer)
                    {
                        this_cli->PrintCLIMessage_Section("", 70);
                        this_cli->PrintCLIMessage_Justify(output.c_str(), output_value.c_str(), 70);
                    }
                    else
                    {
                        std::snprintf(&percentage[0], percentage.capacity(), "(%.2f%%)", (t->value()/m_total)*100);
                        this_cli->PrintCLIMessage_Justify(output.c_str(), output_value.c_str(), 70, percentage.c_str());
                    }
                }
        } bar(m_RawOutput, lEBC_total, this, m_Result);

        PrintCLIMessage_Header("EBC Timers (in seconds)", 70);
        thisAgent->explanationBasedChunker->ebc_timers->for_each(bar);
        std::string tkt, percentage;
        std::snprintf(&tkt[0], tkt.capacity(), "%.2f sec", thisAgent->timers_total_kernel_time.get_sec());
        if (lKernel_total)
        {
            std::snprintf(&percentage[0], percentage.capacity(), "(%.2f%% in EBC)", (lEBC_total/lKernel_total)*100);
        }
        PrintCLIMessage_Section("", 70);
        PrintCLIMessage_Justify("Kernel Total:", tkt.c_str(), 70, percentage.c_str());
    }
    else if (my_param == thisAgent->explanationBasedChunker->ebc_params->singleton)
    {
        if (!pArg2)
        {
            if (doClear) {
                thisAgent->explanationBasedChunker->clear_singletons();
                PrintCLIMessage("User-defined singleton WME patterns cleared.");
            } else {
                thisAgent->explanationBasedChunker->print_singleton_summary();
            }
        } else {
            assert(pArg3 && pArg4);
            singleton_element_type id_type, value_type;
            Symbol* attrSym = thisAgent->symbolManager->make_str_constant(pArg3->c_str());
            std::string resultString;
            if (!attrSym)
            {
                return SetError("Invalid attribute element of singleton.  Must be a valid string constant.");
            }
            if (thisAgent->explanationBasedChunker->ebc_params->element_type->validate_string(pArg2->c_str()))
            {
                thisAgent->explanationBasedChunker->ebc_params->element_type->set_string(pArg2->c_str());
                id_type = thisAgent->explanationBasedChunker->ebc_params->element_type->get_value();
            } else {
                return SetError("Invalid pattern type for identifier element of singleton.  Must be any, constant, identifier, operator, or state.");
            }
            if (thisAgent->explanationBasedChunker->ebc_params->element_type->validate_string(pArg4->c_str()))
            {
                thisAgent->explanationBasedChunker->ebc_params->element_type->set_string(pArg4->c_str());
                value_type = thisAgent->explanationBasedChunker->ebc_params->element_type->get_value();
            } else {
                return SetError("Invalid pattern type for value element of singleton.  Must be any, constant, identifier, operator, or state.");
            }
            if (doRemove)
            {
                resultString = thisAgent->explanationBasedChunker->remove_singleton(id_type, attrSym, value_type);
            } else {
                resultString = thisAgent->explanationBasedChunker->add_new_singleton(id_type, attrSym, value_type);
            }
            if (!IsSourcingFile())
            {
                PrintCLIMessage(&resultString);
            }
            return true;
        }
    }
    else if ((my_param == thisAgent->explanationBasedChunker->ebc_params->help_cmd) || (my_param == thisAgent->explanationBasedChunker->ebc_params->qhelp_cmd))
    {
        thisAgent->explanationBasedChunker->print_chunking_settings();
    }
    else {
        if (!pArg2)
        {
            tempStringStream << my_param->get_name() << " is" ;
            PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
        } else {
            if (!my_param->validate_string(pArg2->c_str()))
            {
                return SetError("Invalid argument for chunking command. Use 'chunk ?' to see a list of valid settings.");
            }

            bool result = my_param->set_string(pArg2->c_str());

            if (!result)
            {
                return SetError("The chunking parameter could not be changed.");
            }
            else
            {
                tempStringStream << my_param->get_name() << " is now " << pArg2->c_str();
                PrintCLIMessage(&tempStringStream);
            }
            /* The following code assumes that all parameters except learn are boolean */
            if (!strcmp(pArg1->c_str(), "learn"))
            {
                thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent);
            } else if (!strcmp(pArg1->c_str(), "max-chunks") || !strcmp(pArg1->c_str(), "max-dupes")) {
                thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent, NULL, static_cast<soar_module::integer_param*>(my_param));
            } else {
                thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent, static_cast<soar_module::boolean_param*>(my_param));
            }
            return result;
        }
    }
    return true;
}

bool CommandLineInterface::DoLearn(const LearnBitset& options)
{
    // No options means print current settings
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (options.none() || options.test(LEARN_LIST))
    {
        std::string tempstr1(""), tempstr2("");
        DoChunk();
        PrintCLIMessage("Warning:  'learn' has been deprecated.  New corresponding command is: chunk");
        return true;
    }

    if (options.test(LEARN_ONLY))
    {
        std::string tempstr1("learn"), tempstr2("only");
        DoChunk(&tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_EXCEPT))
    {
        std::string tempstr1("learn"), tempstr2("all-except");
        DoChunk(&tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_ENABLE))
    {
        std::string tempstr1("learn"), tempstr2("always");
        DoChunk(&tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_DISABLE))
    {
        std::string tempstr1("learn"), tempstr2("never");
        DoChunk(&tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_ALL_LEVELS))
    {
        std::string tempstr1("bottom-only"), tempstr2("off");
        DoChunk(&tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_BOTTOM_UP))
    {
        std::string tempstr1("bottom-only"), tempstr2("on");
        DoChunk(&tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_ENABLE_THROUGH_LOCAL_NEGATIONS))
    {
        std::string tempstr1("allow-local-negations"), tempstr2("on");
        DoChunk(&tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_DISABLE_THROUGH_LOCAL_NEGATIONS))
    {
        std::string tempstr1("allow-local-negations"), tempstr2("off");
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_ENABLE_THROUGH_EVALUATION_RULES))
    {
        std::string tempstr1("add-osk"), tempstr2("on");
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_DISABLE_THROUGH_EVALUATION_RULES))
    {
        std::string tempstr1("add-osk"), tempstr2("off");
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    thisAgent->explanationBasedChunker->ebc_params->update_params(thisAgent->explanationBasedChunker->ebc_settings);
    return true;
}
