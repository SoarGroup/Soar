
/* WME_Type.h
*
*  WME_Type is an Abstract interface to all of the concrete WME_Types that will be 
*  derived from this class
*
*/

#ifndef WME_TYPE
#define WME_TYPE

#include "Smart_Pointer.h"
#include "sml_Client.h"

#include <string>
#include <iosfwd>

class WME_Type : public Reference_Counted_Object
{
public:
	
	// return the name of the parent identifier
	std::string get_id_name() const
		{ return m_id; }

	// return the name of the attribute
	std::string get_attribute() const
		{ return m_attribute; }

	// this function is used by the print wmes function, it returns true if it is an identifier
	virtual bool print_object();

	// override this and return the string form of your value
	virtual std::string get_value() const = 0;

	// get rid of the actual underlying object
	virtual void destroy_sml_object(sml::Agent* pAgent) = 0;

	// output the command to create me to a file
	virtual void save_yourself(std::ofstream* outfile);

protected:

	WME_Type(const std::string& in_id, const std::string& in_attribute);
	

private:

	std::string m_id, m_attribute;
};

#endif