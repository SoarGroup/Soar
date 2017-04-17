/*
 * soarinstance.cpp
 *
 *  Created on: Nov 19, 2013
 *      Author: mazzin
 */

#include "soar_instance.h"

#include "agent.h"
#include "dprint.h"
#include "memory_manager.h"
#include "output_manager.h"
#include "sml_Client.h"
#include "sml_AgentSML.h"

#include <iostream>

Soar_Instance::Soar_Instance()
{
    //std::cout << "= Creating Soar instance =\n";
    m_Kernel = NULL;
    m_Output_Manager = NULL;
    m_launched_by_unit_test = false;
    m_tcl_enabled = false;
    m_loadedLibraries = new std::unordered_map<std::string, Soar_Loaded_Library* >();
    m_agent_table = new std::unordered_map< std::string, sml::AgentSML* >();
}

void Soar_Instance::init_Soar_Instance(sml::Kernel* pKernel)
{
    m_Kernel = pKernel;

    /* -- Sets up the Output Manager -- */
    m_Output_Manager = &Output_Manager::Get_OM();
}

Soar_Instance::~Soar_Instance()
{
    dprint(DT_SOAR_INSTANCE, "= Destroying Soar instance =\n");
    m_Kernel = NULL;

    m_agent_table->clear();
    delete m_agent_table;

    dprint(DT_SOAR_INSTANCE, "Cleaning up loaded libraries...\n");
    for (std::unordered_map< std::string, Soar_Loaded_Library* >::iterator it = (*m_loadedLibraries).begin(); it != (*m_loadedLibraries).end(); ++it)
    {
        dprint(DT_SOAR_INSTANCE, "Sending CLI module %s a DELETE command.\n", it->first.c_str());
        it->second->libMessageFunction("delete", NULL);
    }
    for (std::unordered_map< std::string, Soar_Loaded_Library* >::iterator it = (*m_loadedLibraries).begin(); it != (*m_loadedLibraries).end(); ++it)
    {
        delete it->second;
    }
    m_loadedLibraries->clear();
    delete m_loadedLibraries;
    dprint(DT_SOAR_INSTANCE, "= Soar instance destroyed =\n");
}


void Soar_Instance::Register_Library(sml::Kernel* pKernel, const char* pLibName, MessageFunction pMessageFunction)
{
    // Convert to lower case
    std::string lLibName(pLibName);
    std::transform(lLibName.begin(), lLibName.end(), lLibName.begin(), ::tolower);

    /* -- Store library information -- */
    std::unordered_map< std::string, Soar_Loaded_Library* >::iterator iter = (*m_loadedLibraries).find(lLibName);
    if (iter == (*m_loadedLibraries).end())
    {
        if (!pMessageFunction)
        {
            m_Output_Manager->print("Library did not pass in a message function.  Not registering.\n");
            return;
        }

        Soar_Loaded_Library* new_library = new Soar_Loaded_Library;
        new_library->libMessageFunction = pMessageFunction;
        new_library->isOn = false;
        (*m_loadedLibraries)[lLibName] = new_library;

        dprint(DT_SOAR_INSTANCE, "CLI Extension %s registered.\n", lLibName.c_str());

    }
}

std::string Soar_Instance::Tcl_Message_Library(const char* pMessage)
{
    std::string lFullCommand("tcl ");
    lFullCommand.append(pMessage);
    return Message_Library(lFullCommand);
}

std::string Soar_Instance::Message_Library(std::string &pMessage)
{
    std::string resultString("CLI extension command failed.");
    Soar_Loaded_Library* libraryInfo;

    /* -- Convert command to lower case -- */
    std::string lFullCommand(pMessage);
    std::transform(lFullCommand.begin(), lFullCommand.end(), lFullCommand.begin(), ::tolower);

    /* -- Compose the library name = CLI extension name + SoarLib.
     *    Note that the LoadExternalLibrary command will take care of any platform-specific
     *    extensions. -- */
    std::string lCLIExtensionName = lFullCommand.substr(0, lFullCommand.find(' '));
    std::string lMessage = lFullCommand.substr(lCLIExtensionName.size() + 1, lFullCommand.size() - 1);
    lCLIExtensionName += "soarlib";

    std::unordered_map< std::string, Soar_Loaded_Library* >::iterator iter = (*m_loadedLibraries).find(lCLIExtensionName.c_str());
    if (iter == (*m_loadedLibraries).end())
    {
        // load library
        std::string result = m_Kernel->LoadExternalLibrary(lCLIExtensionName.c_str());

        // zero length is success
        if (result.size() != 0)
        {
            resultString = "Could not load library " + lCLIExtensionName + ": " + result;
            return resultString;
        }
    }
    /* -- A new library will register itself, so it will now have a
     *    libraryInfo entry even if it was not found above. -- */
    libraryInfo = (*m_loadedLibraries)[lCLIExtensionName.c_str()];

    if (((lMessage == "on") && libraryInfo->isOn) || ((lMessage == "off") && !libraryInfo->isOn))
    {
        resultString = "CLI extension " + lCLIExtensionName + "is already " + lMessage + ".  Ignoring command.";
        return resultString;
    }
    if (lMessage == "off")
    {
        resultString = "Turning off CLI modules is currently disabled. Will be fixed in future version.  Restart Soar to turn off for now.";
        return resultString;
    }
    void* success = libraryInfo->libMessageFunction(lMessage.c_str(), NULL);
    if (!success)
    {
        resultString = "Message " + lMessage + " to CLI library " + lCLIExtensionName + " returned unsuccessful.";
        return resultString;
    }
    else
    {
        // Note that may be other possible messages in the future, so these
        // arent' the only two cases.
        if (lMessage == "on")
        {
            libraryInfo->isOn = true;
            resultString = "\n" + lCLIExtensionName + " is loaded and enabled.\n";
            m_Output_Manager->print(resultString.c_str());
            m_tcl_enabled = true; // This was intended to be general purpose, but currently only used for tcl
        }
        else if (lMessage == "off")
        {
            resultString = lCLIExtensionName + " has been deactivated.\n";
            m_Output_Manager->print(resultString.c_str());
            libraryInfo->isOn = false;
            m_tcl_enabled = false; // This was intended to be general purpose, but currently only used for tcl
        }
    }

    resultString.erase();
    return resultString;
}

void Soar_Instance::Register_Soar_AgentSML(char* pAgentName, sml::AgentSML* pSoarAgentSML)
{
    std::unordered_map< std::string, sml::AgentSML* >::iterator iter = (*m_agent_table).find(pAgentName);
    if (iter == (*m_agent_table).end())
    {
        (*m_agent_table)[std::string(pAgentName)] = pSoarAgentSML;
    } else {
        iter->second = pSoarAgentSML;
    }

    /* -- If only agent, make sure it's the default agent for soar debug printing. -- */
    if (m_agent_table->size() == 1)
    {
        m_Output_Manager->set_default_agent(pSoarAgentSML->GetSoarAgent());
    }
}

void Soar_Instance::Delete_Agent(char* pAgentName)
{
    bool update_OM = false;

    /* -- Update the Output Manager with the agent we're deleting -- */
    if (!strcmp(m_Output_Manager->get_default_agent()->name, pAgentName))
    {
        update_OM = true;
    }

    /* -- Delete agent from agent table -- */
    std::unordered_map< std::string, sml::AgentSML* >::iterator iter = (*m_agent_table).find(pAgentName);
    if (iter != (*m_agent_table).end())
    {
        (*m_agent_table).erase(iter);

        if (update_OM)
        {
            if (m_agent_table->size() > 0)
            {
                m_Output_Manager->set_default_agent((*m_agent_table).begin()->second->GetSoarAgent());
            }
            else
            {
                m_Output_Manager->clear_default_agent();
            }
        }
        return;
    }
}

void Soar_Instance::Print_Agent_Table()
{
    m_Output_Manager->print("------------------------------------\n");
    m_Output_Manager->print("------------ Agent Table -----------\n");
    m_Output_Manager->print("------------------------------------\n");
    for (std::unordered_map< std::string, sml::AgentSML* >::iterator it = (*m_agent_table).begin(); it != (*m_agent_table).end(); ++it)
    {
        m_Output_Manager->print_sf("%s -> %s\n", it->first.c_str(), it->second->GetSoarAgent()->name);
    }
}

sml::AgentSML* Soar_Instance::Get_Agent_Info(const char* pAgentName)
{
    std::unordered_map< std::string, sml::AgentSML* >::iterator iter = (*m_agent_table).find(pAgentName);
    if (iter != (*m_agent_table).end())
    {
        return iter->second;
    }
    return NULL;
}

void Soar_Instance::CLI_Debug_Print(const char* text)
{
    this->m_Output_Manager->debug_print(DT_DEBUG, text);
}

void configure_for_unit_tests()
{
    Soar_Instance::Get_Soar_Instance().configure_for_unit_tests();
    Output_Manager::Get_OM().set_output_params_global(false);
    debug_set_mode_info(Output_Manager::Get_OM().mode_info, false);
}

void configure_agent_for_unit_tests(agent* testAgent)
{
    agent* thisAgent = testAgent ? testAgent : Output_Manager::Get_OM().get_default_agent();
    if (thisAgent)
    {
        thisAgent->output_settings->set_output_params_agent(false);
    }
}
/* -- The following is a bit of a hack used to get Tcl access
 *    to Soar data structures via SWIG proxy functions. -- */

Soar_Instance* getSoarInstance()
{
    return &(Soar_Instance::Get_Soar_Instance());
}

Output_Manager* getOM()
{
    return &(Output_Manager::Get_OM());
}
