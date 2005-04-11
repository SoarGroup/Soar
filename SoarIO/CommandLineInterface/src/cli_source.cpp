#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include <fstream>

#include "cli_Constants.h"
#include "sml_StringOps.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSource(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	if (argv.size() < 2) {
		// Source requires a filename
		return SetError(CLIError::kTooFewArgs);

	} else if (argv.size() > 2) {
		// but only one filename
		return SetError(CLIError::kSourceOnlyOneFile);
	}

	return DoSource(pAgent, argv[1]);
}

bool CommandLineInterface::Trim(std::string& line) {
	// trim whitespace and comments from line
	if (!line.size()) return true; // nothing on the line
	
	// remove leading whitespace
	std::string::size_type datapos = line.find_first_not_of(" \t");
	if (datapos != std::string::npos) {
		line = line.substr(datapos);
	}

	if (!line.size()) return true; // nothing else on the line

	// if there is a pipe on the line, we need to parse more carefully
	if (line.find_first_of('|') != std::string::npos) {
		
		bool pipe = false;

		for (unsigned pos = 0; pos < line.size(); ++pos) {
			if (!pipe) {
				// normal parsing
				switch (line[pos]) {
					case '|':	// enter pipe mode
						pipe = true;
						break;
					case '#':	// trim comments, done
						line = line.substr(pos, line.size() - pos);
						pos = line.size();
						break;
					default:	// keep going
						break;
				}
			} else {
				switch (line[pos]) {
					case '|':	// exit pipe mode
						pipe = false;
						break;
					default:	// keep going
						break;
				}
			}
		}
		// if in pipe mode, newline inside pipes
		if (pipe) return SetError(CLIError::kNewlineBeforePipe);

	} else {
		// no pipe, simply remove comments
		std::string::size_type commentpos = line.find_first_of('#');
		if (commentpos != std::string::npos) {
			line = line.substr(0, commentpos);
		}
	}
	return true;
}

bool CommandLineInterface::DoSource(gSKI::IAgent* pAgent, std::string filename) {
	if (!RequireAgent(pAgent)) return false;

	// Chop of quotes if they are there, open doesn't like them
	if ((filename.length() > 2) && (filename[0] == '\"') && (filename[filename.length() - 1] == '\"')) {
		filename = filename.substr(1, filename.length() - 2);
	}

	// Separate the path out of the filename if any
	std::string path;
	unsigned int separator1 = filename.rfind('/');
	if (separator1 != std::string::npos) {
		++separator1;
		if (separator1 < filename.length()) {
			path = filename.substr(0, separator1);
			filename = filename.substr(separator1, filename.length() - separator1);
			if (!DoPushD(path)) return false;
		}
	}
	unsigned int separator2 = filename.rfind('\\');
	if (separator2 != std::string::npos) {
		++separator2;
		if (separator2 < filename.length()) {
			path = filename.substr(0, separator2);
			filename = filename.substr(separator2, filename.length() - separator2);
			if (!DoPushD(path)) return false;
		}
	}

	// Open the file
	std::ifstream soarFile(filename.c_str());
	if (!soarFile) {
		if (path.length()) DoPopD();
		return SetError(CLIError::kOpenFileFail);
	}

	std::string line;				// Each line removed from the file
	std::string command;			// The command, sometimes spanning multiple lines
	std::string::size_type pos;		// Used to find braces on a line (triggering multiple line spanning commands)
	int braces = 0;					// Brace nest level (hopefully all braces are supposed to be closed)
	std::string::iterator iter;		// Iterator when parsing for braces and pounds
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

		// Trim whitespace and comments
		if (!Trim(line)) return false;

		if (!line.length()) continue; // Nothing on line, skip it

		// If there is a brace on the line, concatenate lines until the closing brace
		pos = line.find('{');

		if (pos != std::string::npos) {
			
			// Save this line number for error messages
			lineCountCache = lineCount;

			// While we are inside braces, stay in special parsing mode
			do {
				if (lineCountCache != lineCount) {
					if (!Trim(line)) return false; // Trim whitespace and comments on additional lines
				}

				// nothing on line or just whitespace and comments
				if (!line.size()) continue;

				// Enter special parsing mode
				iter = line.begin();
				while (iter != line.end()) {
					// Go through each of the characters, counting brace nesting level
					if (*iter == '{') ++braces;
					else if (*iter == '}') --braces;

					// Next character
					++iter;
				}
				
				// We finished that line, add it to the command
				command += line;

				// Did we close all of the braces?
				if (!braces) break; // Yes, break out of special parsing mode

				// Did we go negative?
				if (braces < 0) break; // Yes, break out on error

				// Put the newline back on it (getline eats the newline)
				command += '\n';

				// We're getting another line, increment count now
				++lineCount;

				// Get the next line from the file and repeat
			} while (getline(soarFile, line));

			// Did we break out because of closed braces or EOF?
			if (braces > 0) {
				// EOF while still nested
				SetError(CLIError::kUnmatchedBrace);
				HandleSourceError(lineCountCache, filename);
				if (path.length()) DoPopD();
				return false;

			} else if (braces < 0) {
				SetError(CLIError::kExtraClosingBrace);
				HandleSourceError(lineCountCache, filename);
				if (path.length()) DoPopD();
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
		unsigned oldResultSize = m_Result.str().size();
		if (DoCommandInternal(pAgent, command)) {
			// Add trailing newline if result changed size
			unsigned newResultSize = m_Result.str().size();
			if (oldResultSize != newResultSize) {
				// but don't add after sp's
				if (m_Result.str()[m_Result.str().size()-1] != '*') {
					m_Result << '\n';
				}
			}

		} else {
			// Command failed, error in result
			HandleSourceError(lineCountCache, filename);
			if (path.length()) DoPopD();
			return false;
		}	
	}

	// Completion
	--m_SourceDepth;

	// if we're returing to the user
	if (!m_SourceDepth) {
		// Print working directory if source dir depth !=  0
		if (m_SourceDirDepth != 0) DoPWD();	// Ignore error
		m_SourceDirDepth = 0;

		// Add finished message
		if (m_Result.str()[m_Result.str().size()-1] != '\n') m_Result << '\n';
		m_Result << "Source finished.";
	}

	soarFile.close();
	if (path.length()) DoPopD();
	return true;
}

void CommandLineInterface::HandleSourceError(int errorLine, const std::string& filename) {
	if (!m_SourceError) {

		// Output error message
		m_SourceErrorDetail.clear();
		m_SourceErrorDetail += "\nSource command error on line ";

		char buf[kMinBufferSize];
		m_SourceErrorDetail += Int2String(errorLine, buf, kMinBufferSize);

		m_SourceErrorDetail += " of ";
		
		std::string directory;
		GetCurrentWorkingDirectory(directory); // Again, ignore error here

		m_SourceErrorDetail += filename + " (" + directory + ")";

		// PopD to original source directory
		while (m_SourceDirDepth) {
			if (m_SourceDirDepth < 0) m_SourceDirDepth = 0; // don't loop forever
			DoPopD(); // Ignore error here since it will be rare and a message confusing
		}

		// Reset depth to zero
		m_SourceDepth = 0;

		m_SourceError = true;

	} else {
		m_SourceErrorDetail += "\n\t--> Sourced by: " + filename;
	}
}

