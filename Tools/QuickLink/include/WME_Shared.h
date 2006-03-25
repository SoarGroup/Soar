
#ifndef WME_SHARED
#define WME_SHARED

#include "WME_Type.h"

class WME_Shared : public WME_Type
{
public:
	//named/virtual constructor
	static Smart_Pointer<WME_Shared> create(const std::string& loop_start, const std::string& in_attribute, const std::string& loop_end, sml::Agent* pAgent, sml::Identifier* id_start, sml::Identifier* id_end);
	static Smart_Pointer<WME_Shared> create_from_sml_object(const std::string& in_id_name, sml::Identifier* object);

	virtual bool print_object();

	virtual void destroy_sml_object(sml::Agent* pAgent);

	virtual void save_yourself(std::ofstream* outfile);

	virtual std::string get_value() const
	{return m_value;}

private:
	WME_Shared(const std::string& in_parent, const std::string& in_attribute, const std::string& in_value, sml::Agent* pAgent, sml::Identifier* id_start, sml::Identifier* id_end);
	WME_Shared(const std::string& in_id_name, const std::string& in_attribute, sml::Identifier* in_object);
	~WME_Shared();

	std::string m_value;
	sml::Identifier* m_object;
};
#endif

