#include "cli_CommandLineInterface.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_MultiAttribute.h"
#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

// ____                     __  __       _ _   _    _   _   _        _ _           _
//|  _ \ __ _ _ __ ___  ___|  \/  |_   _| | |_(_)  / \ | |_| |_ _ __(_) |__  _   _| |_ ___  ___
//| |_) / _` | '__/ __|/ _ \ |\/| | | | | | __| | / _ \| __| __| '__| | '_ \| | | | __/ _ \/ __|
//|  __/ (_| | |  \__ \  __/ |  | | |_| | | |_| |/ ___ \ |_| |_| |  | | |_) | |_| | ||  __/\__ \
//|_|   \__,_|_|  |___/\___|_|  |_|\__,_|_|\__|_/_/   \_\__|\__|_|  |_|_.__/ \__,_|\__\___||___/
//
bool CommandLineInterface::ParseMultiAttributes(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// No more than three arguments
	if (argv.size() > 3) {
		return HandleSyntaxError(Constants::kCLIMultiAttributes, Constants::kCLITooManyArgs);
	}

	int n = 0;

	// If we have 3 arguments, third one is an integer
	if (argv.size() > 2) {
		if (!IsInteger(argv[2])) {
			// Must be an integer
			return HandleSyntaxError(Constants::kCLIMultiAttributes, "Third argument must be an integer.");
		}
		n = atoi(argv[2].c_str());
		if (n <= 0) {
			// Must be greater than 0
			return HandleSyntaxError(Constants::kCLIMultiAttributes, "Third argument must be greater than 0.");
		}
	}

	// If we have two arguments, second arg is an attribute/identifer/whatever
	if (argv.size() > 1) {
		return DoMultiAttributes(pAgent, &(argv[1]), n);
	} 
	return DoMultiAttributes(pAgent, 0, n);
}

// ____        __  __       _ _   _    _   _   _        _ _           _
//|  _ \  ___ |  \/  |_   _| | |_(_)  / \ | |_| |_ _ __(_) |__  _   _| |_ ___  ___
//| | | |/ _ \| |\/| | | | | | __| | / _ \| __| __| '__| | '_ \| | | | __/ _ \/ __|
//| |_| | (_) | |  | | |_| | | |_| |/ ___ \ |_| |_| |  | | |_) | |_| | ||  __/\__ \
//|____/ \___/|_|  |_|\__,_|_|\__|_/_/   \_\__|\__|_|  |_|_.__/ \__,_|\__\___||___/
//
bool CommandLineInterface::DoMultiAttributes(gSKI::IAgent* pAgent, std::string* pAttribute, int n) {
	if (!RequireAgent(pAgent)) return false;

	if (!pAttribute && !n) {
		// No args, print current setting
		int count = 0;
		
		// Arbitrary buffer size
		char buf[kMinBufferSize];

		gSKI::tIMultiAttributeIterator* pIt = pAgent->GetMultiAttributes();
		if (!pIt->GetNumElements()) {
			if (m_RawOutput) {
				m_Result += "No multi-attributes declared for this agent.";
			}

		} else {
			if (m_RawOutput) {
				m_Result += "Value\tSymbol";
			}

			gSKI::IMultiAttribute* pMA;


			for(; pIt->IsValid(); pIt->Next()) {
				pMA = pIt->GetVal();

				if (m_RawOutput) {
					m_Result += "\n";
					m_Result += Int2String(pMA->GetMatchingPriority(), buf, kMinBufferSize);
					m_Result += "\t";
					m_Result += pMA->GetAttributeName();
				} else {
					// Value
					AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(count, buf, sizeof(buf)));
					// Symbol
					AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, pMA->GetAttributeName());
				}

				++count;
				pMA->Release();
			}

		}
		pIt->Release();

		if (!m_RawOutput) {
			// Add the count tag to the front
			PrependArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(count, buf, sizeof(buf)));
		}
		return true;
	}

	// TODO: Check whether attribute is a valid symbolic constant. The way the old kernel
	// does this is to call get_lexeme_from_string(m_agent, argv[1]) and check that
	// the lex type is symbolic constant. At this point, that functionality isn't
	// exposed by gSKI...

	// Setting defaults to 10
	if (!n) {
		n = 10;
	}

	// Set it
	pAgent->SetMultiAttribute(pAttribute->c_str(), n);
 	return true;
}

