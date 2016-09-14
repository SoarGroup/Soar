/////////////////////////////////////////////////////////////////
// dirs command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_Utils.h"

#include <errno.h>

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoCD(const std::string* pDirectory)
{
    if (chdir(pDirectory->c_str()))
    {
        return SetError("Error changing to " + *pDirectory);
    }
    return true;
}

bool CommandLineInterface::DoPWD()
{

    std::string directory;
    bool ret = GetCurrentWorkingDirectory(directory);

    if (directory.size())
    {
        if (m_RawOutput)
        {
            m_Result << directory;
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamDirectory, sml_Names::kTypeString, directory);
        }
    }

    return ret;
}

bool CommandLineInterface::DoDirs()
{

    StringStack tempStack;
    
    std::string cwd;
    if (!GetCurrentWorkingDirectory(cwd))
    {
        return false;
    }
    
    // cwd is top of stack
    if (m_RawOutput)
    {
        m_Result << cwd;
    }
    else
    {
        AppendArgTagFast(sml_Names::kParamDirectory, sml_Names::kTypeString, cwd);
    }
    
    // print rest of stack making a new one
    while (m_DirectoryStack.size())
    {
        if (m_RawOutput)
        {
            m_Result << ' ' << m_DirectoryStack.top();
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamDirectory, sml_Names::kTypeString, m_DirectoryStack.top());
        }
        
        tempStack.push(m_DirectoryStack.top());
        m_DirectoryStack.pop();
    }
    
    // put the old stack back together
    while (tempStack.size())
    {
        m_DirectoryStack.push(tempStack.top());
        tempStack.pop();
    }
    return true;
}

bool CommandLineInterface::DoPushD(const std::string& directory)
{

    // Sanity check thanks to rchong
    if (directory.length() == 0)
    {
        return true;
    }

    // Target directory required, checked in DoCD call.

    // Save the current (soon to be old) directory
    std::string oldDirectory;
    if (!GetCurrentWorkingDirectory(oldDirectory))
    {
        return false;    // Error message handled in function
    }

    // Change to the new directory.
    if (!DoCD(&directory))
    {
        return false;    // Error message handled in function
    }

    // Directory change successful, store old directory and move on
    m_DirectoryStack.push(oldDirectory);
    return true;
}

bool CommandLineInterface::DoPopD()
{

    // There must be a directory on the stack to pop
    if (m_DirectoryStack.empty())
    {
        return SetError("Directory stack is empty.");
    }

    // Change to the directory
    if (!DoCD(&(m_DirectoryStack.top())))
    {
        return false;    // error handled in DoCD
    }

    // Pop the directory stack
    m_DirectoryStack.pop();
    return true;
}
bool CommandLineInterface::DoLS()
{

#ifdef WIN32

    // Windows-specific directory listing
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    // Open a file find using the universal dos wildcard *.*
    hFind = FindFirstFile("*.*", &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {

        // Not a single file, or file system error, we'll just assume no files
        return true;
    }

    // At least one file found, concatinate additional ones with newlines
    do
    {
        if (m_RawOutput)
        {
            m_Result << '\n';
        }
        PrintFilename(FindFileData.cFileName, FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? true : false);

    }
    while (FindNextFile(hFind, &FindFileData));

    // Close the file find
    FindClose(hFind);
    return true;

#else // WIN32
    DIR* directoryPointer;
    struct dirent* entry;

    // Get the current working directory and store in dir
    std::string dir;
    if (!GetCurrentWorkingDirectory(dir))
    {
        return false;
    }

    // Open the directory for reading
    if ((directoryPointer = opendir(dir.c_str())) == 0)
    {
        return SetError("Error opening directory.");
    }

    // Read the files
    errno = 0;
    while ((entry = readdir(directoryPointer)) != 0)
    {
        m_Result << '\n';
#ifndef DT_DIR
#define DT_DIR 4
#endif
        PrintFilename(entry->d_name, entry->d_type == DT_DIR);
    }

    // Check for error
    if (errno != 0)
    {
        return SetError("Error reading directory.");
    }

    // Ignoring close error
    closedir(directoryPointer);

    return true;
#endif // WIN32
}

void CommandLineInterface::PrintFilename(const std::string& name, bool isDirectory)
{
    if (m_RawOutput)
    {
        if (isDirectory)
        {
            m_Result << '[';
        }
        m_Result << name;
        if (isDirectory)
        {
            m_Result << ']';
        }
    }
    else
    {
        if (isDirectory)
        {
            AppendArgTag(sml_Names::kParamDirectory, sml_Names::kTypeString, name);
        }
        else
        {
            AppendArgTag(sml_Names::kParamFilename, sml_Names::kTypeString, name);
        }
    }
}

