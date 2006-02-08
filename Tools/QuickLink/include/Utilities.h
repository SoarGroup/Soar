#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <sstream>

/* Utility functions and classes used by other modules */

// a simple class for error exceptions - msg points to a C-string error message
struct Error {
	Error(std::string in_msg = "") :
		msg(in_msg)
		{}

		void set_msg(std::string in_msg = "")
		{ msg = in_msg; }

	std::string msg;
};

// Clear the cin stream and throw all characters
// up to and including the next newline. If eof 
// is encountered, note the stream is left in the eof state. 
void skip_rest_of_line();

// Make all of the letters in a string uppercase.  Return
// a copy of the new uppercase word.
std::string string_upper(const std::string& word);

// return a string form of the object
template <typename T>
std::string string_make(T value);

template <typename T>
std::string string_make(T value)
{
	std::ostringstream ss;
	ss << value;
	return ss.str();
}

// construct a key value for an identifier, attribute, value triple
std::string make_key(const std::string& id, const std::string& att, const std::string& value = "");

// remove the first character in a string.
void remove_first_char(std::string& str);

#endif

