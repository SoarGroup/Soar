
#include "QL_Interface.h"
#include "Utilities.h"
#include "WME_Id.h"

#include <string>
#include <functional>
#include <iostream>
#include <fstream>
#if defined _WIN32 || _WIN64
#include <process.h>
#include <windows.h>
#endif

using std::string; using std::make_pair; using std::cerr; using std::endl;
using sml::Kernel; using sml::Identifier;
using std::for_each; using std::mem_fun; using std::bind2nd;
using std::ofstream;
using sml::smlSystemEventId; using sml::smlEVENT_SYSTEM_START;

void MyStartSystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel);

QL_Interface& QL_Interface::instance()
{
	static QL_Interface QLI;
	return QLI;
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
		kernel_destroyed = true;
		throw Error(error);
	}
	kernel_destroyed = false;

	m_pAgent = m_pKernel->CreateAgent("QuickLink");

	if(m_pKernel->HadError())
		throw Error(m_pKernel->GetLastErrorDescription());

	m_pKernel->RegisterForSystemEvent(smlEVENT_SYSTEM_START, MyStartSystemEventHandler, this );
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
		kernel_destroyed = true;
		throw Error(error);
	}
	kernel_destroyed = false;

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
	return true;
}


// create and set the name of the input-link object
void QL_Interface::setup_input_link(const string& name)
{
	if(!m_pAgent)
		throw Error("You must create or connect to a kernel first");

	m_id_container.insert(make_pair(name, WME_Id::create("" , "" , name , m_pAgent)));
	m_input_link_name = name;

	//build_input_link();

	update_views();
}

// identifier functions

// add an identifier
void QL_Interface::add_identifier(const string& parent_id, const string& attribute, const string& id_name)
{
	if(!m_pAgent)
		throw Error("You must create or connect to a kernel first");

	Smart_Pointer<WME_Id> parent = get_identifier(parent_id);
	Smart_Pointer<WME_Id> new_item = WME_Id::create(parent_id, attribute, id_name, m_pAgent, parent->get_id_object());

	m_id_container.insert(make_pair(id_name, new_item));
	parent->add_child(new_item);

	update_views();
}

// add an identifier that has already been created
void QL_Interface::add_created_identifier(Smart_Pointer<WME_Id> identifier)
{
	m_id_container.insert(make_pair(identifier->get_value(), identifier));
}

// check to see if a given identifier exists
bool QL_Interface::id_exists(const string& id_name)
{
	Smart_Pointer<WME_Id> ptr = m_id_container[id_name];
	if(ptr)
		return true;

	m_id_container.erase(id_name);
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

	update_views();
}

// value-wme functions

// add a value wme
void QL_Interface::add_value_wme(const string& identifier, const string& attribute, const string& value)
{
	if(!m_pAgent)
		throw Error("You must create or connect to a kernel first");

	Smart_Pointer<WME_Id> parent = get_identifier(identifier);
	parent->add_child(WME_String::create(identifier, attribute, value, m_pAgent, parent->get_id_object()));

	update_views();
}

void QL_Interface::add_value_wme(const string& identifier, const string& attribute, int value)
{
	if(!m_pAgent)
		throw Error("You must create or connect to a kernel first");

	Smart_Pointer<WME_Id> parent = get_identifier(identifier);
	parent->add_child(WME_Int::create(identifier, attribute, value, m_pAgent, parent->get_id_object()));

	update_views();
}

void QL_Interface::add_value_wme(const string& identifier, const string& attribute, double value)
{
	if(!m_pAgent)
		throw Error("You must create or connect to a kernel first");

	Smart_Pointer<WME_Id> parent = get_identifier(identifier);
	parent->add_child(WME_Float::create(identifier, attribute, value, m_pAgent, parent->get_id_object()));

	update_views();
}

// view functions
void QL_Interface::attach_view(View_Type* new_view)
{
	new_view->initialize();
	views.push_back(new_view);
}

// general functions

void QL_Interface::soar_command_line(const string& command)
{
	string info = m_pKernel->ExecuteCommandLine(command.c_str(), m_pAgent->GetAgentName());

	update_views("\n" + info + "\n");
}

void QL_Interface::clear_input_link()
{
	Smart_Pointer<WME_Id> il = m_id_container[m_input_link_name];
	il->remove_all_children(m_pAgent);

	m_id_container.clear();
	m_id_container[m_input_link_name] = il;

	update_views();
}

void QL_Interface::save_input_link(const string& filename)
{
	ofstream outfile(filename.c_str());
	if(!outfile)
		throw Error("Unable to open " + filename);

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
#if defined _WIN32 || _WIN64

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

#else

	pid_t pid = fork();

	if (pid < 0)
		return false;
	else if (pid == 0)
	{
		/*char* pStr = ;
		assert(pStr && "SOAR_LIBRARY path not set");
		if (chdir(pStr) < 0)
		assert(false && "chdir to SOAR_LIBRARY did not work");*/
		system("java -jar SoarJavaDebugger.jar -remote");
		pKernel->CheckForIncomingCommands();
		exit(1); // this forked process dies
	}
	else
		return true; // parent process continues as normal

#endif

	// wait until we are notified that the debugger is spawned
	m_pKernel->GetAllConnectionInfo();
	char const * java_debugger = "java-debugger";
	char const * ready = "ready";

	update_views("loading...");

	while(1)
	{
		Sleep(100);
		m_pKernel->GetAllConnectionInfo();
		char const * status = m_pKernel->GetAgentStatus(java_debugger);
		update_views(".");
		if(status && !strcmp(status,ready)) break;
	}

	update_views("complete!");
}


// event handlers

void MyStartSystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel)
{
	QL_Interface::instance().commit();
}

// member functions

// clean up all connection variables
void QL_Interface::prepare_for_new_connection()
{
	m_id_container.clear();

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
