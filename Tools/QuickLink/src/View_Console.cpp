
#include "View_Console.h"
#include "Smart_Pointer.h"
#include "WME_Id.h"
#include "QL_Interface.h"
#include "sml_Client.h"
#include "Input_Controller.h"

#include <list>
#include <iostream>

using std::list; using std::string; using std::cout; using std::endl;
using sml::Agent; using sml::Kernel; using sml::smlUpdateEventId; using sml::smlRunFlags;
using sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES;

void MyUpdateEventHandler(smlUpdateEventId id, void* pUserData, Kernel* pKernel, smlRunFlags runFlags);

// named constructor
Smart_Pointer<View_Console> View_Console::create()
{
	Smart_Pointer<View_Console> ptr = new View_Console();
	return ptr;
}

View_Console::View_Console()
: View_Type()
{}

// structure used to maintain pairs of the current id and its proper indentation length
struct ptr_indent {
	ptr_indent(id_container_t::const_iterator in_id, string in_indent)
		: id(in_id), indent(in_indent)
	{}

	id_container_t::const_iterator id;
	string indent;
};

void View_Console::update(const id_container_t wme_ids)
{
	typedef list<ptr_indent> id_queue_t;
	id_queue_t id_queue; // queue of id's to be printed
	// push back the input link
	id_queue.push_back(ptr_indent(wme_ids.find(c_input_link_name), ""));

	cout << endl << "***** Current Input-Link Structure *****" << endl << endl;

	while(id_queue.size() > 0)
	{
		// get the top identifier, pop it and print it
		id_queue_t::iterator top = id_queue.begin();
		cout << top->indent << "(" << top->id->first;
		
		// get the identifer's children
		Smart_Pointer<WME_Id> top_ptr = top->id->second;
		all_children_t children = top_ptr->notify_of_children();

		for(all_children_t::iterator it = children.begin(); it != children.end(); it++)
		{
			// print all children, 
			if((*it)->print_object()) // returns true if it has children to print
			{
				string val = (*it)->get_value();
				Smart_Pointer<WME_Id> id = (*(wme_ids.find(val))).second;
				id_queue.push_back(ptr_indent(wme_ids.find((*it)->get_value()), top->indent + "  "));
			}
		}


		cout << ")" << endl;

		id_queue.pop_front();
	}
}

// this is called if a string needs to be printed to console
void View_Console::update_info(string info)
{
	cout << info;
}

// on initialization, register for update event
void View_Console::initialize()
{
	Agent* pAgent = QL_Interface::instance().get_agent_ptr();
	pAgent->GetKernel()->RegisterForUpdateEvent(smlEVENT_AFTER_ALL_OUTPUT_PHASES, MyUpdateEventHandler, this);
}

// display the output
void MyUpdateEventHandler(smlUpdateEventId, void* pUserData, Kernel* pKernel, smlRunFlags)
{
	View_Console* vc = (View_Console*)pUserData;
	vc->display_output(pKernel);
}

void View_Console::display_output(Kernel* pKernel)
{
	storeO.resize(0);

	//******GET OUTPUT******	

	string agent_name = QL_Interface::instance().get_agent_name();
	int numberCommands = pKernel->GetAgent(agent_name.c_str())->GetNumberOutputLinkChanges() ;

	// go through all changes and capture the information
	for(int i = 0; i < numberCommands; i++)
	{
		if(pKernel->GetAgent(agent_name.c_str())->IsOutputLinkChangeAdd(i))
		{
			triple trip;
			sml::WMElement* tmp = pKernel->GetAgent(agent_name.c_str())->GetOutputLinkChange(i) ;
			trip.name = tmp->GetIdentifierName() ;
			trip.att = tmp->GetAttribute();
			trip.val = tmp->GetValueAsString();
			trip.printed = false;
			storeO.push_back(trip);
		}		
	}

	printOutput();
	
	if(Input_Controller::instance().should_print_prompt)
		cout << endl << endl << "> ";
	Input_Controller::instance().should_print_prompt = true;
	pKernel->GetAgent(agent_name.c_str())->ClearOutputLinkChanges();
}

void View_Console::printOutput()
{
	cout << endl << endl << "******OUTPUT******" << endl;
	
	for(unsigned int i = 0; i < storeO.size(); i++)
	{
		if(storeO[i].printed == false)
		{
			string indent = "";
			string iden;
			iden = storeO[i].name;
			for(unsigned int s = 0; s < SC.size(); s++)
				if(iden == SC[s].iden)
					indent = ("  " + SC[s].indent);
			spaceControl tmp;
			tmp.iden = storeO[i].val;
			tmp.indent = indent;
			SC.push_back(tmp);

			cout << indent << "(" << iden << " ^" << storeO[i].att << " " << storeO[i].val;
			storeO[i].printed = true;
			int returner = 1;  //controls word wrapping
			for(unsigned int j = 0; j < storeO.size(); j++)
			{
				if(storeO[j].name == iden && !storeO[j].printed)
				{
					if(returner > 3)  //controls word wrapping
					{
						cout << endl << indent;
						returner = 0;  //controls word wrapping
					}
					else
					{
						returner++;
						cout << " ";
					}
					cout << "^" << storeO[j].att << " " << storeO[j].val << " ";
					storeO[j].printed = true;
					spaceControl tmp2;
					tmp2.iden = storeO[j].val;
					tmp2.indent = indent;
					SC.push_back(tmp2);
				}
			}
			cout << ")" << endl;
		}
	}
	for(unsigned int i = 0; i < storeO.size(); i++)  //resets all printed flags to false
		storeO[i].printed = false;

	cout << endl << endl << "******END OF OUTPUT******" << endl << endl;

	

	QL_Interface::instance().update_views_now();
}

