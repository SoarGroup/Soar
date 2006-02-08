
#ifndef WME_ID
#define WME_ID

#include "WME_Int.h"
#include "WME_Float.h"
#include "WME_String.h"
#include "Smart_Pointer.h"
#include "sml_Client.h"
#include "Utilities.h"

#include <map>
#include <set>

// A comparison function for the all_children_t set
struct less_WME_Type_ptrs {
	bool operator()(const Smart_Pointer<WME_Type> one, const Smart_Pointer<WME_Type> two) const
	{
		std::string one_key = make_key(one->get_id_name(), one->get_attribute(), one->get_value());
		std::string two_key = make_key(two->get_id_name(), two->get_attribute(), two->get_value());
		return one_key < two_key;
	}
};

typedef std::set<Smart_Pointer<WME_Type>, less_WME_Type_ptrs > all_children_t;

class WME_Id : public WME_Type
{
public:

	// named/virtual constructor
	static Smart_Pointer<WME_Id> create(const std::string& in_parent, const std::string& in_attribute, const std::string& in_value, sml::Agent* pAgent, sml::Identifier* in_id = 0);
	static Smart_Pointer<WME_Id> create_from_sml_object(const std::string& in_id_name, sml::Identifier* object);

	// return the value (name of this identifier)
	virtual std::string get_value() const
	{ return m_value; }

	// return a pointer to the object
	sml::Identifier* get_id_object()
	{ return m_object; }

	// return a const map of all the children
	const all_children_t notify_of_children();

	// this function is used by the print wmes function, it returns true if it is an identifier
	virtual bool print_object();

	// get rid of the actual underlying object
	virtual void destroy_sml_object(sml::Agent* pAgent)
	{ pAgent->DestroyWME(m_object); }

	// add child
	void add_child(Smart_Pointer<WME_Int> in_child);
	void add_child(Smart_Pointer<WME_Float> in_child);
	void add_child(Smart_Pointer<WME_String> in_child);
	void add_child(Smart_Pointer<WME_Id> in_child);

	// return true if given child exists
	bool has_child(std::string attribute, std::string value);
	bool has_child(std::string attribute, int value);
	bool has_child(std::string attribute, double value);

	// remove child
	void remove_wme_child(const std::string& att, const std::string& value, sml::Agent* pAgent);
	void remove_wme_child(const std::string& att, int value, sml::Agent* pAgent);
	void remove_wme_child(const std::string& att, double value, sml::Agent* pAgent);
	void remove_id_child(const std::string& att, const std::string& value, sml::Agent* pAgent);
	void remove_all_children(sml::Agent* pAgent);

	// update child
	void update_wme_child(const std::string& att, const std::string& old_value, const std::string& new_value, sml::Agent* pAgent);
	void update_wme_child(const std::string& att, int old_value, int new_value, sml::Agent* pAgent);
	void update_wme_child(const std::string& att, double old_value, double new_value, sml::Agent* pAgent);

	// output command to create your structure to file
	void save_yourself(std::ofstream* outfile);


	~WME_Id() {}

private:

	WME_Id(const std::string& in_parent, const std::string& in_attribute, const std::string& in_value, sml::Agent* pAgent, sml::Identifier* in_id = 0);
	WME_Id(const std::string& in_id_name, const std::string& in_attribute, sml::Identifier* in_object);

	std::string m_value;
	sml::Identifier* m_object;
	all_children_t m_all_children;

    typedef	std::map<std::string, Smart_Pointer<WME_Float> > float_children_t;
	float_children_t m_float_children;

	typedef	std::map<std::string, Smart_Pointer<WME_Int> > int_children_t;
	int_children_t m_int_children;

	typedef	std::map<std::string, Smart_Pointer<WME_String> > string_children_t;
    string_children_t m_string_children;

	typedef std::map<std::string, Smart_Pointer<WME_Id> > id_children_t;
	id_children_t m_id_children;

	// member functions
	void build_children();
	
	
	
};



#endif

