#ifndef CLI_CLIERROR_H
#define CLI_CLIERROR_H

#include <string>

namespace cli {

	class CLIError {
	public:

		enum {
			kNoError,
			kCustomError,
			kgSKIError,
			kGetOptError,

			kCommandNotFound,
			kNoCommandPointer,
			kNotImplemented,
			kOptionNotImplemented,

			kExtraClosingParen,
			kUnmatchedBracketOrQuote,
			kExtraClosingBrace,
			kUnmatchedBrace,

			kgetcwdFail,
			kgettimeofdayFail,
			kchdirFail,

			kAgentRequired,
			kKernelRequired,

			kAliasNotFound,
			kAliasExists,
			kAliasError,

			kNoUsageInfo,
			kNoUsageFile,

			kTooManyArgs,
			kTooFewArgs,
			kUnrecognizedOption,
			kMissingOptionArg,

			kIntegerExpected,
			kIntegerMustBePositive,
			kIntegerMustBeNonNegative,
			kIntegerOutOfRange,

			kInvalidOperation,
			kInvalidWMEDetail,
			kInvalidNumericIndifferentMode,
			kInvalidIndifferentSelectionMode,
			kInvalidProductionType,

			kNoProdTypeWhenProdName,
			kSourceOnlyOneFile,
			kProductionNotFound,

			kLogAlreadyOpen,
			kLogOpenFailure,
			kLogNotOpen,

			kDirectoryOpenFailure,
			kDirectoryEntryReadFailure,
			kDirectoryStackEmpty,
			kMissingFilenameArg,
			kOpenFileFail,

			kCantSaveReteWithJustifications,
			kCantLoadReteWithProductions,
			kReteSaveOperationFail,
			kReteLoadOperationFail,
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
