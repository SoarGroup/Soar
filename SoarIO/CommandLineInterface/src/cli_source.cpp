#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include <fstream>

#include "cli_Constants.h"

#ifdef _MSC_VER
#define snprintf _snprintf 
#endif // _MSC_VER

using namespace cli;

bool CommandLineInterface::ParseSource(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	if (argv.size() < 2) {
		// Source requires a filename
		return HandleSyntaxError(Constants::kCLISource, Constants::kCLITooFewArgs);

	} else if (argv.size() > 2) {
		// but only one filename
		return HandleSyntaxError(Constants::kCLISource, "Source only one file at a time.");
	}

	return DoSource(pAgent, argv[1]);
}

bool CommandLineInterface::DoSource(gSKI::IAgent* pAgent, std::string filename) {
	if (!RequireAgent(pAgent)) return false;

	// Chop of quotes if they are there, open doesn't like them
	if ((filename.length() > 2) && (filename[0] == '\"') && (filename[filename.length() - 1] == '\"')) {
		filename = filename.substr(1, filename.length() - 2);
	}

	// Open the file
	std::ifstream soarFile(filename.c_str());
	if (!soarFile) {
		return HandleError("Failed to open file '" + filename + "' for reading.");
	}

	std::string line;					// Each line removed from the file
	std::string command;					// The command, sometimes spanning multiple lines
	std::string::size_type pos;			// Used to find braces on a line (triggering multiple line spanning commands)
	int braces = 0;					// Brace nest level (hopefully all braces are supposed to be closed)
	std::string::iterator iter;			// Iterator when parsing for braces and pounds
	int lineCount = 0;				// Count the lines per file
	int lineCountCache = 0;			// Used to save a line number

	// Set dir depth to zero on first call to source, even though it should be zero anyway
	if (m_SourceDepth == 0) {
		m_SourceDirDepth = 0;
	}
	++m_SourceDepth;

	// Go through each line of the file (Yay! C++ file parsing!)
	while (getline(soarFile, line)) {
	
		// Increment line count
		++lineCount;

		// Clear out the old command
		command.clear();

		// Remove leading whitespace
		iter = line.begin();
		while (isspace(*iter)) {
			line.erase(iter);

			if (!line.length()) {
				// Nothing but space left, next line
				continue;
			}
			
			// Next character
			iter = line.begin();
		}

		// Was it actually trailing whitespace?
		if (!line.length()) {
			// Nothing left to do
			continue;
		}

		// Is the first character a comment?
		if (*iter == '#') {
			// Yes, ignore
			continue;
		}

		// If there is a brace on the line, concatenate lines until the closing brace
		pos = line.find('{');

		if (pos != std::string::npos) {
			
			// Save this line number for error messages
			lineCountCache = lineCount;

			// While we are inside braces, stay in special parsing mode
			do {
				// Enter special parsing mode
				iter = line.begin();
				while (iter != line.end()) {
					// Go through each of the characters, counting brace nesting level
					if (*iter == '{') {
						++braces;
					}
					if (*iter == '}') {
						--braces;
					}
					// Next character
					++iter;
				}

				// We finished that line, add it to the command, and put the newline back on it (getline eats the newline)
				command += line + '\n';

				// Did we close all of the braces?
				if (!braces) {
					// Yes, break out of special parsing mode
					break;
				}

				// Did we go negative?
				if (braces < 0) {
					// Yes, break out on error
					break;
				}

				// We're getting another line, increment count now
				++lineCount;

				// Get the next line from the file and repeat
			} while (getline(soarFile, line));

			// Did we break out because of closed braces or EOF?
			if (braces > 0) {
				// EOF while still nested
				HandleError("Unexpected end of file. Unmatched opening brace.");
				HandleSourceError(lineCountCache, filename);
				return false;

			} else if (braces < 0) {
				HandleError("Closing brace(s) found without matching opening brace.");
				HandleSourceError(lineCountCache, filename);
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
		if (!DoCommandInternal(pAgent, command)) {
			// Command failed, error in result
			HandleSourceError(lineCountCache, filename);
			return false;
		}	
	}

	// Completion
	--m_SourceDepth;

	// if we're returing to the user and there is stuff on the source dir stack, print warning (?)
	if (!m_SourceDepth) {
		
		// Print working directory if source dir depth !=  0
		if (m_SourceDirDepth != 0) {
			DoPWD();	// Ignore error
		}
		m_SourceDirDepth = 0;
	}

	soarFile.close();
	return true;
}

void CommandLineInterface::HandleSourceError(int errorLine, const std::string& filename) {
	if (!m_SourceError) {
		// PopD to original source directory
		while (m_SourceDirDepth) {
			DoPopD(); // Ignore error here since it will be rare and a message confusing
			--m_SourceDirDepth;
		}

		// Reset depths to zero
		m_SourceDepth = 0;
		m_SourceDirDepth = 0; // TODO: redundant

		m_SourceError = true;
		m_ErrorMessage += "\nSource command error on line ";
		// TODO: arbitrary buffer size here
		char buf[256];
		memset(buf, 0, 256);
		snprintf(buf, 256, "%d", errorLine);

		m_ErrorMessage += " of ";
		
		std::string directory;
		GetCurrentWorkingDirectory(directory); // Again, ignore error here

		m_ErrorMessage += filename + " (" + directory + ")";

	} else {
		m_ErrorMessage += "\n\t--> Sourced by: " + filename;
	}
}

