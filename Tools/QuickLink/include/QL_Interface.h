
#ifndef QL_INTERFACE
#define QL_INTERFACE

#include "WME_Id.h"
#include "View_Type.h"
#include "Smart_Pointer.h"
#include "Utilities.h"
#include "sml_Client.h"

#include <map>
#include <cassert>
#include <list>

// container for all of the identifiers
typedef std::map<std::string, Smart_Pointer<WME_Id> > id_container_t;

// default name for the input-link identifier
const std::string c_input_link_name = "IL";

class QL_Interface
{
public:

	/*************************************************************
	* @brief Use this static method to get the instance of the QL_Interface Singleton
	*
	*************************************************************/
	static QL_Interface& instance();

	/*************************************************************
	* @brief This function creates a local kernel on the port specified.  Note: setup_input_link()
	*		 must also be called after this.  They are not implemented together to allow you to attach
	*		 views after the kernel is created.  This function must be called before views are attached.
	*
	* @param port  The number of the port the kernel should be created on.
	*************************************************************/
	void create_new_kernel(int port = 12121);

	/*************************************************************
	* @brief This function is used to create a remote connection to a kernel on the default port.
	*		 This must be followed by the specify_agent function.
	*
	* @return  A '\n' delimited string of valid agent names are returned. Use specify_agent to connect
	*		   to the appropriate agent.
	*************************************************************/
	std::string establish_remote_connection();

	/*************************************************************
	* @brief This function should only be called after calling establish_remote_connection and must
	*		 be followed by a call to setup_input_link().
	*
	* @param name  This is the name of the agent QL should connect to.  Valid names are provided
	*			   in the return value of establish_remote_connection
	*************************************************************/
	bool specify_agent(const std::string& name);

	/*************************************************************
	* @brief This function sets up the QL representation of the input-link.  This must be called
	*		 before making any changes to Soar.
	*
	* @param name  This is the case-insensitive id for the input-link that is to be used when
	*			   modifying the input-link using the commands below.
	*************************************************************/
	void setup_input_link(const std::string& name = "IL");

	/*************************************************************
	* @brief Call this function before exiting the program
	*************************************************************/
	void QL_Shutdown()
	{ prepare_for_new_connection(); }

	// get access to the agent pointer so you can register for events
	// TODO: encapsulate this somehow
	sml::Agent* get_agent_ptr()
	{ return m_pKernel->GetAgent(m_pAgent->GetAgentName()); }

	/*************************************************************
	* @brief This returns the name of the agent
	*************************************************************/
	std::string get_agent_name()
	{ return m_pAgent->GetAgentName(); }

	// identifier functions

	/*************************************************************
	* @brief This function adds an identifier to the input-link in the form 
	*		 parent_id ^attribue id_name
	*
	* @param parent_id  This is the case-insensitive QL-unique identifier that should be the parent of this id.
	*				    For instance, if you used the default value for the input link name, this
	*					would be "il"
	* @param attribute  This is the attribute value of the identifier
	* @param id_name	This is the case-insensitive QL-unique identifier that will be used to identify this identifier when
	*					changes (adds, deletes, etc.) need to be made to it.
	*************************************************************/
	void add_identifier(const std::string& parent_id, const std::string& attribute, const std::string& id_name);
	
	// this function takes a pre-made smart_pointer to an identifier.  This should only be called
	// from the WME_Id class
	void add_created_identifier(Smart_Pointer<WME_Id> identifier);

	/*************************************************************
	* @brief This function deletes an identifier to the input-link in the form 
	*		 parent_id ^attribue id_name
	*
	* @param parent_id  This is the case-insensitive QL-unique identifier that should be the parent of this id.
	*				    For instance, if you used the default value for the input link name, this
	*					would be "il"
	* @param attribute  This is the attribute value of the identifier
	* @param id_name	This is the case-insensitive QL-unique identifier that was supplied when the wme was created
	*************************************************************/
	void delete_identifier(const std::string& parent_id, const std::string& attribute, const std::string& id_name);

	/*************************************************************
	* @brief This function returns true if there is an identifier with the name provided that exists, and false if
	*		 there isn't.  This is the case-insensitive QL-unique id provided when the id was created.
	*
	* @param name  This is the case-insensitive QL-unique id provided when the id was created.
	*************************************************************/
	bool id_exists(const std::string& id_name);

	// value-wme functions
	
	/*************************************************************
	* @brief These functions adds a string, int or float wme depending on the value supplied in the form
	*        identifier ^attribute value
	*
	* @param identifier  The case-insensitive QL-unique name of the identifier this structure should be added to
	* @param attribute   The attribute name of this wme
	* @param value		 The type-sensitive value of this wme.
	*************************************************************/
	void add_value_wme(const std::string& identifier, const std::string& attribute, const std::string& value);
	void add_value_wme(const std::string& identifier, const std::string& attribute, int value);
	void add_value_wme(const std::string& identifier, const std::string& attribute, double value);

	/*************************************************************
	* @brief These functions deletes a string, int or float wme depending on the value supplied in the form
	*        identifier ^attribute value
	*
	* @param identifier  The case-insensitive QL-unique name of the identifier this structure should be deleted from.
	* @param attribute   The attribute name of this wme
	* @param value		 The type-sensitive value of this wme.
	*************************************************************/
	template <typename T>
	void delete_value_wme(const std::string& identifier, const std::string& attribute, T value);

	/*************************************************************
	* @brief These functions changes the value of a string, int or float wme depending on the value supplied in the form
	*        identifier ^attribute old_value to identifier ^attribute new_value
	*
	* @param identifier  The case-insensitive QL-unique name of the identifier this structure should be changed on.
	* @param attribute   The attribute name of this wme
	* @param old_value	 The current value of this wme, to be replaced by new_value.
	* @param new_value   The value to replace the current wme value with.
	*************************************************************/
	template <typename T>
	void update_value_wme(const std::string& identifier, const std::string& attribute, T old_value, T new_value);

	/*************************************************************
	* @brief This function returns true if the wme supplied exists
	*
	* @param identifier  The case-insensitive QL-unique name of the identifier this structure should be found on.
	* @param attribute   The attribute name of this wme
	* @param value		 The type-sensitive value of this wme.
	*************************************************************/
	template <typename T>
	bool wme_exists(const std::string& identifier, const std::string& attribute, T value);

	/*************************************************************
	* @brief This will execute the command provided on Soar's command line interface
	*
	* @param command  The command to be executed.
	*************************************************************/
	void soar_command_line(const std::string& command);

	// View Functions

	/*************************************************************
	* @brief This function adds a view that will receive updates from this module when updates are needed.  See
	*		 the View_Type class for more information.  The reason this is done is to give all displays a plugin 
	*		 interface, so they can be very independant.
	*
	* @param new_view  This is a class that you want to receive updates when they are issued.  See the View_Type 
	*				   class for more info.
	*************************************************************/
	void attach_view(View_Type* new_view);

	/*************************************************************
	* @brief This function will turn off updates, this is usually done when a lot of output is generated that
	*		 you don't want displayed.
	*************************************************************/
	void turn_off_updates()
	{ should_update_views = false; }

	/*************************************************************
	* @brief This function will turn on updates, this is usually done after calling turn_off_updates when you
	*		 want the updates to come back again.
	*************************************************************/
	void turn_on_updates()
	{ should_update_views = true; }

	/*************************************************************
	* @brief This function will issue an update of all the views, this is usually done after calling turn_on_updates when you
	*		 want the views to be updated.
	*************************************************************/
	void update_views_now()
	{ update_views(); }

	/*************************************************************
	* @brief This command issues a call to all the views to print there last outputs.
	*************************************************************/
	void print_last_output();

	// Maintenance functions

	/*************************************************************
	* @brief Clears the current input-link
	*************************************************************/
	void clear_input_link();

	/*************************************************************
	* @brief Saves the current input-link to the filename provided.
	*************************************************************/
	void save_input_link(const std::string& filename);

	/*************************************************************
	* @brief Commit the changes you have made to the input-link by adds, updates and deletes
	*************************************************************/
	void commit()
	{ m_pAgent->Commit(); }

	/*************************************************************
	* @brief This will spawn the JavaDebugger
	*************************************************************/
	void spawn_debugger();

	// Run operations

	/*************************************************************
	* @brief This will run soar until output is generated
	*************************************************************/
	void run_til_output() 
	{ m_pAgent->RunSelfTilOutput(); }

private:

	// things need to be kept private for singleton implementation
	QL_Interface() : should_update_views(true), kernel_destroyed(true) {}
	~QL_Interface() {} 
	QL_Interface(const QL_Interface&);
	QL_Interface& operator= (const QL_Interface&);

	id_container_t m_id_container;

	typedef std::list<Smart_Pointer<View_Type> > Views_t;
	Views_t views;

	std::string m_input_link_name;
	bool should_update_views;
	bool kernel_destroyed;

	// sml stuff
	sml::Kernel* m_pKernel;
	sml::Agent* m_pAgent;

	// member functions

	// build up the children of an input-link that already exists
	void build_input_link();

	// call this before establishing a new connection
	void prepare_for_new_connection();

	// return the identifier with the specified name
	Smart_Pointer<WME_Id> get_identifier(std::string id);

	// update all of the views
	void update_views();
	void update_views(std::string info);

};

template <typename T>
bool QL_Interface::wme_exists(const std::string& identifier, const std::string& attribute, T value)
{
	Smart_Pointer<WME_Id> id = get_identifier(identifier);
	assert(id->get_id_object());
	return id->has_child(attribute, value);
}

template <typename T>
void QL_Interface::delete_value_wme(const std::string& identifier, const std::string& attribute, T value)
{
	Smart_Pointer<WME_Id> parent = get_identifier(identifier);
	parent->remove_wme_child(attribute, value, m_pAgent);

	update_views();
}

template <typename T>
void QL_Interface::update_value_wme(const std::string& identifier, const std::string& attribute, T old_value, T new_value)
{
	Smart_Pointer<WME_Id> parent = get_identifier(identifier);
	parent->update_wme_child(attribute, old_value, new_value,  m_pAgent);

	update_views();
}

#endif