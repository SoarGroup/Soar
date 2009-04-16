/////////////////////////////////////////////////////////////////
// help command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include <iostream>
#include <fstream>

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseHelp(std::vector<std::string>& argv) {
	if (argv.size() > 2) {
		SetErrorDetail("Pass only the command name you would like help with.");
		return SetError(CLIError::kTooManyArgs);
	}

	if (argv.size() == 2) {
		return DoHelp(&(argv[1]));
	}
	return DoHelp();
}

bool listHelpTopics(const std::string& directory, std::list< std::string >& topics) {

#ifdef WIN32
	// Windows-specific directory listing
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	std::stringstream fullpath;
	fullpath << directory << "\\" << "*.*";

	// Open a file find using the universal dos wildcard *.*
	hFind = FindFirstFile( fullpath.str().c_str(), &FindFileData );
	if ( hFind == INVALID_HANDLE_VALUE ) {
		// Not a single file, or file system error, we'll just assume no files
		return true;	
	}

	do {
		if ( !( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {
			topics.push_back( std::string( FindFileData.cFileName ) );
		}
	} while ( FindNextFile(hFind, &FindFileData ) );

	// Close the file find
	FindClose( hFind );
	return true;

#else // WIN32
	//DIR* directoryPointer;
	//struct dirent* entry;

	//// Get the current working directory and store in dir
	//std::string dir;
	//if ( !GetCurrentWorkingDirectory( dir ) ) return false;

	//// Open the directory for reading
	//if ((directoryPointer = opendir(dir.c_str())) == 0) return SetError(CLIError::kDirectoryOpenFailure);

	//// Read the files
	//errno = 0;
	//while ((entry = readdir(directoryPointer)) != 0) {
	//	m_Result << '\n';
	//	PrintFilename(entry->d_name, entry->d_type == DT_DIR);
	//}

	//// Check for error
	//if (errno != 0) return SetError(CLIError::kDirectoryEntryReadFailure);

	//// Ignoring close error
	//closedir(directoryPointer);

	return true;
#endif // WIN32
}

bool CommandLineInterface::DoHelp(const std::string* pCommand) {

	std::string helpFile;
	if (!pCommand || !pCommand->size()) {
		std::list< std::string > topics;
		if ( !listHelpTopics( m_LibraryDirectory + "/CLIHelp", topics ) ) {
			return SetError( CLIError::kDirectoryOpenFailure );
		}

		if ( topics.size() == 0 ) {
			SetErrorDetail("The library location appears to be incorrect, please use set-library-location <path> where path leads to SoarLibrary.");
			return SetError(CLIError::kNoHelpFile);
		}

		std::list< std::string >::iterator iter = topics.begin();
		m_Result << "Help is available for the following topics:\n";
		while ( iter != topics.end() ) {
			m_Result << "  " << *iter << std::endl;
			++iter;
		}
		return true;

	} else {
		// check for aliases
		if (m_Aliases.IsAlias(*pCommand)) {
			m_Result << *pCommand << " is an alias.  Type 'alias " << *pCommand 
				<< "' to find out what command it is an alias for and look up help on that command.";
			return true;
		}

		helpFile = m_LibraryDirectory + "/CLIHelp/" + *pCommand;
	}

	if (!GetHelpString(helpFile)) return false;
	return true;
}

bool CommandLineInterface::GetHelpString(const std::string& helpFile) {

	std::ifstream helpFileStream(helpFile.c_str());
	if (!helpFileStream) 
	{
		// this should exist, if not then we have a library location problem.
		std::string runHelpFile = m_LibraryDirectory + "/CLIHelp/run";
		std::ifstream runHelpFileStream( runHelpFile.c_str() );
		if ( !runHelpFileStream )
		{
			SetErrorDetail("The library location appears to be incorrect, please use set-library-location <path> where path leads to SoarLibrary.");
			return SetError(CLIError::kNoHelpFile);
		}

		SetErrorDetail("No help is available for " + helpFile);
		return SetError(CLIError::kNoHelpFile);
	}

	char ch;
	while(helpFileStream.get(ch)) m_Result.put(ch);

	if (!helpFileStream.eof() || !m_Result) return SetError(CLIError::kHelpFileError);
	return true;
}

