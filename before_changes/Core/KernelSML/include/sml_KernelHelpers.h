/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

#include <string>

// Included because we need XMLCallbackData defined in KernelSML/CLI
#include "xml.h"
namespace soarxml
{
  class XMLTrace ;
}

namespace sml
{
  class AgentSML ;
  class KernelSML ;

  class KernelHelpers
  {
  public:

     /**
     * @brief
     */
     virtual ~KernelHelpers(){}

     /**
     * @brief
     */
     KernelHelpers(){}

     /**
     * @brief
     */
	 void SetSysparam (AgentSML* agent, int param_number, long new_value);
     long GetSysparam(AgentSML* agent, int param_number);
	 const long* const GetSysparams(AgentSML* agent) const; // USE ONLY AS READ-ONLY

     rete_node* NameToProduction(AgentSML* agent, char* string_to_test);

     void PrintStackTrace(AgentSML* thisAgent, bool print_states, bool print_operators);
     void PrintSymbol(AgentSML*     thisAgent, 
                      char*       arg, 
                      bool        name_only, 
                      bool        print_filename, 
                      bool        internal,
					  bool        tree,
                      bool        full_prod,
                      int         depth);
     void PrintUser(AgentSML*       thisAgent,
                    char*         arg,
                    bool          internal,
                    bool          print_filename,
                    bool          full_prod,
                    unsigned int  productionType);
                 
     bool Preferences(AgentSML* thisAgent, int detail, bool object, const char* idString, const char* attrString);

     bool ProductionFind(agent*      agnt,
                         bool        lhs,
                         bool        rhs,
                         char*       arg,
                         bool        show_bindings,
                         bool        just_chunks,
                         bool        no_chunks);

     bool GDSPrint(AgentSML* thisAgent);

	 void GetForceLearnStates(AgentSML* pIAgent, std::stringstream& res);
	 void GetDontLearnStates(AgentSML* pIAgent, std::stringstream& res);

	 void SetVerbosity(AgentSML* pIAgent, bool setting);
	 bool GetVerbosity(AgentSML* pIAgent);

	 bool BeginTracingProduction(AgentSML* pIAgent, const char* pProductionName);
	 bool StopTracingProduction(AgentSML* pIAgent, const char* pProductionName);

	 void PrintInternalSymbols(AgentSML* pIAgent);

	 int AddWMEFilter(AgentSML* pIAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes);
	 int RemoveWMEFilter(AgentSML* pIAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes);
	 bool ResetWMEFilters(AgentSML* pIAgent, bool adds, bool removes);
	 void ListWMEFilters(AgentSML* pIAgent, bool adds, bool removes);

	 void ExplainListChunks(AgentSML* pIAgent);
	 bool ExplainChunks(AgentSML* pIAgent, const char* pProduction, int mode);
	 void print_rl_rules( agent* thisAgent, char *arg,bool internal, bool print_filename, bool full_prod);

  };
}
#endif

