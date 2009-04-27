#include <portability.h>

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

#include "sml_StringOps.h"

#include <iostream>
#include <sstream>

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

		return (strcasecmp(pStr1, pStr2) == 0) ;
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

	// Changed from ltoa -> SNPRINTF by voigtjr
	SNPRINTF(buffer, maxChars, "%ld", value);
	buffer[maxChars - 1] = 0; // windows doesn't guarantee null termination
	return buffer;
}

/*************************************************************
* @brief Convert double to string.
*************************************************************/
char* sml::Double2String(double value, char* buffer, int maxChars)
{
	//return gcvt(value, maxChars - 1, buffer) ; // gcvt not portable
	SNPRINTF(buffer, maxChars, "%f", value);
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
	std::stringstream arg;
	bool quotes = false;
	bool pipes = false;
	bool escaped = false;
	int brackets = 0;
	int parens = 0;

	// Trim leading whitespace and comments from line
	if (!Trim(cmdline)) return -1;

	iter = cmdline.begin();
	for (;;) {

		// Skip leading whitespace
		while (isspace(*iter)) {
			if ( ++iter == cmdline.end() ) {
				break;
			}
		}
		if ( iter == cmdline.end() ) {
			break; // Nothing but space
		}

		// We have an argument
		++argc;
		arg.str( std::string() );
		// Use space as a delimiter unless inside quotes or brackets (nestable) or escaped with backslash
		while (!isspace(*iter) || quotes || pipes || brackets || parens) {
			if (escaped) {
				// Skip this one, return to unescaped mode
				escaped = false;
			} else {
				if (*iter == '\\') {
					// Flip the escaped flag
					escaped = true;

				} else if (*iter == '"') {
					// Flip the quotes flag
					quotes = !quotes;

				} else if (*iter == '|') {
					// Flip the pipes flag
					pipes = !pipes;

				} else if (!pipes) {
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
			}

			// Add to argument (if we eat quotes, this has to be moved into the else above
			arg << (*iter);

			// Move on
			++iter;

			// Are we at the end of the string?
			if ( iter == cmdline.end() ) 
			{
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
		argumentVector.push_back( arg.str() );

		if ( iter == cmdline.end() ) 
		{
			break;
		}
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
	bool quote = false;
	std::string::size_type searchpos = 0;

	const char* targets = "\\#|\"";
	for (pos = line.find_first_of(targets, searchpos); pos != std::string::npos; pos = line.find_first_of(targets, searchpos)) {
		switch (line[pos]) {
			case '\\': // skip backslashes
				searchpos = pos + 2;
				break;

			case '#': // if not inside pipes or quotes, erase from pound to end or newline encountered
				if (pipe || quote) {
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

			case '"': // note quote
				quote = !quote;
				searchpos = pos + 1;
				break;
		}
	}

	if (pipe || quote) {
		return false;
	}
	return true;
}

