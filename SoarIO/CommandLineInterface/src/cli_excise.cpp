#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "sml_StringOps.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Production.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseExcise(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"all",		0, 0, 'a'},
		{"chunks",	0, 0, 'c'},
		{"default", 0, 0, 'd'},
		{"task",	0, 0, 't'},
		{"user",	0, 0, 'u'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	unsigned int options = 0;

	for (;;) {
		option = m_pGetOpt->GetOpt_Long(argv, "acdtu", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'a':
				options |= OPTION_EXCISE_ALL;
				break;
			case 'c':
				options |= OPTION_EXCISE_CHUNKS;
				break;
			case 'd':
				options |= OPTION_EXCISE_DEFAULT;
				break;
			case 't':
				options |= OPTION_EXCISE_TASK;
				break;
			case 'u':
				options |= OPTION_EXCISE_USER;
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLIExcise, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// If there are options, no additional argument.
	if (options) {
		if (argv.size() != (unsigned)GetOpt::optind) {
			return HandleSyntaxError(Constants::kCLIExcise, Constants::kCLITooManyArgs);
		}
		return DoExcise(pAgent, options);
	}

	// If there are no options, there must be only one production name argument
	if ((argv.size() - GetOpt::optind) != 1) {
		return HandleSyntaxError(Constants::kCLIExcise);		
	}

	// Pass the productions to the DoExcise function
	return DoExcise(pAgent, options, &(argv[GetOpt::optind]));
}

bool CommandLineInterface::DoExcise(gSKI::IAgent* pAgent, const unsigned int options, std::string* pProduction) {
	if (!RequireAgent(pAgent)) return false;

	// Acquire production manager
	gSKI::IProductionManager *pProductionManager = pAgent->GetProductionManager();
	if (!pProductionManager) {
		return HandleError("Unable to get production manager!");
	}

	int exciseCount = 0;

	// Process the general options
	if (options & OPTION_EXCISE_ALL) {
		ExciseInternal(pProductionManager->GetAllProductions(), exciseCount);
		this->DoInitSoar(pAgent);	// from the manual, init when --all or --task are executed
	}
	if (options & OPTION_EXCISE_CHUNKS) {
		ExciseInternal(pProductionManager->GetChunks(), exciseCount);
		ExciseInternal(pProductionManager->GetJustifications(), exciseCount);
	}
	if (options & OPTION_EXCISE_DEFAULT) {
		ExciseInternal(pProductionManager->GetDefaultProductions(), exciseCount);
	}
	if (options & OPTION_EXCISE_TASK) {
		ExciseInternal(pProductionManager->GetChunks(), exciseCount);
		ExciseInternal(pProductionManager->GetJustifications(), exciseCount);
		ExciseInternal(pProductionManager->GetUserProductions(), exciseCount);
		this->DoInitSoar(pAgent);	// from the manual, init when --all or --task are executed
	}
	if (options & OPTION_EXCISE_USER) {
		ExciseInternal(pProductionManager->GetUserProductions(), exciseCount);
	}

	// Excise specific production
	if (pProduction) {
		// Check for the production
		gSKI::tIProductionIterator* pProdIter = pProductionManager->GetProduction((*pProduction).c_str());
		if (!pProdIter->GetNumElements()) {
			return HandleError("Production not found: " + (*pProduction));
		}

		ExciseInternal(pProdIter, exciseCount);
	}

	if (!m_RawOutput) {
		// Add the count tag to the front
		char buffer[kMinBufferSize];
		PrependArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(exciseCount, buffer, sizeof(buffer)));
	}

	return true;
}

void CommandLineInterface::ExciseInternal(gSKI::tIProductionIterator *pProdIter, int& exciseCount) {
	// Iterate through the productions using the production iterator and
	// excise and release.
	while(pProdIter->IsValid()) {
		gSKI::IProduction* ip = pProdIter->GetVal();

		if (!m_RawOutput) {
			// Save the name for the structured response
			AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, ip->GetName());
		}

		// Increment the count for the structured response
		++exciseCount;	// Don't *need* to outside of if above but why not.

		ip->Excise();
		ip->Release();
		pProdIter->Next();
	}
	pProdIter->Release();

}

