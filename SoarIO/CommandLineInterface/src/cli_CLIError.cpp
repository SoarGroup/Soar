#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CLIError.h"

using namespace cli;

int CLIError::GetErrorCode() {
	return m_Code;
}

char const* CLIError::GetErrorDescription() {
	switch (m_Code) {
		case kNoError:							return "No Error.";
		case kCustomError:						return m_Description.c_str();
		case kgSKIError:						return "gSKI error.";
		case kGetOptError:						return "GetOpt returned with an error.";

		case kCommandNotFound:					return "Command not found.";
		case kProductionNotFound:				return "Production not found.";
		case kMultiAttributeNotFound:			return "Multi-attribute not found.";
		case kNoCommandPointer:					return "Command found but function pointer is NULL.";
		case kNotImplemented:					return "Not implemented.";
		case kOptionNotImplemented:				return "Option not implemented.";

		case kExtraClosingParen:				return "Closing bracket found without opening counterpart.";
		case kUnmatchedBracketOrQuote:			return "No closing quotes/brackets found.";
		case kExtraClosingBrace:				return "Closing brace found without opening counterpart.";
		case kUnmatchedBrace:					return "Unexpected end of file. Unmatched opening brace.";

		case kTooManyArgs:						return "Too many arguments for the specified (or unspecified) options, check syntax.";
		case kTooFewArgs:						return "Too few arguments for the specified (or unspecified) options, check syntax.";
		case kUnrecognizedOption:				return "Unrecognized option.";
		case kMissingOptionArg:					return "Missing option argument.";

		case kgetcwdFail:						return "Error getting current working directory.";
		case kgettimeofdayFail:					return "gettimeofday() failed.";
		case kchdirFail:						return "Error changing to directory.";

		case kAgentRequired:					return "An agent is required for this command.";
		case kKernelRequired:					return "A kernel is required for this command.";

		case kAliasNotFound:					return "Alias not found.";
		case kAliasExists:						return "Alias exists, remove to overwrite.";
		//case kAliasError:						return "Error adding alias.";

		case kNoUsageInfo:						return "Usage information not found for the command.";
		case kNoUsageFile:						return "Usage file not available (error opening file).";

		case kIntegerExpected:					return "Integer argument expected.";
		case kIntegerMustBePositive:			return "Integer argument must be positive.";
		case kIntegerMustBeNonNegative:			return "Integer argument must be non-negative.";
		case kIntegerOutOfRange:				return "Integer argument out of range.";

		case kInvalidOperation:					return "Invalid operation.";
		case kInvalidWMEDetail:					return "Invalid WME detail level.";
		case kInvalidNumericIndifferentMode:	return "Invalid numeric indifferent mode.";
		case kInvalidIndifferentSelectionMode:	return "Invalid indifferent selection mode.";
		case kInvalidProductionType:			return "Invalid production type.";

		case kNoProdTypeWhenProdName:			return "Do not specify production type when specifying a production name.";
		case kSourceOnlyOneFile:				return "Source only one file at a time.";

		case kLogAlreadyOpen:					return "Log already open.";
		case kLogOpenFailure:					return "Failed to open file for logging.";
		case kLogNotOpen:						return "Log is not open.";

		case kDirectoryOpenFailure:				return "Unable to open directory for reading.";
		case kDirectoryEntryReadFailure:		return "Unable to read directory entry for reading.";
		case kDirectoryStackEmpty:				return "Directory stack empty, no directory to change to.";
		case kMissingFilenameArg:				return "Missing filename argument.";
		case kOpenFileFail:						return "Failed to open file for reading.";

		case kCantSaveReteWithJustifications:	return "Can't save rete while justifications are present.";
		case kCantLoadReteWithProductions:		return "Can't load rete unless production memory is empty.";
		case kReteSaveOperationFail:			return "Rete save operation failed.";
		case kReteLoadOperationFail:			return "Rete load operation failed.";

		case kInvalidProduction:				return "Invalid production.";
		case kInvalidLearnSetting:				return "Invalid learn setting, expected noprint, print, fullprint, or 0-2.";
		case kRemoveOrZeroExpected:				return "Invalid argument, expected remove or 0.";

		case kInvalidID:						return "Unknown or invalid ID.";
		case kInvalidAttribute:					return "Unknown or invalid attribute.";
		case kInvalidValue:						return "Unknown or invalid value.";

		case kRemoveWMEFailed:					return "Remove WME failed.";

		case kInvalidOSupportMode:				return "Invalid O-Support mode, use 0-4.";

		case kInvalidWMEFilterType:				return "Invalid WME filter type, expected 'adds' 'removes' or 'both'.";
		case kFilterExpected:					return "ID/Attribute/Value filter expected, one or more missing.";
		case kDuplicateWMEFilter:				return "That WME filter already exists.";
		case kInvalidMode:						return "Invalid mode.";
		case kTypeRequired:						return "A type (-t adds/removes/both) is required for this command.";

		default:								return "Unknown error code.";
	}
}

bool CLIError::SetError(int code) { 
	m_Code = code; 
	m_Description.clear();
	if (code == kCustomError) m_Description = "Details not set.";
	return false;
}

bool CLIError::SetError(const std::string& description) { 
	m_Code = kCustomError; 
	m_Description = description; 
	return false;
}
