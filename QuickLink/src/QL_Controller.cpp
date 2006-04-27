/* QL_Controller.cpp
*
* This file represents the command line interface to QL.  This is the model for how to
* interface with QL_Interface.
*
*/

#include "Utilities.h"
#include "QL_Interface.h"
#include "Input_Type.h"
#include "Input_Controller.h"
#include "View_Console.h"

#include <map>
#include <string>
#include <list>
#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include <fstream>
#include <iterator>

/* this is for VS's memory checking functionality */
//#define MEM_LEAK_DEBUG // uncomment this for functionality
#ifdef MEM_LEAK_DEBUG
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _MSC_VER
#endif //MEM_LEAK_DEBUG

/* end mem check */

using std::map; using std::string; using std::list;
using std::endl; using std::cout; using std::copy;
using std::istringstream; using std::vector; using std::ofstream;
using std::ostream_iterator; using std::cin;

enum value_type { e_INT , e_FLOAT , e_STRING };

typedef void (*command_fp_t)(istringstream& command);
typedef map<string, command_fp_t> command_map_t;

void load_command_map(command_map_t& command_map);

// command map functions
void add_object(istringstream& command);
void delete_object(istringstream& command);
void update_object(istringstream& command);
void clear_input_link(istringstream& command);
void save_input_link(istringstream& command);
void load_input_file(istringstream& command);
void run_til_output(istringstream& command);
void save_process(istringstream& command);
void clear_process_mem(istringstream& command);
void pause_process(istringstream& command);
void continue_process(istringstream& command);
void print_last_output(istringstream& command);
void end_current_input_type(istringstream& command);
void display_input_link(istringstream& command);
void spawn_debugger(istringstream& command);
void connect_remotely(istringstream& command);
void create_local_kernel(istringstream& command);

// helper functions
void create_identifier(const string& id, const string& att, string value);
template <typename T>
void create_value_wme(string id, string att, T value);
template <typename T>
void delete_value_wme(string id, string att, T value);
template <typename T>
void change_value_wme(const string& id, const string& att, T old_value, T new_value);
value_type decipher_type(const string& str);
bool setup_id_att_value(istringstream& command, string& id, string& att, string& value, const string& usage_error);

const string c_delete_usage_error = "Syntax error: Usage: \"delete <identifer> ^<attribute> <value>\"";
const string c_add_usage_error = "Syntax error: Usage: \"add <identifer> ^<attribute> <value>\"";
const string c_change_usage_error = "Syntax error: Usage: \"change <identifer> ^<attribute> <old-value> <new-value>\"";
const string c_save_usage_error = "Syntax error: Usage: \"save <filename>\"";
const string c_load_usage_error = "Syntax error: Usage: \"load <filename>\"";
const string c_savep_usage_error = "Syntax error: Usage: \"savep <filename>\"";

// this vector holds the information for the processes
vector<string> process_memory;
bool process_paused = false;
bool acquiring_new_connection = false;

int main()
{
#ifdef MEM_LEAK_DEBUG
	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 112;
	// we put everything in its own scope so memory leak detection can be done
	{
#endif //MEM_LEAK_DEBUG
		
		// create and load the command map
		command_map_t command_map;
		load_command_map(command_map);

		// create instances of the input controller and ql interface		
		Input_Controller& input = Input_Controller::instance();
		QL_Interface& ql_interface = QL_Interface::instance();

		// the following three things should always be done in the specified order
		// 1. ALWAYS create the kernel
		// 2. (optional) attach all views
		// 3. ALWAYS setup the input link, the parameter string is the name the client (this)
		//    will refer to the input link as
		ql_interface.create_new_kernel();
		ql_interface.attach_view(View_Console::create());
		ql_interface.setup_input_link("IL");
		

		while(true)  // do not leave command loop unless a break is encountered
		{
			try
			{
				istringstream command_line;
				if(process_paused)
					command_line.str(input.force_command_line_input());
				else // get the command and check to see if it is a valid command
					command_line.str(input.get_command());
				Input_Controller::instance().should_print_prompt = false;
				// save the string into the process memory
				process_memory.push_back(command_line.str());
				string command;
				command_line >> command; // get the command from the string
				command = string_upper(command);

				if(command == "QUIT" || command == "EXIT")
					break;
				// this will be true when a command to create a new kernel or connect to a remote one has
				// failed, and we cannot continue until a connection has been established
				if(acquiring_new_connection && command != "REMOTE" && command != "LOCAL")
					throw Error("Please use \"remote\" or \"local\" to create a new connection before proceeding");

				command_fp_t ptr = command_map[command]; // get the function that corresponds to the command string

				if(ptr)  // if there is a valid function, call it
					ptr(command_line);
				else // not valid
				{
					ql_interface.soar_command_line(command_line.str()); // try to execute it on the CLI
					command_map.erase(command); // erase the command from the command map, this must be done
				}
				
				Input_Controller::instance().should_print_prompt = true;

			}
			catch (Error& error)
			{
				cout << error.msg << endl;
			}
		}

		process_memory.clear();
		input.IC_Shutdown();
		ql_interface.QL_Shutdown();
#ifdef MEM_LEAK_DEBUG
	}

#ifdef _MSC_VER
	printf("\nNow checking memory.  Any leaks will appear below.\nNothing indicates no leaks detected.\n") ;
	printf("\nIf no leaks appear here, but some appear in the output\nwindow in the debugger, they have been leaked from a DLL.\nWhich is reporting when it's unloaded.\n\n") ;

	// Set the memory checking output to go to Visual Studio's debug window (so we have a copy to keep)
	// and to stdout so we can see it immediately.
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	// Now check for memory leaks.
	// This will only detect leaks in objects that we allocate within this executable and static libs.
	// If we allocate something in a DLL then this call won't see it because it works by overriding the
	// local implementation of malloc.
	_CrtDumpMemoryLeaks();
#endif // _MSC_VER
#endif //MEM_LEAK_DEBUG
}

void load_command_map(command_map_t& command_map)
{
	command_map["ADD"] = add_object;
	command_map["A"] = add_object;
	command_map["DELETE"] = delete_object;
	command_map["D"] = delete_object;
	command_map["CHANGE"] = update_object;
	command_map["C"] = update_object;
	command_map["CLEAR"] = clear_input_link;
	command_map["SAVE"] = save_input_link;
	command_map["S"] = save_input_link;
	command_map["LOAD"] = load_input_file;
	command_map["L"] = load_input_file;
	command_map["GO"] = run_til_output;
	command_map["G"] = run_til_output;
	command_map["SCRIPT"] = save_process;
	command_map["SC"] = save_process;
	command_map["CLEARS"] = clear_process_mem;
	command_map["CS"] = clear_process_mem;
	command_map["PAUSE"] = pause_process;
	command_map["CONTINUE"] = continue_process;
	command_map["CONT"] = continue_process;
	command_map["OUTPUT"] = print_last_output;
	command_map["ENDS"] = end_current_input_type;
	command_map["ES"] = end_current_input_type;
	command_map["INPUT"] = display_input_link;
	command_map["DEBUG"] = spawn_debugger;
	command_map["REMOTE"] = connect_remotely;
	command_map["LOCAL"] = create_local_kernel;
}

// command map functions

void add_object(istringstream& command)
{
	// there are 3 things left in the stream command: identifier, attribute and value
	string id, att, value;
	
	// if this is an identifier, this function will return true
	if(setup_id_att_value(command, id, att, value, c_add_usage_error))
		create_identifier(id, att, value);
	else // this is some form of a value-type wme
	{
		value_type type = decipher_type(value);
		switch(type) {
			case e_STRING :
				create_value_wme(id, att, value);
				break;
			case e_INT :
				create_value_wme(id, att, atoi(value.c_str()));
				break;
			case e_FLOAT :
				create_value_wme(id, att, atof(value.c_str()));
				break;
			default :
				assert(false && "Bad value_type in add_object");
		}
	}
}

// remove an object from the QuickLink

void delete_object(istringstream& command)
{
	// there are 3 things left in the stream command: identifier, attribute and value
	string id, att, value;

	// this returns true if it is an id
	if(setup_id_att_value(command, id, att, value, c_delete_usage_error)) 
		QL_Interface::instance().delete_identifier(id, att, value);
	else // some type of value wme
	{
		value_type type = decipher_type(value);
		switch(type) {
			case e_STRING :
				delete_value_wme(id, att, value);
				break;
			case e_INT :
				delete_value_wme(id, att, atoi(value.c_str()));
				break;
			case e_FLOAT :
				delete_value_wme(id, att, atof(value.c_str()));
				break;
			default :
				assert(false && "Bad value_type in delete_object");
		}
	}
}

// update the value of a wme
void update_object(istringstream& command)
{
	string id, att, old_value, new_value;

	// we can ignore the return type here because a ID cannot be updated
	setup_id_att_value(command, id, att, old_value, c_change_usage_error);
	if(!(command >> new_value))
		throw Error(c_change_usage_error);

	value_type type = decipher_type(old_value);
	switch(type) {
			case e_STRING :
				change_value_wme(id, att, old_value, new_value);
				break;
			case e_INT :
				change_value_wme(id, att, atoi(old_value.c_str()), atoi(new_value.c_str()));
				break;
			case e_FLOAT :
				change_value_wme(id, att, atof(old_value.c_str()), atof(new_value.c_str()));
				break;
			default :
				assert(false && "Bad value_type in delete_object");
	}

	
}

// clear the input-link
void clear_input_link(istringstream&)
{
	QL_Interface::instance().clear_input_link();
}

// save the input-link
void save_input_link(istringstream& command)
{
	string filename;
	if(!(command >> filename))
		throw Error(c_save_usage_error);

	QL_Interface::instance().save_input_link(filename);
}

// load an input file
void load_input_file(istringstream& command)
{
	//remove this command from the process memory
	process_memory.pop_back();

	string filename;
	if(!(command >> filename))
		throw Error(c_load_usage_error);

	// notify input controller to create a new input file object and make it
	// the current source
	Input_Controller::instance().generate_new_input(INPUT_FILE_T, filename);
}

// have soar run til output is generated
void run_til_output(istringstream&)
{
	QL_Interface::instance().run_til_output();
}

// save the current process memory to a file
void save_process(istringstream& command)
{
	process_memory.pop_back(); // we do not want this command to end up in the process mem file

	string filename;
	if(!(command >> filename)) // check to make sure filename is present
		throw Error(c_savep_usage_error);
	ofstream outfile(filename.c_str());
	if(!outfile)
		throw Error("Unable to open " + filename + ".");

	// this ninja line prints all of the strings to a file, each line is followed by a '\n'
	copy(process_memory.begin(), process_memory.end(), ostream_iterator<string>(outfile, "\n"));

	cout << endl << "Script Saved" << endl;
}

// clear the current process memory
void clear_process_mem(istringstream&)
{
	process_memory.clear();

	cout << endl << "Script Memory Cleared" << endl;
}

// allow keyboard input in the middle of a process
void pause_process(istringstream&)
{
	// check to make sure it is okay to pause
	if(Input_Controller::instance().okay_to_pause())
	{
		if(process_paused)
			throw Error("Process is already paused.");
		process_paused = true;
		QL_Interface::instance().turn_on_updates(); // views will be updated
		QL_Interface::instance().update_views_now(); // notify the views of the current state
	}
}

void continue_process(istringstream&)
{
	process_paused = false;  // this will cause input to come from the correct source
	QL_Interface::instance().turn_off_updates(); // views will not be updated
}

// print the last generated output
void print_last_output(istringstream&)
{
	QL_Interface::instance().print_last_output();
	process_memory.pop_back();

}

// kill the current input-type
void end_current_input_type(istringstream&)
{
	Input_Controller::instance().kill_current_input_source();
}

// show the current input-link
void display_input_link(istringstream&)
{
	QL_Interface::instance().update_views_now();
	process_memory.pop_back();
}

// spawn the debugger
void spawn_debugger(istringstream&)
{
	QL_Interface::instance().spawn_debugger();

	process_memory.pop_back();
}

// make a remote connection
void connect_remotely(istringstream&)
{
	acquiring_new_connection = true; // set this true so we can detect what state we are in
									 // if an error is thrown within this code
	
	// establish remote connection returns a string of the possible agents
	string agents = QL_Interface::instance().establish_remote_connection();
	string agent_name;

	string junk;
	cout << "\nAvailable agents are:\n\n" << agents << endl << endl;
	cout << "\nWhat is the name of the agent you would like to connect to: ";
	cin >> agent_name;
	getline(cin, junk);

	while(!QL_Interface::instance().specify_agent(agent_name))
	{
		cout << "That agent name is invalid, please specify a new name: ";
		cin >> agent_name;
		getline(cin, junk);
	}

	// always setup the input-link after establishing a remote connection
	QL_Interface::instance().setup_input_link("IL");

	acquiring_new_connection = false;
}

// make a local kernel
void create_local_kernel(istringstream&)
{
	acquiring_new_connection = true;
	QL_Interface::instance().create_new_kernel();
	QL_Interface::instance().setup_input_link("IL");
	acquiring_new_connection = false;
}






// helper functions

// create an identifier
void create_identifier(const string& id, const string& att, string value)
{
	QL_Interface& inst = QL_Interface::instance();
	if(inst.id_exists(value)) // this should be a shared id
		inst.add_shared_id(id, att, value);
	else // otherwise, create it
		inst.add_identifier(id, att, value);
}

// check to see if the wme exists, if it doesn't, add it
template <typename T>
void create_value_wme(string id, string att, T value)
{
	QL_Interface& inst = QL_Interface::instance();

	// make sure the wme doesn't already exist
	if(inst.wme_exists(id, att, value))
		throw Error("The WME " + id + " ^" + att + " " + string_make(value) + " already exists");

	inst.add_value_wme(id, att, value);
}

// check to see if the wme exists, if it does, delete it
template <typename T>
void delete_value_wme(string id, string att, T value)
{
	QL_Interface& inst = QL_Interface::instance();

	if(!inst.wme_exists(id, att, value))
		throw Error("The WME " + id + " ^" + att + " " + string_make(value) + " doesn't exist");

	inst.delete_value_wme(id, att, value);
}

// check to see if the wme exists, if it does, update its value
template <typename T>
void change_value_wme(const string& id, const string& att, T old_value, T new_value)
{
	QL_Interface& inst = QL_Interface::instance();

	if(!inst.wme_exists(id, att, old_value))
		throw Error("The WME " + id + " ^" + att + " " + string_make(old_value) + " doesn't exist");

	inst.update_value_wme(id, att, old_value, new_value);
}

// decide the type of the given string
value_type decipher_type(const string& str)
{
	// look at each member of the string.  If we see a character we know it is a string
	// otherwise, if we see a period, and all numbers we know it is a float
	int num_periods = 0;
	for(int i = 0; i < int(str.length()) ; i++)
	{
		if (str[i] == '.')
			num_periods++;
		else if(isalpha(str[i]) || ispunct(str[i])) // we know it is a string
			return e_STRING;
	}
	if(num_periods == 0) // we haven't seen a character and it doesn't have a period - int
		return e_INT;
	else if(num_periods == 1 && (!isalpha(str[0]) || !isalpha(str[str.length()-1]))) // - float
		return e_FLOAT;

	// otherwise just return string
	return e_STRING;
}

/* This function extracts the id att and value from the command.  Error checking is done.
   The boolean returned will be true if the triple is an identifier */
bool setup_id_att_value(istringstream& command, string& id, string& att, string& value, const string& usage_error)
{
	// there are 3 things left in the stream command: identifier, attribute and value
	if(!(command >> id >> att >> value))
		throw Error(usage_error);

	// make the id string uppercase and check to see if it exists
	id = string_upper(id);
	if(!QL_Interface::instance().id_exists(id))
		throw Error("Identifier " + id + " does not exist.");

	// remove the '^' from the attribute
	remove_first_char(att);

	// if this is an identifier, it will start with a '/'
	if(value[0] == '/')
	{
		remove_first_char(value); // get rid of '/'
		value = string_upper(value);
		return true;
	}

	return false;
}



