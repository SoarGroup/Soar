/* View_Type.h
*
* This is the abstract base class that all views should inherit from
*
*/

#ifndef VIEW_TYPE_H
#define VIEW_TYPE_H

#include "Smart_Pointer.h"
#include "WME_Id.h"

#include <map>

// container for all of the identifiers
typedef std::map<std::string, Smart_Pointer<WME_Id> > id_container_t;

class View_Type : public Reference_Counted_Object
{
public:

	// see View_Console for how to properly override these
	virtual void update(const id_container_t wme_ids) = 0;
	virtual void update_info(std::string info) = 0;
	virtual void display_last_output() = 0;
	virtual void initialize() = 0;

protected:
	View_Type() {}
};

#endif

