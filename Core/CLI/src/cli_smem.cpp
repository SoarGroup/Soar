/////////////////////////////////////////////////////////////////
// smem command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com,
//         Nate Derbinsky, nlderbin@umich.edu
// Date  : 2009
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "semantic_memory.h"
#include "agent.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSMem(const char pOp, const std::string* pAttr, const std::string* pVal)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempString;
    
    if (!pOp)
    {
        // Print SMem Settings
        PrintCLIMessage_Header("Semantic Memory Settings", 40);
        PrintCLIMessage_Item("learning:", thisAgent->smem_params->learning, 40);
        PrintCLIMessage_Section("Storage", 40);
        PrintCLIMessage_Item("database:", thisAgent->smem_params->database, 40);
        PrintCLIMessage_Item("append:", thisAgent->smem_params->append_db, 40);
        PrintCLIMessage_Item("path:", thisAgent->smem_params->path, 40);
        PrintCLIMessage_Item("lazy-commit:", thisAgent->smem_params->lazy_commit, 40);
        PrintCLIMessage_Section("Activation", 40);
        PrintCLIMessage_Item("activation-mode:", thisAgent->smem_params->activation_mode, 40);
        PrintCLIMessage_Item("activate-on-query:", thisAgent->smem_params->activate_on_query, 40);
        PrintCLIMessage_Item("base-decay:", thisAgent->smem_params->base_decay, 40);
        PrintCLIMessage_Item("base-update-policy:", thisAgent->smem_params->base_update, 40);
        PrintCLIMessage_Item("base-incremental-threshes:", thisAgent->smem_params->base_incremental_threshes, 40);
        PrintCLIMessage_Item("thresh:", thisAgent->smem_params->thresh, 40);
        PrintCLIMessage_Section("Performance", 40);
        PrintCLIMessage_Item("page-size:", thisAgent->smem_params->page_size, 40);
        PrintCLIMessage_Item("cache-size:", thisAgent->smem_params->cache_size, 40);
        PrintCLIMessage_Item("optimization:", thisAgent->smem_params->opt, 40);
        PrintCLIMessage_Item("timers:", thisAgent->smem_params->timers, 40);
        PrintCLIMessage_Section("Experimental", 40);
        PrintCLIMessage_Item("merge:", thisAgent->smem_params->merge, 40);
        PrintCLIMessage_Item("mirroring:", thisAgent->smem_params->mirroring, 40);
        PrintCLIMessage("");
        
        return true;
    }
    else if (pOp == 'a')
    {
        std::string* err = NULL;
        bool result = smem_parse_chunks(thisAgent, pAttr->c_str(), &(err));
        
        if (!result)
        {
            SetError(*err);
            delete err;
        }
        else
        {
            PrintCLIMessage("Knowledge added to semantic memory.");
        }
        
        return result;
    }
    else if (pOp == 'b')
    {
        std::string err;
        bool result = smem_backup_db(thisAgent, pAttr->c_str(), &(err));
        
        if (!result)
        {
            SetError("Error while backing up database: " + err);
        }
        else
        {
            tempString << "Semantic memory database backed up to " << pAttr->c_str();
            PrintCLIMessage(&tempString);
        }
        
        return result;
    }
    else if (pOp == 'e')
    {
        bool result = thisAgent->smem_params->learning->set_string("on");
        
        if (!result)
        {
            SetError("This parameter is protected while the semantic memory database is open.");
        }
        else
        {
            PrintCLIMessage("Semantic memory enabled.");
        }
        
        return result;
    }
    else if (pOp == 'd')
    {
        bool result = thisAgent->smem_params->learning->set_string("off");
        
        if (!result)
        {
            SetError("This parameter is protected while the semantic memory database is open.");
        }
        else
        {
            PrintCLIMessage("Semantic memory disabled.");
        }
        
        return result;
    }
    else if (pOp == 'g')
    {
        soar_module::param* my_param = thisAgent->smem_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid semantic memory parameter.  Use 'help smem' to see list of valid settings.");
        }
        
        PrintCLIMessage_Item("", my_param, 0);
        return true;
    }
    else if (pOp == 'h')
    {
        smem_lti_id lti_id = NIL;
        uint64_t depth = 1;
        bool history = true;
        smem_attach(thisAgent);

        if (thisAgent->smem_db->get_status() != soar_module::connected)
        {
            return SetError("Semantic memory database not connected.");
        }

        if (pAttr)
        {
            get_lexeme_from_string(thisAgent, pAttr->c_str());
            if (thisAgent->lexeme.type == IDENTIFIER_LEXEME)
            {
                lti_id = smem_lti_get_id(thisAgent, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number);
            }
            if (lti_id == NIL)
            {
                return SetError("LTI not found");
            }
        }

        std::string viz;

        smem_print_lti(thisAgent, lti_id, depth, &(viz), history);

        if (viz.empty())
        {
            return SetError("Could not find information on LTI.");
        }

        PrintCLIMessage_Header("Semantic Memory", 40);
        PrintCLIMessage(&viz);
        return true;
    }
    else if (pOp == 'i')
    {
        // Because of LTIs, re-initializing requires all other memories to be reinitialized.
        // epmem - close before working/production memories to get re-init benefits
        // smem - close before working/production memories to prevent id counter mess-ups
        // production memory (automatic init-soar clears working memory as a result)
        
        epmem_reinit_cmd(thisAgent);
        smem_reinit_cmd(thisAgent);
        
        ExciseBitset options(0);
        options.set(EXCISE_ALL, true);
        DoExcise(options, 0);
        
        PrintCLIMessage("Semantic memory system re-initialized.");
        return true;
    }
    else if (pOp == 'p')
    {
        smem_lti_id lti_id = NIL;
        unsigned int depth = 1;
        
        smem_attach(thisAgent);
        if (thisAgent->smem_db->get_status() != soar_module::connected)
        {
            return SetError("Semantic memory database not connected.");
        }
        
        if (pAttr)
        {
            get_lexeme_from_string(thisAgent, pAttr->c_str());
            if (thisAgent->lexeme.type == IDENTIFIER_LEXEME)
            {
                if (thisAgent->smem_db->get_status() == soar_module::connected)
                {
                    lti_id = smem_lti_get_id(thisAgent, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number);
                    
                    if ((lti_id != NIL) && pVal)
                    {
                        from_c_string(depth, pVal->c_str());
                    }
                }
            }
            
            if (lti_id == NIL)
            {
                return SetError("LTI not found.");
            }
        }
        
        std::string viz;
        
        if (lti_id == NIL)
        {
            smem_print_store(thisAgent, &(viz));
            PrintCLIMessage_Header("Semantic Memory", 40);
        }
        else
        {
            smem_print_lti(thisAgent, lti_id, depth, &(viz));
        }
        if (viz.empty())
        {
            return SetError("Semantic memory is empty.");
        }
        
        PrintCLIMessage(&viz);
        return true;
    }
    else if (pOp == 'q')
    {
        std::string *err = new std::string;
        std::string *retrieved = new std::string;
        uint64_t nunmber_to_retrieve = 1;

        if (pVal)
        {
            from_c_string(number_to_retrieve, pVal->c_str());
        }

        bool result = smem_parse_cues(thisAgent, pAttr->c_str(), &(err), &(retrieved), number_to_retrieve);

        if (!result)
        {
            SetError("Error while parsing query" + *err + ".");
        }
        else
        {
            PrintCLIMessage(retrieved);
            PrintCLIMessage("SMem| Query complete.");
        }
        delete err;
        delete retrieved;
        return result;
    }
    else if (pOp == 'r')
    {
        std::string *err = new std::string;
        std::string *retrieved = new std::string;
        bool force = false;
        //I need to add here a check for the number of arguments, and if there is an extra one,
        //check if it is the "force". If it is, then I need to pass a nondefault boolean argument to smem_parse_remove
        //that tells the function to attempt to continue removing even when something doesn't seem right.
        if (pVal)
        {
            force = (!strcmp(pVal->c_str(),'f') || (!strcmp(pVal->c_str(),'force'));
        }

        bool result = smem_parse_remove(thisAgent, pAttr->c_str(), &(err), &(retrieved), force);

        if (!result)
        {
            SetError("Error while attempting removal.\n" + *err);
        }
        else
        {
            PrintCLIMessage(retrieved);
            PrintCLIMessage("SMem| Removal complete.");
        }
        delete err;
        delete retrieved;
        return result;
    }
    else if (pOp == 's')
    {
        soar_module::param* my_param = thisAgent->smem_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid SMem parameter.");
        }
        
        if (!my_param->validate_string(pVal->c_str()))
        {
            return SetError("Invalid setting for SMem parameter.");
        }
        
        smem_param_container::db_choices last_db_mode = thisAgent->smem_params->database->get_value();
        bool result = my_param->set_string(pVal->c_str());
        
        if (!result)
        {
            SetError("This parameter is protected while the semantic memory database is open.");
        }
        else
        {
            tempString << pAttr->c_str() << " = " << pVal->c_str();
            PrintCLIMessage(&tempString);
            if (thisAgent->smem_db->get_status() == soar_module::connected)
            {
                if (((!strcmp(pAttr->c_str(), "database")) && (thisAgent->smem_params->database->get_value() != last_db_mode)) ||
                        (!strcmp(pAttr->c_str(), "path")))
                {
                    PrintCLIMessage("To finalize database switch, issue an smem --init command.\n");
                }
            }
            if (!strcmp(pAttr->c_str(), "append"))
            {
                if (thisAgent->smem_params->append_db->get_value() == off)
                {
                    PrintCLIMessage("Warning: Since append mode is off, starting/reinitializing,\n"
                                    "         Soar will erase the semantic memory database.\n");
                }
                
            }
        }
        return result;
    }
    else if (pOp == 'S')
    {
        smem_attach(thisAgent);
        if (!pAttr)
        {
            // Print SMem Settings
            PrintCLIMessage_Header("Semantic Memory Statistics", 40);
            PrintCLIMessage_Item("SQLite Version:", thisAgent->smem_stats->db_lib_version, 40);
            PrintCLIMessage_Item("Memory Usage:", thisAgent->smem_stats->mem_usage, 40);
            PrintCLIMessage_Item("Memory Highwater:", thisAgent->smem_stats->mem_high, 40);
            PrintCLIMessage_Item("Retrieves:", thisAgent->smem_stats->expansions, 40);
            PrintCLIMessage_Item("Queries:", thisAgent->smem_stats->cbr, 40);
            PrintCLIMessage_Item("Stores:", thisAgent->smem_stats->stores, 40);
            PrintCLIMessage_Item("Activation Updates:", thisAgent->smem_stats->act_updates, 40);
            PrintCLIMessage_Item("Mirrors:", thisAgent->smem_stats->mirrors, 40);
            PrintCLIMessage_Item("Nodes:", thisAgent->smem_stats->chunks, 40);
            PrintCLIMessage_Item("Edges:", thisAgent->smem_stats->slots, 40);
        }
        else
        {
            soar_module::statistic* my_stat = thisAgent->smem_stats->get(pAttr->c_str());
            if (!my_stat)
            {
                return SetError("Invalid statistic.");
            }
            
            PrintCLIMessage_Item("", my_stat, 0);
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
                    
                    foo& operator=(const foo&)
                    {
                        return *this;
                    }
                    
                public:
                    foo(bool m_RawOutput, cli::CommandLineInterface* new_cli, std::ostringstream& m_Result): raw(m_RawOutput), this_cli(new_cli), m_Result(m_Result) {};
                    
                    void operator()(soar_module::timer* t)
                    {
                        std::string output(t->get_name());
                        output += ":";
                        this_cli->PrintCLIMessage_Item(output.c_str(), t, 40);
                    }
            } bar(m_RawOutput, this, m_Result);
            
            PrintCLIMessage_Header("Semantic Memory Timers", 40);
            thisAgent->smem_timers->for_each(bar);
        }
        else
        {
            soar_module::timer* my_timer = thisAgent->smem_timers->get(pAttr->c_str());
            if (!my_timer)
            {
                return SetError("Invalid timer.");
            }
            
            PrintCLIMessage_Item("", my_timer, 0);
        }
        
        return true;
    }
    else if (pOp == 'v')
    {
        smem_lti_id lti_id = NIL;
        unsigned int depth = 1;
        
        // vizualizing the store requires an open semantic database
        smem_attach(thisAgent);
        
        if (pAttr)
        {
            get_lexeme_from_string(thisAgent, pAttr->c_str());
            if (thisAgent->lexeme.type == IDENTIFIER_LEXEME)
            {
                if (thisAgent->smem_db->get_status() == soar_module::connected)
                {
                    lti_id = smem_lti_get_id(thisAgent, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number);
                    
                    if ((lti_id != NIL) && pVal)
                    {
                        from_c_string(depth, pVal->c_str());
                    }
                }
            }
            
            if (lti_id == NIL)
            {
                return SetError("Invalid long-term identifier.");
            }
        }
        
        std::string viz;
        
        if (lti_id == NIL)
        {
            smem_visualize_store(thisAgent, &(viz));
        }
        else
        {
            smem_visualize_lti(thisAgent, lti_id, depth, &(viz));
        }
        
        if (viz.empty())
        {
            return SetError("Nothing to visualize.");
        }
        PrintCLIMessage(&viz);
        
        return true;
    }
    
    return SetError("Unknown option.");
}
