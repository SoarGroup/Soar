#include "Utilities.h"

#include <iostream>
#include <cctype>

using std::cin;
using std::string;

// a unique separator is needed to separate the id, att, value triple
// words to avoid problems that could be caused with "id" + "att" + "value"
// and "ida" + "tt" + "value".  Without a separator, these would have the same 
// key
const string make_key_word_separator = "@%$";

// Clear the cin stream and throw all characters
// up to and including the next newline. If eof 
// is encountered, note the stream is left in the eof state. 
void skip_rest_of_line()
{
	cin.clear();
	while(cin.get() != '\n') {}
}

// return an uppercase version of the string
string string_upper(const string& word)
{
	string toReturn;
	for(int index = 0; index < int(word.length()); index++)
		toReturn.push_back(toupper(word[index]));

	return toReturn;
}

// construct a key for the id att value triple
string make_key(const string& id, const string& att, const string& value)
{
	string key = id + make_key_word_separator + att + make_key_word_separator + value;
	return key;
}

// remove the first character from the word
void remove_first_char(std::string& str)
{
	str = str.substr(1, str.length()-1);
}