/*
 * soarinstance.cpp
 *
 *  Created on: Nov 19, 2013
 *      Author: mazzin
 */

#include <iostream>

#include "soar_instance.h"
#include "print.h"
#include "output_manager.h"
#include "sml_Client.h"
#include "sml_AgentSML.h"
#include "debug.h"
#include "agent.h"
#include "mempool_manager.h"

Soar_Instance::Soar_Instance() :
    m_Kernel(NULL),
    chunkNameFormat(ruleFormat)
{
    m_loadedLibraries = new std::map<std::string, Soar_Loaded_Library* >();
    m_agent_table = new std::map< char*, Agent_Info*, cmp_str >();
    dprint_header(DT_SOAR_INSTANCE, PrintBoth, "= Soar instance created =\n");
}

void Soar_Instance::init_Soar_Instance(sml::Kernel* pKernel)
{
    m_Kernel = pKernel;

    /* -- Sets up the Output Manager -- */
    m_Output_Manager = &Output_Manager::Get_OM();
    m_Output_Manager->init_Output_Manager(pKernel, this);
    m_Memory_Manager = &MemPool_Manager::Get_MPM();
    m_Memory_Manager->init_MemPool_Manager(pKernel, this);

}

Soar_Instance::~Soar_Instance()
{
    dprint_header(DT_SOAR_INSTANCE, PrintBefore, "= Destroying Soar instance =\n");
    m_Kernel = NULL;

    for (std::map< std::string, Soar_Loaded_Library* >::iterator it = (*m_loadedLibraries).begin(); it != (*m_loadedLibraries).end(); ++it)
    {
        dprint(DT_SOAR_INSTANCE, "Sending CLI module %s a DELETE command.\n", it->first.c_str());
        it->second->libMessageFunction("delete", NULL);
    }
    for (std::map< std::string, Soar_Loaded_Library* >::iterator it = (*m_loadedLibraries).begin(); it != (*m_loadedLibraries).end(); ++it)
    {
        delete it->second;
    }
    m_loadedLibraries->clear();
    delete m_loadedLibraries;

    for (std::map< char*, Agent_Info*, cmp_str >::iterator it = (*m_agent_table).begin(); it != (*m_agent_table).end(); ++it)
    {
        delete it->second;
    }
    m_agent_table->clear();
    delete m_agent_table;

    dprint_header(DT_SOAR_INSTANCE, PrintAfter, "= Soar instance destroyed =\n");
}

void Soar_Instance::Register_Library(sml::Kernel* pKernel, const char* pLibName, MessageFunction pMessageFunction)
{
    // Convert to lower case
    std::string lLibName(pLibName);
    std::transform(lLibName.begin(), lLibName.end(), lLibName.begin(), ::tolower);

    /* -- Store library information -- */
    std::map< std::string, Soar_Loaded_Library* >::iterator iter = (*m_loadedLibraries).find(lLibName);
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

std::string Soar_Instance::Message_Library(const char* pMessage)
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

    std::map< std::string, Soar_Loaded_Library* >::iterator iter = (*m_loadedLibraries).find(lCLIExtensionName.c_str());
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
            resultString = "\n" + lCLIExtensionName + " CLI module loaded and enabled.\n";
            m_Output_Manager->print(resultString.c_str());
        }
        else if (lMessage == "off")
        {
            resultString = lCLIExtensionName + " CLI module deactivated.\n";
            m_Output_Manager->print(resultString.c_str());
            libraryInfo->isOn = false;
        }
    }

    resultString.erase();
    return resultString;
}

void Soar_Instance::Register_Soar_AgentSML(char* pAgentName, sml::AgentSML* pSoarAgentSML)
{
    Agent_Info* lAgent_Info;
    lAgent_Info = Get_Agent_Info(pAgentName);
    if (!lAgent_Info)
    {
        lAgent_Info = new Agent_Info;
        (*m_agent_table)[strdup(pAgentName)] = lAgent_Info;
    }
    lAgent_Info->soarAgentSML = pSoarAgentSML;

    /* -- If only agent, make sure it's the default agent for soar debug printing. -- */
    if (m_agent_table->size() == 1)
    {
        m_Output_Manager->set_default_agent(pSoarAgentSML->GetSoarAgent());
    }
}

void Soar_Instance::Delete_Agent(char* pAgentName)
{
    Agent_Info* lAgent_Info;
    char* lAgent_Name;
    bool update_OM = false;

    /* -- Update the Output Manager with the agent we're deleting -- */
    if (!strcmp(m_Output_Manager->get_default_agent()->name, pAgentName))
    {
        update_OM = true;
    }

    /* -- Delete agent from agent table -- */
    std::map< char*, Agent_Info*, cmp_str >::iterator iter = (*m_agent_table).find(pAgentName);
    if (iter != (*m_agent_table).end())
    {
        lAgent_Name = iter->first;
        lAgent_Info = iter->second;
        (*m_agent_table).erase(pAgentName);
        free(lAgent_Name);
        delete lAgent_Info;

        if (update_OM)
        {
            if (m_agent_table->size() > 0)
            {
                m_Output_Manager->set_default_agent((*m_agent_table).begin()->second->soarAgentSML->GetSoarAgent());
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
    for (std::map< char*, Agent_Info*, cmp_str >::iterator it = (*m_agent_table).begin(); it != (*m_agent_table).end(); ++it)
    {
        m_Output_Manager->print_sf("%s -> %s\n", it->first, it->second->soarAgentSML->GetSoarAgent()->name);
    }
}

Agent_Info* Soar_Instance::Get_Agent_Info(char* pAgentName)
{
    std::map< char*, Agent_Info*, cmp_str >::iterator iter = (*m_agent_table).find(pAgentName);
    if (iter != (*m_agent_table).end())
    {
        return iter->second;
    }
    return NULL;
}

sml::AgentSML* Soar_Instance::Get_Soar_AgentSML(char* pAgentName)
{
    Agent_Info* lAgent_Info;
    lAgent_Info = Get_Agent_Info(pAgentName);

    if (lAgent_Info)
    {
        return lAgent_Info->soarAgentSML;
    }
    return NULL;
}

void Soar_Instance::CLI_Debug_Print(const char* text)
{
    this->m_Output_Manager->debug_print(DT_CLI_LIBRARIES, text);
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
