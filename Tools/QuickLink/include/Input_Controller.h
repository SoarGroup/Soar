
/* Input_Controller.h
*
*  A Meyers-Singleton class used to control where the input comes from that is
*  supplied to  QL_Controller
*
*/

#ifndef INPUT_CONTROLLER
#define INPUT_CONTROLLER

#include "Input_Type.h"
#include "Smart_Pointer.h"

#include <string>
#include <list>

enum INPUT_TYPE_T { INPUT_CONSOLE_T , INPUT_FILE_T };

class Input_Controller 
{
public:

	// get and instance of the singleton
	static Input_Controller& instance();

	// create a new input source and make it the current input source
	void generate_new_input(INPUT_TYPE_T e_input, std::string command);

	// figure out the input source and return the next command
	std::string get_command();

	// disregard the input stack and force input from command line
	std::string force_command_line_input();

	// query the current input and see if it is okay to pause
	bool okay_to_pause();

	// kills current input_type if it is possible
	void kill_current_input_source();

	// shutdown the input controller
	void IC_Shutdown();

	// reports whether or not the IC is shutting down
	bool is_shutting_down();

	bool should_print_prompt;

private:

	// things need to be kept private for singleton implementation
	Input_Controller();
	~Input_Controller() {}
	Input_Controller(const Input_Controller&);
	Input_Controller& operator= (const Input_Controller&);

	// input source queue
	std::list<Smart_Pointer<Input_Type> > input_stack;

	bool shutting_down;
};

#endif
