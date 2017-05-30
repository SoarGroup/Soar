/*
 * soarinstance.h
 *
 *  Created on: Nov 19, 2013
 *      Author: mazzin
 */

#ifndef SOARINSTANCE_H_
#define SOARINSTANCE_H_

#include "kernel.h"
#include "Export.h"

#include <unordered_map>

typedef void* (*MessageFunction)(const char* pMessage, void* pMessageData);
typedef struct Soar_Loaded_Library_struct
{
    MessageFunction libMessageFunction;
    bool isOn;
} Soar_Loaded_Library;

class EXPORT Soar_Instance
{
    public:

        static Soar_Instance& Get_Soar_Instance()
        {
            static Soar_Instance instance;
            return instance;
        }
        ~Soar_Instance();

        void                        init_Soar_Instance(sml::Kernel* pKernel);

        void                        Register_Library(sml::Kernel* pKernel, const char* pLibName, MessageFunction pMessageFunction);
        std::string                 Tcl_Message_Library(const char* pMessage);
        bool                        is_Tcl_on() { return m_tcl_enabled; };
        std::string                 Message_Library(std::string &pMessage);

        void                        Register_Soar_AgentSML(char* pAgentName, sml::AgentSML* pSoarAgentSML);
        void                        Delete_Agent(char* pAgentName);
        sml::AgentSML*              Get_Agent_Info(const char* pAgentName);

        void                        CLI_Debug_Print(const char* text);

        void                        configure_for_unit_tests()                  { m_launched_by_unit_test = true; }
        bool                        was_run_from_unit_test()                    { return m_launched_by_unit_test; };

        void                        Set_Kernel(sml::Kernel* pKernel)            { m_Kernel = pKernel; };
        void                        Set_OM(Output_Manager* pOutput_Manager)     { m_Output_Manager = pOutput_Manager; };
        void                        Set_CLI(cli::CommandLineInterface* pCLI)    { m_CLI = pCLI; };

        sml::Kernel*                Get_Kernel()                                { return m_Kernel; };
        Output_Manager*             Get_OM()                                    { return m_Output_Manager; };
        cli::CommandLineInterface*  Get_CLI()                                   { return m_CLI; };

    private:

        Soar_Instance();
        Soar_Instance(Soar_Instance const&) {};
        void operator=(Soar_Instance const&) {};

        void Print_Agent_Table();

        sml::Kernel*                m_Kernel;
        Output_Manager*             m_Output_Manager;
        cli::CommandLineInterface*  m_CLI;
        bool                        m_launched_by_unit_test;
        bool                        m_tcl_enabled;

        std::unordered_map< std::string, sml::AgentSML*>* m_agent_table;
        std::unordered_map< std::string, Soar_Loaded_Library* >* m_loadedLibraries;
};

/* -- getSoarInstance is used by libraries to retrieve the
 *    SoarInstance via a SWIG proxy function.  It is a bit of a hack
 *    currently used CLI libraries like the SoarTcl library module. -- */

EXPORT Soar_Instance* getSoarInstance();
EXPORT Output_Manager* getOM();
EXPORT void configure_for_unit_tests();
EXPORT void configure_agent_for_unit_tests(agent* testAgent);

#endif /* SOARINSTANCE_H_ */
