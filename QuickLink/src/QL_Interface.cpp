
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__)
#	define WINDOWS
#endif

#include "QL_Interface.h"
#include "Utilities.h"
#include "WME_Id.h"
#include "WME_Shared.h"

#include <string>
#include <functional>
#include <iostream>
#include <fstream>
#ifdef WINDOWS
#include <process.h>
#include <windows.h>
#include <direct.h>
#endif

using std::string; using std::make_pair; using std::cerr; using std::endl;
using sml::Kernel; using sml::Identifier; using sml::Agent;
using std::for_each; using std::mem_fun; using std::bind2nd;
using std::ofstream;
using sml::smlSystemEventId; using sml::smlEVENT_SYSTEM_START;
using sml::smlAgentEventId; using sml::smlEVENT_BEFORE_AGENT_REINITIALIZED;
using sml::smlEVENT_AFTER_AGENT_REINITIALIZED;

void MyAgentEventHandler(smlAgentEventId id, void* pUserData, Agent* pAgent);

// return singleton instance
QL_Interface& QL_Interface::instance()
{
	static QL_Interface QLI;
	return QLI;
}

QL_Interface::QL_Interface()  : should_update_views(true), kernel_destroyed(true) 
{
#ifdef WINDOWS
	_chdir("../../SoarLibrary/bin/");
#endif
}

// return the identifier with the specified name
Smart_Pointer<WME_Id> QL_Interface::get_identifier(string id_name)
{
	Smart_Pointer<WME_Id> id = m_id_container[id_name];
	if(id)
		return id;

	// else clean up id_name
	m_id_container.erase(id_name);
	throw Error("Identifier " + id_name + "does not exist.");
}

// create a new kernel
void QL_Interface::create_new_kernel(int port)
{
	prepare_for_new_connection();

	m_pKernel = Kernel::CreateKernelInNewThread("SoarKernelSML", port);

	if(m_pKernel->HadError())
	{
		string error = m_pKernel->GetLastErrorDescription();
		m_pKernel->Shutdown();
		delete m_pKernel;
		kernel_destroyed = true; // set this flag so we know current state
		throw Error(error);
	}
	kernel_destroyed = false; // this variable is needed so that we can tell when swtiches fail and we don't delete things twice

	m_pAgent = m_pKernel->CreateAgent("QuickLink");

	if(m_pKernel->HadError())
		throw Error(m_pKernel->GetLastErrorDescription());


	// TODO: These should be in a initialization function
	m_pKernel->RegisterForAgentEvent(smlEVENT_BEFORE_AGENT_REINITIALIZED, MyAgentEventHandler, this);
	m_pKernel->RegisterForAgentEvent(smlEVENT_AFTER_AGENT_REINITIALIZED, MyAgentEventHandler, this);

}

// establish a remote connection
string QL_Interface::establish_remote_connection()
{
	prepare_for_new_connection();

	m_pKernel = sml::Kernel::CreateRemoteConnection( true , NULL );

	//Error checking to see if kernel loaded correctly
	if(m_pKernel->HadError())
	{
		string error = m_pKernel->GetLastErrorDescription(); 
		error += "\n\nPlease make sure a kernel exists on the default port and try to connect again\n\n";
		m_pKernel->Shutdown();
		delete m_pKernel;
		kernel_destroyed = true; // set this so we can remember we are in an unstable state
		throw Error(error);
	}
	kernel_destroyed = false;

	// TODO: return the agent names in a better format than delimeted by '\n'
	string agents;	
	for(int i = 0; i < m_pKernel->GetNumberAgents(); i++)
		agents += (m_pKernel->GetAgentByIndex(i))->GetAgentName();

	return agents;
}

// specify the agent you want chosen
bool QL_Interface::specify_agent(const string& name)
{
	m_pAgent = m_pKernel->GetAgent(name.c_str());
	
	//Make sure agent was successfully created
	if (!m_pAgent)
		return false;

	// initialization function
	m_pKernel->RegisterForAgentEvent(smlEVENT_BEFORE_AGENT_REINITIALIZED, MyAgentEventHandler, this);
	m_pKernel->RegisterForAgentEvent(smlEVENT_AFTER_AGENT_REINITIALIZED, MyAgentEventHandler, this);

	return true;
}


// create and set the name of the input-link object
void QL_Interface::setup_input_link(const string& name)
{
	if(!m_pAgent)
		throw Error("You must create or connect to a kernel first");

	m_id_container.insert(make_pair(name, WME_Id::create("" , "" , name , m_pAgent)));
	m_input_link_name = name;

	update_views();
}

// identifier functions

// add an identifier
void QL_Interface::add_identifier(const string& parent_id, const string& attribute, const string& id_name)
{
	if(!m_pAgent)
		throw Error("You must create or connect to a kernel first");

	// get the parent identifer object
	Smart_Pointer<WME_Id> parent = get_identifier(parent_id);
	// create the new ide
	Smart_Pointer<WME_Id> new_item = WME_Id::create(parent_id, attribute, id_name, m_pAgent, parent->get_id_object());

	m_id_container.insert(make_pair(id_name, new_item)); // insert it into id container
	parent->add_child(new_item); // insert it into the parent's id container

	commit();

	update_views();
}


// add an identifier that has already been created
void QL_Interface::add_created_identifier(Smart_Pointer<WME_Id> identifier)
{
	m_id_container.insert(make_pair(identifier->get_value(), identifier));
	commit();
}

// check to see if a given identifier exists
bool QL_Interface::id_exists(const string& id_name)
{
	Smart_Pointer<WME_Id> ptr = m_id_container[id_name];
	if(ptr)
		return true;

	m_id_container.erase(id_name); // if it doesn't exist, erase it, this must be done
	return false;
}

// delete a given identifier
void QL_Interface::delete_identifier(const string& parent_id, const string& attribute, const string& id_name)
{
	// get the parent and tell it to remove the object to be deleted
	Smart_Pointer<WME_Id> parent = get_identifier(parent_id);
	parent->remove_id_child(attribute, id_name, m_pAgent);

	// remove the id from the id container
	m_id_container.erase(id_name);



	commit();

	update_views();
}

// value-wme functions

// add a value wme
void QL_Interface::add_value_wme(const string& identifier, const string& attribute, const string& value)
{
	if(!m_pAgent)
		throw Error("You must create or connect to a kernel first");

	// get the parent and create the value wme and add it to the parents container
	Smart_Pointer<WME_Id> parent = get_identifier(identifier);
	parent->add_child(WME_String::create(identifier, attribute, value, m_pAgent, parent->get_id_object()));

	commit();
	update_views();
}

// creates a shared id
void QL_Interface::add_shared_id(const string& loop_start, const string& attribute, const string& loop_end)
{
	if(!m_pAgent)
		throw Error("You must create or connect to a kernel first");

	// get both id's
	Smart_Pointer<WME_Id> start_ptr = get_identifier(loop_start);
	Smart_Pointer<WME_Id> end_ptr = get_identifier(loop_end);

	// create the shared id
	start_ptr->add_child(WME_Shared::create(loop_start, attribute, loop_end, m_pAgent, start_ptr->get_id_object(), end_ptr->get_id_object()));

	commit();
	update_views();
}


void QL_Interface::add_value_wme(const string& identifier, const string& attribute, int value)
{
	if(!m_pAgent)
		throw Error("You must create or connect to a kernel first");

	// get the parent, create the wme, add it to parent's container
	Smart_Pointer<WME_Id> parent = get_identifier(identifier);
	parent->add_child(WME_Int::create(identifier, attribute, value, m_pAgent, parent->get_id_object()));

	commit();
	update_views();
}

void QL_Interface::add_value_wme(const string& identifier, const string& attribute, double value)
{
	if(!m_pAgent)
		throw Error("You must create or connect to a kernel first");

	// get the parent, create the wme, add it to parent's container
	Smart_Pointer<WME_Id> parent = get_identifier(identifier);
	parent->add_child(WME_Float::create(identifier, attribute, value, m_pAgent, parent->get_id_object()));

	commit();
	update_views();
}

// view functions
void QL_Interface::attach_view(View_Type* new_view)
{
	// let the new view know that it can initialize itself
	// this cannot be done in the constructor because information it needs
	// may not be created yet
	new_view->initialize();
	views.push_back(new_view);
}

// general functions

void QL_Interface::QL_Shutdown()
{
	views.clear();
	prepare_for_new_connection();
}

void QL_Interface::respond_to_init_soar_before()
{
	/*Smart_Pointer<WME_Id> il = m_id_container[m_input_link_name];
	il->remove_all_children(m_pAgent);
	m_id_container.clear();*/

	// call synch input-link in case a new identifier is used for the input-link
//	m_pAgent->SynchronizeInputLink();
//	setup_input_link(m_input_link_name); // generates all existing wmes
}

void QL_Interface::respond_to_init_soar_after()
{
	//m_pAgent->SynchronizeInputLink();
	//setup_input_link(m_input_link_name); // generates all existing wmes
}

void QL_Interface::remove_identifier(const string& name)
{ 
	//Smart_Pointer<WME_Id> test = m_id_container[name];
	m_id_container.erase(name); 
}

void QL_Interface::soar_command_line(const string& command)
{
	string info = m_pKernel->ExecuteCommandLine(command.c_str(), m_pAgent->GetAgentName());

	commit();
	update_views("\n" + info + "\n");
}

void QL_Interface::clear_input_link()
{
	// get the input-link id and remove all its chidren
	Smart_Pointer<WME_Id> il = m_id_container[m_input_link_name];
	il->clear_input_link(m_pAgent);

	// clear the container and put the il back in
	m_id_container.clear();
	m_id_container[m_input_link_name] = il;

	commit();
	update_views();
}

void QL_Interface::save_input_link(const string& filename)
{
	ofstream outfile(filename.c_str());
	if(!outfile)
		throw Error("Unable to open " + filename);

	// this will magically have all of the elements save themself
	Smart_Pointer<WME_Id> il = m_id_container[m_input_link_name];
	il->save_yourself(&outfile);

	update_views();
}

void QL_Interface::print_last_output()
{
	for_each(views.begin(), views.end(), mem_fun(&View_Type::display_last_output));
}

void QL_Interface::spawn_debugger()
{
#ifdef WINDOWS

	// spawn the debugger asynchronously
	
	int ret = _spawnlp(_P_NOWAIT, "javaw.exe", "javaw.exe", "-jar", "SoarJavaDebugger.jar", "-remote", NULL);
	if(ret == -1) {
		switch (errno) {
				case E2BIG:
					throw Error("arg list too long");
					break;
				case EINVAL:
					throw Error("illegal mode");
					break;
				case ENOENT:
					throw Error("file/path not found");
					break;
				case ENOEXEC:
					throw Error("specified file not an executable");
					break;
				case ENOMEM:
					throw Error("not enough memory");
					break;
				default:
					throw Error(string_make(ret));
		}
	}

#else // linux spawnning

	pid_t pid = fork();

	if (pid < 0)
		throw Error("fork failed");
	else if (pid == 0)
	{
		system("java -jar SoarJavaDebugger.jar -remote");
		m_pKernel->CheckForIncomingCommands();
		exit(1); // this forked process dies
	}
	else
		return;// parent process continues as normal

#endif

	// wait until we are notified that the debugger is spawned
	m_pKernel->GetAllConnectionInfo();
	char const * java_debugger = "java-debugger";
	char const * ready = "ready";

	update_views("loading...");

	while(1)
	{
#ifdef WINDOWS
		Sleep(100);
#else
		sleep(1);
#endif
		m_pKernel->GetAllConnectionInfo();
		char const * status = m_pKernel->GetAgentStatus(java_debugger);
		update_views(".");
		if(status && !strcmp(status,ready)) break;
	}

	update_views("complete!");
}


// event handlers

void MyAgentEventHandler(smlAgentEventId id, void*, Agent*)
{
	if(smlEVENT_BEFORE_AGENT_REINITIALIZED == id)
		QL_Interface::instance().respond_to_init_soar_before();
	else if(smlEVENT_AFTER_AGENT_REINITIALIZED == id)
		QL_Interface::instance().respond_to_init_soar_after();
}

// member functions

// clean up all connection variables
void QL_Interface::prepare_for_new_connection()
{
	Smart_Pointer<WME_Id> il = m_id_container[m_input_link_name];
	if(il)
		il->remove_all_children(m_pAgent);
	m_id_container.clear();
	il = 0; // this must happen here otherwise the input-link will not go away properly

	if(!kernel_destroyed)
	{
		m_pKernel->Shutdown();
		delete m_pKernel;
		kernel_destroyed = true;
		m_pKernel = (Kernel*)NULL;
	}
}

// update all of the views
void QL_Interface::update_views()
{
	if(should_update_views)
		for_each(views.begin(), views.end(), bind2nd(mem_fun(&View_Type::update), m_id_container));
}

// update the views with a message
void QL_Interface::update_views(string info)
{
	for_each(views.begin(), views.end(), bind2nd(mem_fun(&View_Type::update_info), info));
}

