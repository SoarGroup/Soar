// #ifdef NUMERIC_INDIFFERENCE
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"

 

#include "cli_Commands.h"

#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseRL(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'e', "enable",		0},
		{'e',"on",			0},
		{'d', "disable",	0},
		{'d',"off",			0},
		{'n', "on-policy",  0},
		{'f', "off-policy", 0}, 
		{'a',"alpha",       1},
		{'g',"gamma",		1},
		{'l',"lambda",		1},
		{0, 0, 0}
	};

	int RLSetting = 0;
	int algSetting = 0;
	double alpha = -1;
	double gamma = -1;
	double lambda = -1;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

	switch (m_Option) {
	case 'e'://on
		RLSetting = 1;
		break;
	case 'd'://off
		RLSetting = 2;
		break;
	case 'n'://on-policy
		algSetting = 1;
		break;
	case 'f'://off-policy
		algSetting = 2;
		break;
	case 'a'://alpha
		if (m_OptionArgument.size()) {
			alpha = strtod(m_OptionArgument.c_str(),0);
			if (alpha < 0) return SetError(CLIError::kAlphaMustBeNonNegative);
		}
		break;
	case 'g'://gamma
		if (m_OptionArgument.size()) {
			gamma = strtod(m_OptionArgument.c_str(),0);
			if ((gamma < 0) || (gamma > 1)) return SetError(CLIError::kParameterOutsideUnitInterval);
		}
		break;
	case 'l'://lambda
		if (m_OptionArgument.size()) {
			lambda = strtod(m_OptionArgument.c_str(),0);
			if ((lambda < 0) || (lambda > 1)) return SetError(CLIError::kParameterOutsideUnitInterval);
		}
		break;		
	default:
		return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoRL(pAgent, RLSetting, algSetting, alpha, gamma, lambda);
}

bool CommandLineInterface::DoRL(gSKI::IAgent* pAgent, const int RLSetting, const int algSetting, const double alpha, const double gamma, const double lambda) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	
	if (alpha >= 0){
		pAgent->SetRLParameter(ALPHA, alpha);
		return true;
	} 
	if (gamma >= 0){
		pAgent->SetRLParameter(GAMMA, gamma);
		return true;
	}
	if (lambda >= 0){
		if (!pKernelHack->GetSysparam(pAgent, RL_ONPOLICY_SYSPARAM) && (lambda > 0)) return SetError(CLIError::kETRequiresOnPolicy);
		pAgent->SetRLParameter(LAMBDA, lambda);
		return true;
	}
	if (RLSetting == 1) {
		pKernelHack->SetSysparam(pAgent, RL_ON_SYSPARAM, true);
		return true;
	}
	if (RLSetting == 2) {
		pKernelHack->SetSysparam(pAgent, RL_ON_SYSPARAM, false);
		pKernelHack->ResetRL(pAgent);
		return true;
	}
	if (algSetting == 1) {
		pKernelHack->SetSysparam(pAgent, RL_ONPOLICY_SYSPARAM, true);
		return true;
	}
	if (algSetting == 2) {
		if (pAgent->GetRLParameter(LAMBDA) > 0) return SetError(CLIError::kOffPolicyDisallowsET);
		pKernelHack->SetSysparam(pAgent, RL_ONPOLICY_SYSPARAM, false);
		return true;
	}	
	if (m_RawOutput) {
			m_Result << "Current RL settings: ";
			if (pKernelHack->GetSysparam(pAgent, RL_ON_SYSPARAM)){
				m_Result << "Reinforcement learning is enabled. \n";
				m_Result << "Learning is " << (pKernelHack->GetSysparam(pAgent, RL_ONPOLICY_SYSPARAM) ? "on-policy. \n" : "off-policy. \n"); 
				m_Result << "alpha: " << pAgent->GetRLParameter(ALPHA);
				m_Result << " gamma: " << pAgent->GetRLParameter(GAMMA);
				m_Result << " lambda: " << pAgent->GetRLParameter(LAMBDA);
			} else {
				m_Result << "Reinforcement learning is disabled."; 
			}
		}
	return true;	
}

// #endif
