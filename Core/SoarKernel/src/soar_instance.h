/*
 * soarinstance.h
 *
 *  Created on: Nov 19, 2013
 *      Author: mazzin
 */

#ifndef SOARINSTANCE_H_
#define SOARINSTANCE_H_

#include "portability.h"
#include <map>
#include "Export.h"
#include "enums.h"

namespace sml
{
    class Kernel;
    class AgentSML;
}

class Output_Manager;
class MemPool_Manager;

typedef struct agent_struct agent;

typedef void* (*MessageFunction)(const char* pMessage, void* pMessageData);
typedef struct Soar_Loaded_Library_struct
{
    MessageFunction libMessageFunction;
    bool isOn;
} Soar_Loaded_Library;

/* -- This stores a mapping from an agent name to pointers for
 *    to the AgentSML structure. (Does this mean we require
 *    that CLI modules are only used within embedded kernels?)
 *     -- */

typedef struct Agent_Info_Struct
{
    sml::AgentSML* soarAgentSML;
} Agent_Info;

struct cmp_str
{
    bool operator()(char const* a, char const* b) const
    {
        return strcmp(a, b) < 0;
    }
};

class EXPORT Soar_Instance
{
    public:

        static Soar_Instance& Get_Soar_Instance()
        {
            static Soar_Instance instance;
            return instance;
        }
        ~Soar_Instance();

        void init_Soar_Instance(sml::Kernel* pKernel);
        void Register_Library(sml::Kernel* pKernel, const char* pLibName, MessageFunction pMessageFunction);
        std::string Message_Library(const char* pMessage);

        void Register_Soar_AgentSML(char* pAgentName, sml::AgentSML* pSoarAgentSML);
        void Delete_Agent(char* pAgentName);
        sml::AgentSML* Get_Soar_AgentSML(char* pAgentName);

        void Set_Kernel(sml::Kernel* pKernel)
        {
            m_Kernel = pKernel;
        };
        sml::Kernel* Get_Kernel()
        {
            return m_Kernel;
        };

        void Set_OM(Output_Manager* pOutput_Manager)
        {
            m_Output_Manager = pOutput_Manager;
        };
        Output_Manager* Get_OM()
        {
            return m_Output_Manager;
        };

        void CLI_Debug_Print(const char* text);

        chunkNameFormats Get_Chunk_Name_Format() {return chunkNameFormat;};
        void Set_Chunk_Name_Format(chunkNameFormats pChunkNameFormat) {chunkNameFormat = pChunkNameFormat;};

    private:

        Soar_Instance();

        /* The following two functions are declared but not implemented to avoid copies of singletons */
        Soar_Instance(Soar_Instance const&) {};
        void operator=(Soar_Instance const&) {};

        sml::Kernel*             m_Kernel;
        Output_Manager*          m_Output_Manager;
        MemPool_Manager*         m_Memory_Manager;

        std::map< char*, Agent_Info*, cmp_str>* m_agent_table;
        std::map< std::string, Soar_Loaded_Library* >* m_loadedLibraries;

        chunkNameFormats chunkNameFormat;
        Agent_Info* Get_Agent_Info(char* pAgentName);
        void Print_Agent_Table();

};

/* -- getSoarInstance is used by libraries to retrieve the
 *    SoarInstance via a SWIG proxy function.  It is a bit of a hack
 *    currently used CLI libraries like the SoarTcl library module. -- */

EXPORT Soar_Instance* getSoarInstance();
EXPORT Output_Manager* getOM();

#endif /* SOARINSTANCE_H_ */
