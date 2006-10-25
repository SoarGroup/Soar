/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_donottouch.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef GSKI_DONOTTOUCH_H
#define GSKI_DONOTTOUCH_H

typedef struct production_struct production;
typedef unsigned char wme_trace_type;
typedef struct rete_node_struct rete_node;
typedef unsigned char ms_trace_type;
typedef struct agent_struct agent;

typedef void * soar_callback_agent;

#include <string>

namespace gSKI
{
   class Agent;
   class Kernel;
   namespace EvilBackDoor 
   {
      class TgDWorkArounds
      {
      public:

         /**
         * @brief
         */
         virtual ~TgDWorkArounds(){}

         /**
         * @brief
         */
         TgDWorkArounds(){}

         /**
         * @brief
         */
         void SetSysparam (Agent* agent, int param_number, long new_value);
         long GetSysparam(Agent* agent, int param_number);
		 const long* GetSysparams(Agent* agent);

         rete_node* NameToProduction(Agent* agent, char* string_to_test);
         void PrintPartialMatchInformation(Agent* thisAgent, 
         struct rete_node_struct *p_node, wme_trace_type wtt);
         void PrintMatchSet(Agent* thisAgent, wme_trace_type wtt, ms_trace_type  mst);
		 virtual void XMLPartialMatchInformation(Agent* thisAgent, 
                                                   struct rete_node_struct *p_node,
                                                   wme_trace_type wtt) ;
        virtual void XMLMatchSet(Agent* thisAgent, wme_trace_type wtt, ms_trace_type  mst) ;

         void PrintStackTrace(Agent* thisAgent, bool print_states, bool print_operators);
         void PrintSymbol(Agent*     thisAgent, 
                          char*       arg, 
                          bool        name_only, 
                          bool        print_filename, 
                          bool        internal,
                          bool        full_prod,
                          int         depth);
         void PrintUser(Agent*       thisAgent,
                        char*         arg,
                        bool          internal,
                        bool          print_filename,
                        bool          full_prod,
                        unsigned int  productionType);
                     
         bool Preferences(Agent* thisAgent, int detail, bool object, const char* idString, const char* attrString);

         bool ProductionFind(Agent*     thisAgent,
                             agent*      agnt,
                             Kernel*    kernel,
                             bool        lhs,
                             bool        rhs,
                             char*       arg,
                             bool        show_bindings,
                             bool        just_chunks,
                             bool        no_chunks);

         bool GDSPrint(Agent* thisAgent);

		 void GetForceLearnStates(Agent* pIAgent, std::string& res);
		 void GetDontLearnStates(Agent* pIAgent, std::string& res);

		 void SetVerbosity(Agent* pIAgent, bool setting);
		 bool GetVerbosity(Agent* pIAgent);

		 bool BeginTracingProduction(Agent* pIAgent, const char* pProductionName);
		 bool StopTracingProduction(Agent* pIAgent, const char* pProductionName);

		 long AddWme(Agent* pIAgent, const char* pIdString, const char* pAttrString, const char* pValueString, bool acceptable);
		 int RemoveWmeByTimetag(Agent* pIAgent, int num);

		 void PrintInternalSymbols(Agent* pIAgent);

		 int AddWMEFilter(Agent* pIAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes);
		 int RemoveWMEFilter(Agent* pIAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes);
		 bool ResetWMEFilters(Agent* pIAgent, bool adds, bool removes);
		 void ListWMEFilters(Agent* pIAgent, bool adds, bool removes);

		 void ExplainListChunks(Agent* pIAgent);
		 bool ExplainChunks(Agent* pIAgent, const char* pProduction, int mode);

		 const char* GetChunkNamePrefix(Agent* pIAgent);
		 bool SetChunkNamePrefix(Agent* pIAgent, const char* pPrefix);

		 unsigned long GetChunkCount(Agent* pIAgent);
		 void SetChunkCount(Agent* pIAgent, unsigned long count);

		 void SeedRandomNumberGenerator(unsigned long int* pSeed);
	  };
   }
}
#endif

