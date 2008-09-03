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

	ExciseBitset options(0);

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "acdtu", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'a':
				options.set(EXCISE_ALL);
				break;
			case 'c':
				options.set(EXCISE_CHUNKS);
				break;
			case 'd':
				options.set(EXCISE_DEFAULT);
				break;
			case 't':
				options.set(EXCISE_TASK);
				break;
			case 'u':
				options.set(EXCISE_USER);
				break;
			case '?':
				{
					std::string detail;
					if (m_pGetOpt->GetOptOpt()) {
						detail = static_cast<char>(m_pGetOpt->GetOptOpt());
					} else {
						detail = argv[m_pGetOpt->GetOptind() - 1];
					}
					SetErrorDetail("Bad option '" + detail + "'.");
				}
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// If there are options, no additional argument.
	if (options.any()) {
		if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kTooManyArgs);
		return DoExcise(pAgent, options);
	}

	// If there are no options, there must be only one production name argument
	if (m_pGetOpt->GetAdditionalArgCount() < 1) {
		SetErrorDetail("Production name is required.");
		return SetError(CLIError::kTooFewArgs);		
	}
	if (m_pGetOpt->GetAdditionalArgCount() > 1) {
		SetErrorDetail("Only one production name allowed, call excise multiple times to excise more than one specific production.");
		return SetError(CLIError::kTooManyArgs);		
	}

	// Pass the production to the DoExcise function
	return DoExcise(pAgent, options, &(argv[m_pGetOpt->GetOptind()]));
}

bool CommandLineInterface::DoExcise(gSKI::IAgent* pAgent, const ExciseBitset& options, const std::string* pProduction) {
	if (!RequireAgent(pAgent)) return false;

	// Acquire production manager
	gSKI::IProductionManager *pProductionManager = pAgent->GetProductionManager();
	if (!pProductionManager) {
		SetErrorDetail("Failed to get production manager.");
		return SetError(CLIError::kgSKIError);
	}

	int exciseCount = 0;

	// Process the general options
	if (options.test(EXCISE_ALL)) {
		ExciseInternal(pProductionManager->GetAllProductions(), exciseCount);
		if (exciseCount) this->DoInitSoar(pAgent);	// from the manual, init when --all or --task are executed
	}
	if (options.test(EXCISE_CHUNKS)) {
		ExciseInternal(pProductionManager->GetChunks(), exciseCount);
		ExciseInternal(pProductionManager->GetJustifications(), exciseCount);
	}
	if (options.test(EXCISE_DEFAULT)) {
		ExciseInternal(pProductionManager->GetDefaultProductions(), exciseCount);
	}
	if (options.test(EXCISE_TASK)) {
		ExciseInternal(pProductionManager->GetChunks(), exciseCount);
		ExciseInternal(pProductionManager->GetJustifications(), exciseCount);
		ExciseInternal(pProductionManager->GetUserProductions(), exciseCount);
		if (exciseCount) this->DoInitSoar(pAgent);	// from the manual, init when --all or --task are executed
	}
	if (options.test(EXCISE_USER)) {
		ExciseInternal(pProductionManager->GetUserProductions(), exciseCount);
	}

	// Excise specific production
	if (pProduction) {
		// Check for the production
		gSKI::tIProductionIterator* pProdIter = pProductionManager->GetProduction((*pProduction).c_str());
		if (!pProdIter->GetNumElements()) {
			SetErrorDetail("Production: " + *pProduction);
			return SetError(CLIError::kProductionNotFound);
		}

		ExciseInternal(pProdIter, exciseCount);
	}

	if (m_RawOutput) {
		m_Result << "\n" << exciseCount << " production" << (exciseCount == 1 ? " " : "s ") << "excised.";
	} else {
		// Add the count tag to the front
		char buf[kMinBufferSize];
		PrependArgTag(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(exciseCount, buf, kMinBufferSize));
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

