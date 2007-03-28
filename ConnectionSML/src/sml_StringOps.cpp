#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

/////////////////////////////////////////////////////////////////
// StringOps
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : July 2004
//
// It's often useful to abstract over the string operations,
// in case a particular library fails to provide the implementation
// in the way we expect.
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_STRINGS_H
#include <strings.h>  // strcasecmp
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <assert.h>

#include "sml_StringOps.h"

#ifdef _MSC_VER
#define snprintf _snprintf 
#define stricmp _stricmp
#endif // _MSC_VER

/*************************************************************
* @brief Returns true if strings are equal (case sensitive).
*************************************************************/
bool sml::IsStringEqual(char const* pStr1, char const* pStr2)
{
	if (pStr1 == NULL || pStr2 == NULL)
		return false ;

	return (strcmp(pStr1, pStr2) == 0) ;
}

/*************************************************************
* @brief Returns true if strings are equal (case insensitive).
*************************************************************/
bool sml::IsStringEqualIgnoreCase(char const* pStr1, char const* pStr2)
{
	if (pStr1 == NULL || pStr2 == NULL)
		return false ;

#ifdef HAVE_STRINGS_H
		return (strcasecmp(pStr1, pStr2) == 0);
#else
		return (stricmp(pStr1, pStr2) == 0) ;
#endif
}

/*************************************************************
* @brief Convert int to string.
*************************************************************/
char* sml::Int2String(long value, char* buffer, int maxChars)
{
	// A 64-bit value can be 20 digits, plus one for -ve and one for NULL.
	// Anything less than that is not safe.
	if (maxChars < kMinBufferSize)
	{
		buffer[0] = '0' ;
		return buffer ;
	}

	// Changed from ltoa -> snprintf by voigtjr
	snprintf(buffer, maxChars, "%ld", value);
	buffer[maxChars - 1] = 0; // windows doesn't guarantee null termination
	return buffer;
}

/*************************************************************
* @brief Convert double to string.
*************************************************************/
char* sml::Double2String(double value, char* buffer, int maxChars)
{
	//return gcvt(value, maxChars - 1, buffer) ; // gcvt not portable
	snprintf(buffer, maxChars, "%f", value);
	buffer[maxChars - 1] = 0; // ensure null termination as win32 behavior is unspecified
	return buffer;
}

/*************************************************************
* @brief Returns a copy of the string.
*************************************************************/
char* sml::StringCopy(char const* pStr)
{
	char* pCopy = new char[strlen(pStr)+1] ;
	return strcpy(pCopy, pStr) ;
}

/*************************************************************
* @brief Deletes copied strings
*************************************************************/
void sml::StringDelete(char* pStr)
{
	delete[] pStr ;
}

/*************************************************************
* @brief A utility function, splits a command line into argument
*		 tokens and stores them in the argumentVector string.
* @return returns negative if there is an error, otherwise it
*		  returns the number of arguments found
*		  error codes:
*			-1 Newline detected before pipe (Trim failed)
*			-2 An extra (unmatched) '}' was found.
*			-3 An extra (unmatched) ')' was found.
*         the rest of the error codes are a mask:
*			-4 Unmatched "
*			-8 Unmatched {
*			-16 Unmatched (
*			-32 Unmatched |
*         the combinations:
*			-12 Unmatched " and {
*			-20 Unmatched " and (
*			-24 Unmatched { and (
*			-28 Unmatched ", { and (
*			-36 Unmatched | and "
*			-40 Unmatched | and {
*			-44 Unmatched | and { and "
*			-48 Unmatched | and (
*			-52 Unmatched | and ( and "
*			-56 Unmatched | and ( and {
*			-60 Unmatched | and ( and { and "
*************************************************************/
int sml::Tokenize(std::string cmdline, std::vector<std::string>& argumentVector) {
	int argc = 0;
	std::string::iterator iter;
	std::string arg;
	bool quotes = false;
	bool pipes = false;
	int brackets = 0;
	int parens = 0;

	// Trim leading whitespace and comments from line
	if (!Trim(cmdline)) return -1;

	for (;;) {

		// Is there anything to work with?
		if(cmdline.empty()) break;

		// Remove leading whitespace
		iter = cmdline.begin();
		while (isspace(*iter)) {
			cmdline.erase(iter);

			if (!cmdline.length()) break; //Nothing but space left
			
			// Next character
			iter = cmdline.begin();
		}

		// Was it actually trailing whitespace?
		if (!cmdline.length()) break;// Nothing left to do

		// We have an argument
		++argc;
		arg.clear();
		// Use space as a delimiter unless inside quotes or brackets (nestable)
		while (!isspace(*iter) || quotes || pipes || brackets || parens) {
			if (*iter == '"') {
				// Flip the quotes flag
				quotes = !quotes;

			} else if (*iter == '|') {
				// Flip the pipes flag
				pipes = !pipes;

			} else {
				if (*iter == '{') {
					++brackets;
				} else if (*iter == '}') {
					--brackets;
					if (brackets < 0) {
						return -2;
					}
				}
				if (*iter == '(') {
					++parens;
				} else if (*iter == ')') {
					--parens;
					if (parens < 0) {
						return -3;
					}
				}
			}

			// Add to argument (if we eat quotes, this has to be moved into the else above
			arg += (*iter);

			// Delete the character and move on on
			cmdline.erase(iter);
			iter = cmdline.begin();

			// Are we at the end of the string?
			if (iter == cmdline.end()) {

				// Did they close their quotes or brackets?
				if (quotes || pipes || brackets || parens) {
					// FIXME: note that Trim will fail with bad pipes before parsing gets here,
					// so I don't think pipes will ever be true at this point.
					int code = 0;
					if (quotes) code += -4;
					if (pipes) code += -32;
					if (brackets) code += -8;
					if (parens) code += -16;
					return code;
				}
				break;
			}
		}

		// Store the arg
		argumentVector.push_back(arg);
	}

	// Return the number of args found
	return argc;
}

/*************************************************************
* @brief Trim comments off of a line (for command parsing)
* @return true on success, false if there is a new-line before a pipe quotation ends
*************************************************************/
bool sml::Trim(std::string& line) {
	// trim whitespace and comments from line
	if (!line.size()) return true; // nothing on the line

	// remove leading whitespace
	std::string::size_type pos = line.find_first_not_of(" \t");
	if (pos != std::string::npos) line = line.substr(pos);

	bool pipe = false;
	std::string::size_type searchpos = 0;

	for (pos = line.find_first_of("\\#|", searchpos); pos != std::string::npos; pos = line.find_first_of("\\#|", searchpos)) {
		switch (line[pos]) {
			case '\\': // skip backslashes
				searchpos = pos + 2;
				break;

			case '#': // if not inside pipe, erase from pound to end or newline encountered
				if (pipe) {
					searchpos = pos + 1;
				} else {
					{
						std::string::size_type nlpos = line.find('\n', pos + 1);
						if (nlpos == std::string::npos) {
							// No newline encountered
							line = line.substr(0, pos);
						} else {
							line.erase(pos, nlpos - pos);
							searchpos = pos;
						}
					}
				}
				break;

			case '|': // note pipe
				pipe = !pipe;
				searchpos = pos + 1;
				break;
		}
	}

	if (pipe) {
		return false;
	}
	return true;
}

