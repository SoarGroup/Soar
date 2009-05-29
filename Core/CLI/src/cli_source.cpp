/////////////////////////////////////////////////////////////////
// source command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include <fstream>

#include "cli_Commands.h"
#include "sml_StringOps.h"
#include "sml_Names.h"
#include "sml_Events.h"
#include "cli_CLIError.h"
#include "misc.h"

#include <algorithm>

#include <assert.h>

using namespace cli;
using namespace sml;
using namespace std;

bool CommandLineInterface::ParseSource(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "all",			OPTARG_NONE},
		{'d', "disable",		OPTARG_NONE},
		{'v', "verbose",		OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	// Set to default on first call to source
	if (m_SourceDepth == 0) {
		m_SourceMode = SOURCE_DEFAULT;
		m_SourceVerbose = false; 
	}

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				// Only process this option on first call to source
				if (m_SourceDepth == 0) {
					m_SourceMode = SOURCE_DISABLE;
				}
				break;
			case 'a':
				// Only process this option on first call to source
				if (m_SourceDepth == 0) {
					m_SourceMode = SOURCE_ALL;
				}
				break;

			case 'v':
				m_SourceVerbose = true;
				break;

			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments < 1) {
		SetErrorDetail("Please supply one file to source. If there are spaces in the path, enclose it in quotes.");
		return SetError(CLIError::kTooFewArgs);

	} else if (m_NonOptionArguments > 2) {
		SetErrorDetail("Please supply one file to source. If there are spaces in the path, enclose it in quotes.");
		return SetError(CLIError::kSourceOnlyOneFile);
	}

	return DoSource(argv[argv.size() - 1]);
}

bool CommandLineInterface::DoSource(std::string filename) {
    StripQuotes(filename);

	// Separate the path out of the filename if any
	std::string path;

	// begin: rchong: 2006-05-05
	//
	// these two variables were defined as unsigned int.  this
	// worked on intel 32-bit processors, but broke on AMD 64-bit
	// processors.  they should be std::string::size_type as is
	// done in other source files.  also, the DoPushD(path)
	// was duplicated in each condition; i moved it outside.

	std::string::size_type separator1 = filename.rfind('/');
	std::string::size_type separator2 = filename.rfind('\\');
	std::string::size_type separator3 = std::string::npos;

	if ( separator1 != std::string::npos && separator2 != std::string::npos )
	{
		separator3 = max( separator1, separator2 );
	} 
	else if ( separator1 == std::string::npos )
	{
		separator3 = separator2;
	}
	else if ( separator2 == std::string::npos )
	{
		separator3 = separator1;
	}

	if (separator3 != std::string::npos) {
		++separator3;
		if (separator3 < filename.length()) {
			path = filename.substr(0, separator3);
			filename = filename.substr(separator3, filename.length() - separator3);
		}
	}

	if (!DoPushD(path)) return false;

	// end: rchong: 2006-05-05

	// Open the file
	std::ifstream soarFile(filename.c_str());
	if (!soarFile) {
		if (path.length()) DoPopD();
		SetErrorDetail(filename);
		return SetError(CLIError::kOpenFileFail);
	}

	bool ret = StreamSource( soarFile, &filename );
	
	soarFile.close();
	if (path.length()) DoPopD();

	return ret;
}

bool CommandLineInterface::StreamSource( std::istream& soarStream, const std::string* pFilename ) {
	std::string line;				// Each line removed from the file
	std::string command;			// The command, sometimes spanning multiple lines
	std::string::size_type pos;		// Used to find braces on a line (triggering multiple line spanning commands)
	int braces = 0;					// Brace nest level (hopefully all braces are supposed to be closed)
	bool quote = false;				// Quotes can be used instead of braces
	bool pipe = false;
	std::string::iterator iter;		// Iterator when parsing for braces and pounds
	int lineCount = 0;				// Count the lines per file
	int lineCountCache = 0;			// Used to save a line number
	
	static int numTotalProductionsSourced;
	static int numTotalProductionsExcised;
	static int numTotalProductionsIgnored;

	if ( m_SourceDepth == 1 ) {				// Check for top-level source call
		m_SourceDirDepth = 0;				// Set directory depth to zero on first call to source, even though it should be zero anyway

		m_NumProductionsSourced = 0;		// set production number caches to zero on top level
		m_NumProductionsExcised = 0;
		m_NumProductionsIgnored = 0;
		numTotalProductionsSourced = 0;
		numTotalProductionsExcised = 0;
		numTotalProductionsIgnored = 0;

		// Register for production removed events so we can report the number of excised productions
		if ( m_pAgentSML ) // only do this if we have an agent
		{
			this->RegisterWithKernel(smlEVENT_BEFORE_PRODUCTION_REMOVED) ;
		}
	}
	++m_SourceDepth;

	if ( m_SourceDepth >= 100 )
	{
		SetError(CLIError::kSourceDepthExceeded);
		HandleSourceError(lineCount, pFilename);
		return false;
	}

	// Go through each line of the file (Yay! C++ file parsing!)
	while (getline(soarStream, line)) {
	
		// Increment line count
		++lineCount;

		// Clear out the old command
		command.clear();

		// Trim whitespace and comments
		TrimLeadingWhitespace(line);

		// bug 987: if it is echo, special handling is required
		bool isEcho = line.substr(0, 4) == "echo";

		if (!isEcho && !TrimComments(line)) {
			SetError(CLIError::kNewlineBeforePipe);
			HandleSourceError(lineCount, pFilename);
			return false;
		}

		if (!line.length()) continue; // Nothing on line, skip it

		// If there is a brace on the line, concatenate lines until the closing brace
		pos = line.find_first_of("{\"");

		if (!isEcho && pos != std::string::npos) {
			
			// Save this line number for error messages
			lineCountCache = lineCount;

			// While we are inside braces, stay in special parsing mode
			do {
				if (lineCountCache != lineCount) {
					TrimLeadingWhitespace(line);
					if (!TrimComments(line)) { // Trim whitespace and comments on additional lines
						SetError(CLIError::kNewlineBeforePipe);
						HandleSourceError(lineCount, pFilename);
						return false; 
					}
				}

				// nothing on line or just whitespace and comments
				if (!line.size()) continue;

				// Enter special parsing mode
				iter = line.begin();
				while (iter != line.end()) {
					// skip escaped characters
					if (*iter == '\\') {
						// skip escape character
						++iter;

						if (iter == line.end()) {
							// can't escape newlines
							SetError(CLIError::kEscapedNewline);
							HandleSourceError(lineCount, pFilename);
							return false; 
						}

					} else {
						// Go through each of the characters, counting brace nesting level
						if (pipe) {	// bug 968 fix
							if (*iter == '|') {
								pipe = false;
							}

						} else {

							if (*iter == '|') { // bug 968 fix
								pipe = true;
							} else if (*iter == '{') {
								++braces;
							} else if (*iter == '}') {
								--braces;
								
								// bug 968 fix
								if (braces < 0) {
									break;
								}

							} else if (*iter == '\"') {
								quote = !quote; // bug 967 fix
							}
						}
					}

					// Next character
					++iter;
				}
				
				// We finished that line, add it to the command
				command += line;

				assert( pipe == false ); // shouldn't be possible because it is handled in Trim()

				// Did we close all of the braces? and quotes?
				if (braces == 0 && quote == false) break; // Yes, break out of special parsing mode

				// Did we go negative?
				if (braces < 0) break; // Yes, break out on error

				// Put the newline back on it (getline eats the newline)
				command += '\n';

				// We're getting another line, increment count now
				++lineCount;

				// Get the next line from the file and repeat
			} while (getline(soarStream, line));

			// Did we break out because of closed braces or EOF?
			if (braces > 0) {
				// EOF while still nested
				SetError(CLIError::kUnmatchedBrace);
				HandleSourceError(lineCountCache, pFilename);
				return false;

			} else if (braces < 0) {
				SetError(CLIError::kExtraClosingBrace);
				HandleSourceError(lineCountCache, pFilename);
				return false;

			} else if (quote == true) { // bug 967 fix
				SetError(CLIError::kUnmatchedBracketOrQuote);
				HandleSourceError(lineCountCache, pFilename);
				return false;

			} else if (pipe == true) { // bug 968 fix
				SetError(CLIError::kNewlineBeforePipe);
				HandleSourceError(lineCountCache, pFilename);
				return false;
			}


			// We're good to go

		} else {
			// No braces on line, set command to line
			command = line;

			// Set cache to same line for error message
			lineCountCache = lineCount;
		}

		// Fire off the command
		size_t oldResultSize = m_Result.str().size();
		if (DoCommandInternal(command)) {
			// Add trailing newline if result changed size
			size_t newResultSize = m_Result.str().size();
			if (newResultSize > 0 && (oldResultSize != newResultSize)) {
				// but don't add after sp's
				if (m_Result.str()[m_Result.str().size()-1] != '*') {
					m_Result << '\n';
				}
			}

		} else {
			// Command failed, error in result
			HandleSourceError(lineCountCache, pFilename);
			return false;
		}	
	}

	// Completion
	--m_SourceDepth;

	// If mode ALL, print summary
	if (pFilename && m_SourceMode == SOURCE_ALL) {
		if (m_RawOutput) {
			if (m_NumProductionsSourced) m_Result << '\n';	// add a newline if a production was sourced
			m_Result << *pFilename << ": " << m_NumProductionsSourced << " production" << ((m_NumProductionsSourced == 1) ? " " : "s ") << "sourced.";
			if (m_NumProductionsExcised) {
				m_Result << " " << m_NumProductionsExcised << " production" << ((m_NumProductionsExcised == 1) ? " " : "s ") << "excised.";
				if (m_SourceVerbose) {
					// print excised production names
					m_Result << "\nExcised productions:";

					std::list< std::string >::iterator iter = m_ExcisedDuringSource.begin();
					while (iter != m_ExcisedDuringSource.end()) {
						m_Result << "\n\t" << (*iter);
						++iter;
					}
				}
			}
			if (m_NumProductionsIgnored) {
				m_Result << " " << m_NumProductionsIgnored << " production" << ((m_NumProductionsIgnored == 1) ? " " : "s ") << "ignored.";
			}

		} else {
			std::string temp;
			AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, *pFilename);
			AppendArgTag(sml_Names::kParamSourcedProductionCount, sml_Names::kTypeInt, to_string(m_NumProductionsSourced, temp));
			AppendArgTag(sml_Names::kParamExcisedProductionCount, sml_Names::kTypeInt, to_string(m_NumProductionsExcised, temp));
			AppendArgTag(sml_Names::kParamExcisedProductionCount, sml_Names::kTypeInt, to_string(m_NumProductionsIgnored, temp));

			std::list< std::string >::iterator iter = m_ExcisedDuringSource.begin();
			while (iter != m_ExcisedDuringSource.end()) {
				AppendArgTagFast( sml_Names::kParamName, sml_Names::kTypeString, *iter );
				++iter;
			}
		}

		if (m_ExcisedDuringSource.size()) m_ExcisedDuringSource.clear();
	}

	numTotalProductionsSourced += m_NumProductionsSourced;
	numTotalProductionsExcised += m_NumProductionsExcised;
	numTotalProductionsIgnored += m_NumProductionsIgnored;
	m_NumProductionsSourced = 0;	// set production number cache to zero after each summary
	m_NumProductionsExcised = 0;	// set production number cache to zero after each summary
	m_NumProductionsIgnored = 0;	// set production number cache to zero after each summary

	// if we're returning to the user
	if ( m_SourceDepth == 1 ) {

		// Remove production listener
		if ( m_pAgentSML ) // only do this if we have an agent
		{
			this->UnregisterWithKernel(smlEVENT_BEFORE_PRODUCTION_REMOVED) ;
		}

		if (m_RawOutput) {
			if (m_SourceMode != SOURCE_DISABLE) {
				if (numTotalProductionsSourced) m_Result << '\n';	// add a newline if a production was sourced
				// If default mode, print file name
				m_Result << "Total: " << numTotalProductionsSourced << " production" << ((numTotalProductionsSourced == 1) ? " " : "s ") << "sourced.";
				if (numTotalProductionsExcised) {
					m_Result << " " << numTotalProductionsExcised << " production" << ((numTotalProductionsExcised == 1) ? " " : "s ") << "excised.";

					if (m_SourceVerbose && (m_SourceMode != SOURCE_ALL)) {
						// print excised production names
						m_Result << "\nExcised productions:";

						std::list< std::string >::iterator iter = m_ExcisedDuringSource.begin();
						while (iter != m_ExcisedDuringSource.end()) {
							m_Result << "\n\t" << (*iter);
							++iter;
						}
					}
				}
				if (numTotalProductionsIgnored) {
					m_Result << " " << numTotalProductionsIgnored << " production" << ((numTotalProductionsIgnored == 1) ? " " : "s ") << "ignored.";
				}
			}

		} else {
			if (m_SourceMode != SOURCE_DISABLE) {
				std::string temp;
				AppendArgTag(sml_Names::kParamSourcedProductionCount, sml_Names::kTypeInt, to_string(numTotalProductionsSourced, temp));
				AppendArgTag(sml_Names::kParamExcisedProductionCount, sml_Names::kTypeInt, to_string(numTotalProductionsExcised, temp));
				AppendArgTag(sml_Names::kParamExcisedProductionCount, sml_Names::kTypeInt, to_string(numTotalProductionsIgnored, temp));

				if (m_SourceVerbose) {
					std::list< std::string >::iterator iter = m_ExcisedDuringSource.begin();
					while (iter != m_ExcisedDuringSource.end()) {
						AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, *iter );
						++iter;
					}
				}
			}
		}

		if (m_ExcisedDuringSource.size()) m_ExcisedDuringSource.clear();

		// Print working directory if source directory depth !=  0
		if (m_SourceDirDepth != 0) DoPWD();	// Ignore error
		m_SourceDirDepth = 0;

		// Add finished message
		if (m_RawOutput) {
			if (m_Result.str().size() && m_Result.str()[m_Result.str().size()-1] != '\n') m_Result << '\n';	// add a newline if none present
			m_Result << "Source finished.";
		}
	}

	return true;
}

void CommandLineInterface::HandleSourceError( int errorLine, const std::string* pFilename ) {
	if ( !m_SourceError && m_SourceDepth > 1 ) { // only do this when we're actually dealing with a source error

		// Remove listener
		if ( m_pAgentSML ) // only do this if we have an agent
		{
			this->UnregisterWithKernel(smlEVENT_BEFORE_PRODUCTION_REMOVED) ;
		}

		// Flush excised production list
		if (m_ExcisedDuringSource.size()) m_ExcisedDuringSource.clear();

		// Output error message
		m_SourceErrorDetail.clear();
		m_SourceErrorDetail += "\nSource command error on (or near) line ";

		std::string temp;
		m_SourceErrorDetail += to_string(errorLine, temp);

		if ( pFilename )
		{
			m_SourceErrorDetail += " of ";

			std::string directory;
			GetCurrentWorkingDirectory(directory); // Again, ignore error here

			m_SourceErrorDetail += *pFilename + " (" + directory + ")";
		}

		// PopD to original source directory
		while (m_SourceDirDepth) {
			if (m_SourceDirDepth < 0) m_SourceDirDepth = 0; // don't loop forever
			DoPopD(); // Ignore error here since it will be rare and a message confusing
		}

		m_SourceError = true;

	} else if ( pFilename ) {
		std::string temp;
		m_SourceErrorDetail += "\n\t--> Sourced by: " + *pFilename + " (line " + to_string(errorLine, temp) + ")";
	}

	// Reset depth to zero
	m_SourceDepth = 0;
}
