/* View_Console.h
*
* This class inherits from View_Type and represents the console view
*
*/

#ifndef VIEW_CONSOLE_H
#define VIEW_CONSOLE_H

#include "View_Type.h"
#include "sml_Client.h"

#include <vector>

class View_Console : public View_Type
{
public:
	// named constructor
	static Smart_Pointer<View_Console> create();

	// call this function when update for soar modifications needs to happen
	virtual void update(const id_container_t wme_ids);

	// call this function when an info strings needs to be displayed
	virtual void update_info(std::string info);

	// this function is called when the view is first attached to the QL singleton.  It should
	// register events with the singleton at this time
	virtual void initialize();

	// this will display the last output shown
	virtual void display_last_output()
	{ printOutput(); }

	// this should be called by the update event handler
	void display_output(sml::Kernel* pKernel);

	~View_Console() {}

private:
	View_Console();

	//******OUTPUT STORAGE******

	struct triple
	{
		std::string name;
		std::string att;
		std::string val;
		bool printed;
	};

	std::vector<triple> storeO;

	//*******SOAR-FORM OUTPUT SPACING TRICK******

	struct spaceControl
	{
		std::string indent;
		std::string iden;
	};

	std::vector<spaceControl> SC;

	// helper functions
	void printOutput();

};
#endif