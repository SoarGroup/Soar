
#include "Input_Console.h"

#include <string>
#include <iostream>

using std::string; using std::cin; using std::cout; using std::endl;

// named/virtual constructor
Smart_Pointer<Input_Type> Input_Console::create()
{
	Smart_Pointer<Input_Console> ptr = new Input_Console();
	return ptr;
}

string Input_Console::get_command()
{
	// get the command from cin
	string command;
	cout << endl << "> ";
	getline(cin, command);
	return command;
}
