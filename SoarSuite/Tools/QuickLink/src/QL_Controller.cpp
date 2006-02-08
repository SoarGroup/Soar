
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
void commit(istringstream& command);
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
	command_map_t command_map;
	load_command_map(command_map);

	
	Input_Controller& input = Input_Controller::instance();
	QL_Interface& ql_interface = QL_Interface::instance();

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
			// save the string into the process memory
			process_memory.push_back(command_line.str());
			string command;
			command_line >> command; // get the command from the string
			command = string_upper(command);

			if(command == "QUIT" || command == "EXIT")
				break;
			if(acquiring_new_connection && command != "REMOTE" && command != "LOCAL")
				throw Error("Please use \"remote\" or \"local\" to create a new connection before proceeding");

			command_fp_t ptr = command_map[command]; // get the function that corresponds to the command string

			if(ptr)  // if there is a valid function, call it
				ptr(command_line);
			else
			{
				ql_interface.soar_command_line(command_line.str());
				command_map.erase(command);
			}

		}
		catch (Error& error)
		{
			cout << error.msg << endl;
		}
	}

	input.IC_Shutdown();
	ql_interface.QL_Shutdown();
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
	command_map["COMMIT"] = commit;
	command_map["SAVEP"] = save_process;
	command_map["SP"] = save_process;
	command_map["CLEARP"] = clear_process_mem;
	command_map["CP"] = clear_process_mem;
	command_map["PAUSE"] = pause_process;
	command_map["CONTINUE"] = continue_process;
	command_map["CONT"] = continue_process;
	command_map["LASTOUTPUT"] = print_last_output;
	command_map["LO"] = print_last_output;
	command_map["ENDP"] = end_current_input_type;
	command_map["EP"] = end_current_input_type;
	command_map["STATUS"] = display_input_link;
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

	if(setup_id_att_value(command, id, att, value, c_delete_usage_error)) 
		QL_Interface::instance().delete_identifier(id, att, value);
	else
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

	setup_id_att_value(command, id, att, old_value, c_change_usage_error);
	if(!(command >> new_value))
		throw Error(c_change_usage_error);

	change_value_wme(id, att, old_value, new_value);
}

// clear the input-link
void clear_input_link(istringstream& command)
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
	string filename;
	if(!(command >> filename))
		throw Error(c_load_usage_error);

	Input_Controller::instance().generate_new_input(INPUT_FILE_T, filename);
}

// have soar run til output is generated
void run_til_output(istringstream& command)
{
	QL_Interface::instance().run_til_output();
}

// commit the changes we have made to the input-link
void commit(istringstream& command)
{
	QL_Interface::instance().commit();
}

// save the current process memory to a file
void save_process(istringstream& command)
{
	process_memory.pop_back(); // we do not want this command to end up in the process mem file

	string filename;
	if(!(command >> filename))
		throw Error(c_savep_usage_error);
	ofstream outfile(filename.c_str());
	if(!outfile)
		throw Error("Unable to open " + filename + ".");

	// this ninja line prints all of the strings to a file, each line is followed by a '\n'
	copy(process_memory.begin(), process_memory.end(), ostream_iterator<string>(outfile, "\n"));
}

// clear the current process memory
void clear_process_mem(istringstream& command)
{
	process_memory.clear();
}

// allow keyboard input in the middle of a process
void pause_process(istringstream& command)
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

void continue_process(istringstream& command)
{
	process_paused = false;  // this will cause input to come from the correct source
	QL_Interface::instance().turn_off_updates(); // views will not be updated
}

// print the last generated output
void print_last_output(istringstream& command)
{
	QL_Interface::instance().print_last_output();
}

// kill the current input-type
void end_current_input_type(istringstream& command)
{
	Input_Controller::instance().kill_current_input_source();
}

// show the current input-link
void display_input_link(istringstream& command)
{
	QL_Interface::instance().update_views_now();
}

// spawn the debugger
void spawn_debugger(istringstream& command)
{
	QL_Interface::instance().spawn_debugger();
}

// make a remote connection
void connect_remotely(istringstream& command)
{
	acquiring_new_connection = true;
	
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

	QL_Interface::instance().setup_input_link("IL");

	acquiring_new_connection = false;
}

// make a local kernel
void create_local_kernel(istringstream& command)
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
	if(inst.id_exists(value))
		throw Error("The identifier name " + value + " already exists.");

	// otherwise, create it
	inst.add_identifier(id, att, value);
}

// check to see if the wme exists, if it doesn't, add it
template <typename T>
void create_value_wme(string id, string att, T value)
{
	QL_Interface& inst = QL_Interface::instance();

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
		if(isalpha(str[i]) || ispunct(str[i])) // we know it is a string
			return e_STRING;
	}
	if(num_periods == 0) // we haven't seen a character and it doesn't have a period - int
		return e_INT;
	else if(num_periods == 1 && (isalpha(str[0]) || isalpha(str[str.length()-1]))) // - float
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



