//////////////////////////////////////////////////////////////////////////////////////////////////
// exploration command file
//////////////////////////////////////////////////////////////////////////////////////////////////
// #ifdef NUMERIC_INDIFFERENCE

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseExploration(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'m',"mode",					1},
		{'t',"temperature",             1},
		{'e',"epsilon",                 1},
		{0, 0, 0}
	};
			 
 	int modeSetting = 0;
	double Temp = -1;
	double epsilon = -1;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

	switch (m_Option) {
	case 'm'://mode
		modeSetting = ParseExplorationOptarg();
		if (modeSetting == -1) return false;
		break;
	case 't'://temperature
		if (m_OptionArgument.size()) {
			Temp = strtod(m_OptionArgument.c_str(),0);
			if (Temp <= 0) return SetError(CLIError::kTempMustBePositive);
		}
		break;
	case 'e'://epsilon
		if (m_OptionArgument.size()) {
			epsilon = strtod(m_OptionArgument.c_str(),0);
			if ((epsilon < 0) || (epsilon > 1)) return SetError(CLIError::kEpsilonOutsideUnitInterval);
		}
		break;
		
	default:
		return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoExploration(pAgent, modeSetting, Temp, epsilon);
}


int CommandLineInterface::ParseExplorationOptarg() {
	if (m_OptionArgument == "Boltzmann"   || m_OptionArgument == "0") return 1;
	if (m_OptionArgument == "epsilon-greedy"     || m_OptionArgument == "1") return 2;
	if (m_OptionArgument == "no-exploration" || m_OptionArgument == "2") return 3;

	SetErrorDetail("Got: " + m_OptionArgument);
	SetError(CLIError::kInvalidExplorationSetting);
	return -1;
}

bool CommandLineInterface::DoExploration(gSKI::IAgent* pAgent, const int modeSetting, const double Temp, const double epsilon) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	
	if (Temp > 0){
		pAgent->SetExplorationParameter(TEMPERATURE, Temp);
		return true;
	} else if (epsilon >= 0){
		pAgent->SetExplorationParameter(EPSILON, epsilon);
		return true;
	} else {
	
		switch (modeSetting) {
			case 0:
				break;
			case BOLTZMANN_EXPLORATION:
				pAgent->SetExplorationMode(gSKI_BOLTZMANN_EXPLORATION);
				return true;
			case EPSILON_GREEDY_EXPLORATION:
				pAgent->SetExplorationMode(gSKI_EPSILON_GREEDY_EXPLORATION);
				return true;
			case NO_EXPLORATION:
				pAgent->SetExplorationMode(gSKI_NO_EXPLORATION);
				return true;
			default:
				return SetError(CLIError::kInvalidExplorationSetting);
		}
	
	if (m_RawOutput) {
		m_Result << "Current exploration mode: ";
		switch (pAgent->GetExplorationMode()) {
			case gSKI_BOLTZMANN_EXPLORATION:
				m_Result << "Boltzmann,  Temperature: " << pAgent->GetExplorationParameter(TEMPERATURE);
				break;
			case gSKI_EPSILON_GREEDY_EXPLORATION:
				m_Result << "epsilon-greedy,  epsilon: " << pAgent->GetExplorationParameter(EPSILON);
				break;
			case gSKI_NO_EXPLORATION:
				m_Result << "No exploration.";
				break;
			default:
					return SetError(CLIError::kInvalidExplorationSetting);
		}
	}

	return true;
}
}
// #endif