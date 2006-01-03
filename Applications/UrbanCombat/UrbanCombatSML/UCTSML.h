#ifndef UCTSML_H_
#define UCTSML_H_

#include "../../SoarIO/ClientSML/include/sml_Client.h"
#include "../UrbanCombatInterface/UrbanCombatInterface.h"
#include "../UrbanCombatInterface/UrbanCombatEffector.h"
#include "../UrbanCombatInterface/UrbanCombatPercepts.h"

#include <vector>
#include <map>

class UCTSML
{
public:
	UCTSML(bool debugger);
	~UCTSML();
	
	void RunSimulation();
	void GetOutputLinkCommands();
	void UpdateInputLink();
	
	// flags used to control running
	bool m_StopNow, m_IsRunning;
	
	// this should be private, but has to be public for event running
	sml::Agent* pAgent;
	
private:	
	// Things needed from sml_Client.h
	sml::Kernel* pKernel;
	sml::Identifier* pInputLink;
	sml::Identifier* pOutputLink;
	
	// Things needed from UC Interface
	UrbanCombatInterface ucI;
	UrbanCombatPercepts* pPercepts;
	UrbanCombatEffector* pEffectors;	
	
	// input-link WME storage
	enum il_float_indices{ x_loc, y_loc, z_loc, x_vel, y_vel, z_vel, pitch, yaw, roll };
	std::vector< sml::FloatElement* > il_float_wmes;
	
	// helper functions
	bool LaunchDebugger();	
	void CreateInputLink();
	
	// output-link command map
	typedef void (UrbanCombatEffector::*effector_fp_t)(UrbanCombatInterface& ucI);
	typedef std::map<std::string, effector_fp_t> ol_map_t;
	ol_map_t ol_command_map;
	
	// function to load command map
	void load_command_map();
};

#endif /*UCTSML_H_*/
