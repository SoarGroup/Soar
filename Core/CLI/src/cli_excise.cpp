/////////////////////////////////////////////////////////////////
// excise command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "agent.h"
#include "production.h"
#include "symtab.h"

#include "reinforcement_learning.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseExcise(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "all",		OPTARG_NONE},
		{'c', "chunks",		OPTARG_NONE},
		{'d', "default",	OPTARG_NONE},
		{'r', "rl",			OPTARG_NONE},
		{'t', "task",		OPTARG_NONE},
		{'T', "template",	OPTARG_NONE},
		{'u', "user",		OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	ExciseBitset options(0);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'a':
				options.set(EXCISE_ALL);
				break;
			case 'c':
				options.set(EXCISE_CHUNKS);
				break;
			case 'd':
				options.set(EXCISE_DEFAULT);
				break;
			case 'r':
				options.set(EXCISE_RL);
				break;
			case 't':
				options.set(EXCISE_TASK);
				break;
			case 'T':
				options.set(EXCISE_TEMPLATE);
				break;
			case 'u':
				options.set(EXCISE_USER);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// If there are options, no additional argument.
	if (options.any()) {
		if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);
		return DoExcise(options);
	}

	// If there are no options, there must be only one production name argument
	if (m_NonOptionArguments < 1) {
		SetErrorDetail("Production name is required.");
		return SetError(CLIError::kTooFewArgs);		
	}
	if (m_NonOptionArguments > 1) {
		SetErrorDetail("Only one production name allowed, call excise multiple times to excise more than one specific production.");
		return SetError(CLIError::kTooManyArgs);		
	}

	// Pass the production to the DoExcise function
	return DoExcise(options, &(argv[m_Argument - m_NonOptionArguments]));
}

bool CommandLineInterface::DoExcise(const ExciseBitset& options, const std::string* pProduction) {
	int64_t exciseCount = 0;

	// Process the general options
	if (options.test(EXCISE_ALL)) {
		exciseCount += m_pAgentSoar->num_productions_of_type[USER_PRODUCTION_TYPE];
		exciseCount += m_pAgentSoar->num_productions_of_type[CHUNK_PRODUCTION_TYPE];
		exciseCount += m_pAgentSoar->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE];
		exciseCount += m_pAgentSoar->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];

		excise_all_productions( m_pAgentSoar, false );

		this->DoInitSoar();	// from the manual, init when --all or --task are executed
	}
	if (options.test(EXCISE_CHUNKS)) {
		exciseCount += m_pAgentSoar->num_productions_of_type[CHUNK_PRODUCTION_TYPE];
		exciseCount += m_pAgentSoar->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE];

		excise_all_productions_of_type(m_pAgentSoar, CHUNK_PRODUCTION_TYPE, false);
		excise_all_productions_of_type(m_pAgentSoar, JUSTIFICATION_PRODUCTION_TYPE, false);
	}
	if (options.test(EXCISE_DEFAULT)) {
	  exciseCount += m_pAgentSoar->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];

      excise_all_productions_of_type(m_pAgentSoar, DEFAULT_PRODUCTION_TYPE, false);
	}
	if (options.test(EXCISE_RL)) {					    
	   for ( production *prod = m_pAgentSoar->all_productions_of_type[DEFAULT_PRODUCTION_TYPE]; prod != NIL; prod = prod->next )
	   {
		   if ( prod->rl_rule )
		   {
			   exciseCount++;
			   excise_production( m_pAgentSoar, prod, static_cast<Bool>(m_pAgentSoar->sysparams[TRACE_LOADING_SYSPARAM]) );
		   }
	   }

	   for ( production *prod = m_pAgentSoar->all_productions_of_type[USER_PRODUCTION_TYPE]; prod != NIL; prod = prod->next )
	   {
		   if ( prod->rl_rule )
		   {
			   exciseCount++;
			   excise_production( m_pAgentSoar, prod, static_cast<Bool>(m_pAgentSoar->sysparams[TRACE_LOADING_SYSPARAM]) );
		   }
	   }

	   for ( production *prod = m_pAgentSoar->all_productions_of_type[CHUNK_PRODUCTION_TYPE]; prod != NIL; prod = prod->next )
	   {
		   if ( prod->rl_rule )
		   {
			   exciseCount++;
			   excise_production( m_pAgentSoar, prod, static_cast<Bool>(m_pAgentSoar->sysparams[TRACE_LOADING_SYSPARAM]) );
		   }
	   }
	   
	   rl_initialize_template_tracking( m_pAgentSoar );
	}
	if (options.test(EXCISE_TASK)) {
		exciseCount += m_pAgentSoar->num_productions_of_type[USER_PRODUCTION_TYPE];
		exciseCount += m_pAgentSoar->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];

		excise_all_productions_of_type(m_pAgentSoar, USER_PRODUCTION_TYPE, false);
		excise_all_productions_of_type(m_pAgentSoar, DEFAULT_PRODUCTION_TYPE, false);

	    this->DoInitSoar();	// from the manual, init when --all or --task are executed
	}
	if (options.test(EXCISE_TEMPLATE)) {
		exciseCount += m_pAgentSoar->num_productions_of_type[TEMPLATE_PRODUCTION_TYPE];

		excise_all_productions_of_type(m_pAgentSoar, TEMPLATE_PRODUCTION_TYPE, false);
	}
	if (options.test(EXCISE_USER)) {
		exciseCount += m_pAgentSoar->num_productions_of_type[USER_PRODUCTION_TYPE];

		excise_all_productions_of_type(m_pAgentSoar, USER_PRODUCTION_TYPE, false);
	}

	// Excise specific production
	if (pProduction) 
	{
		Symbol* sym = find_sym_constant( m_pAgentSoar, pProduction->c_str() );

		if (!sym || !(sym->sc.production))
		{
			return SetError(CLIError::kProductionNotFound);
		}
		
		if (!m_RawOutput) 
		{
			// Save the name for the structured response
			AppendArgTagFast( sml_Names::kParamName, sml_Names::kTypeString, *pProduction );
		}

		// Increment the count for the structured response
		++exciseCount;	

		excise_production(m_pAgentSoar, sym->sc.production, false);
	}

	if (m_RawOutput) {
		m_Result << "\n" << exciseCount << " production" << (exciseCount == 1 ? " " : "s ") << "excised.";
	} else {
		// Add the count tag to the front
		std::string temp;
		PrependArgTag(sml_Names::kParamCount, sml_Names::kTypeInt, to_string(exciseCount, temp));
	}

	return true;
}
