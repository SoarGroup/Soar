#ifndef CLI_CLIERROR_H
#define CLI_CLIERROR_H

#include <string>

namespace cli {

	class CLIError {
	public:

		enum {
			kNoError = 0,
			kCustomError = 1,
			kgSKIError = 2,
			kGetOptError = 3,

			kCommandNotFound = 4,
			kProductionNotFound = 5,
			kMultiAttributeNotFound = 6,
			kNoCommandPointer = 7,
			kNotImplemented = 8,
			kOptionNotImplemented = 9,

			kExtraClosingParen = 10,
			kUnmatchedBracketOrQuote = 11,
			kExtraClosingBrace = 12,
			kUnmatchedBrace = 13,

			kTooManyArgs = 14,
			kTooFewArgs = 15,
			kUnrecognizedOption = 16,
			kMissingOptionArg = 17,

			kgetcwdFail = 18,
			kgettimeofdayFail = 19,
			kchdirFail = 20,

			kAgentRequired = 21,
			kKernelRequired = 22,

			kAliasNotFound = 23,
			kAliasExists = 24,
			//kAliasError = 25,

			kNoUsageInfo = 26,
			kNoUsageFile = 27,

			kIntegerExpected = 28,
			kIntegerMustBePositive = 29,
			kIntegerMustBeNonNegative = 30,
			kIntegerOutOfRange = 31,

			kInvalidOperation = 32,
			kInvalidWMEDetail = 33,
			kInvalidNumericIndifferentMode = 34,
			kInvalidIndifferentSelectionMode = 35,
			kInvalidProductionType = 36,

			kNoProdTypeWhenProdName = 37,
			kSourceOnlyOneFile = 38,

			kLogAlreadyOpen = 39,
			kLogOpenFailure = 40,
			kLogNotOpen = 41,

			kDirectoryOpenFailure = 42,
			kDirectoryEntryReadFailure = 43,
			kDirectoryStackEmpty = 44,
			kMissingFilenameArg = 45,
			kOpenFileFail = 46,

			kCantSaveReteWithJustifications = 47,
			kCantLoadReteWithProductions = 48,
			kReteSaveOperationFail = 49,
			kReteLoadOperationFail = 50,

			kInvalidProduction = 51,
			kInvalidLearnSetting = 52,
			kRemoveOrZeroExpected = 53,

			kInvalidID = 54,
			kInvalidAttribute = 55,
			kInvalidValue = 56,

			kRemoveWMEFailed = 57,

			kInvalidOSupportMode = 58,

			kInvalidWMEFilterType = 59,
			kFilterExpected = 60,
			kDuplicateWMEFilter = 61, 
			kInvalidMode = 62,
			kTypeRequired = 63,
			kWMEFilterNotFound = 64,

			kProductionRequired = 65,
			kInvalidConditionNumber = 66,

			kInvalidPrefix = 67,
			kCountGreaterThanMaxChunks = 68,
			kCountLessThanChunks = 69,
		};

		CLIError() : m_Code(0) {}

		bool SetError(int code);
		bool SetError(const std::string& description);

		int GetErrorCode();

		char const* GetErrorDescription();

	protected:

		int m_Code;
		std::string m_Description;
	};

} // namespace cli


#endif // CLI_CLIERROR_H
