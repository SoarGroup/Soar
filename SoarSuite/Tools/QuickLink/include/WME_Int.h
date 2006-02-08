
#ifndef WME_INT
#define WME_INT

#include "WME_Type.h"
#include "sml_Client.h"

class WME_Int : public WME_Type
{
public:

	// named/virtual constructor
	static Smart_Pointer<WME_Int> create(const std::string& in_id_name, const std::string& in_attribute, const int in_value, sml::Agent* pAgent, sml::Identifier* in_id);
	static Smart_Pointer<WME_Int> create_from_sml_object(const std::string& in_id_name, sml::IntElement* object);

	// return the value (name of this identifier)
	virtual std::string get_value() const
	{ return m_object->GetValueAsString(); }

	// this function is used by the print wmes function, it returns true if it is an identifier
	virtual bool print_object();

	// get rid of the actual underlying object
	virtual void destroy_sml_object(sml::Agent* pAgent)
	{ pAgent->DestroyWME(m_object); }

	// update the actual sml object
	void update_sml_object(int new_value, sml::Agent* pAgent)
	{ pAgent->Update(m_object, new_value); }

	// output information to recreate object
	void save_yourself(std::ofstream* outfile);

	~WME_Int() {}

private:

	WME_Int(const std::string& in_id_name, const std::string& in_attribute, const int in_value, sml::Agent* pAgent, sml::Identifier* in_id);
	WME_Int(const std::string& in_id_name, const std::string& in_attribute, sml::IntElement* in_object);

	sml::IntElement* m_object;
};

#endif