/////////////////////////////////////////////////////////////////
// gSKITowersSoarAgent class file.
//
// Author: Devvan Stokes, University of Michigan
// Date  : October 2004
//
// This class is a wrapper around an actual Soar kernel agent.
// The agent automatically receives an updated view of the 
//	game world whenever  it is run (by the Update callback 
// function inherited form IOutputProcessor
/////////////////////////////////////////////////////////////////
#ifndef GSKI_HANOI_SOAR_AGENT
#define GSKI_HANOI_SOAR_AGENT

#include "gSKITowers.h"
#include "IgSKI_OutputProcessor.h"
#include "IgSKI_Agent.h"

class HanoiWorld;

class SoarAgent : public gSKI::IOutputProcessor
{
public:
	/*************************************************************
	* @brief	Sets the private state and the gSKI 
	*			automatic update flag
	* @param	inAgent	Pointer to the actual Soar agent
	* @param	inWorld	Pointer to the game world
	*************************************************************/
	SoarAgent(gSKI::IAgent* inAgent, HanoiWorld* inWorld);

	~SoarAgent();

	/*************************************************************
	* @brief	Reads the internal Soar agent's command from the
	*			output link.  There will always be a command to 
	*			read if this functions is called (because of 
	*			the callback registration process).
	*			Move the top disk from the source tower to the
	*			destination tower.
	*			Neither of the pointer params are client-owned, 
	*			so they need not be released
	*************************************************************/
	void ProcessOutput(gSKI::IWorkingMemory* wmemory, gSKI::IWMObject* object);

	/*************************************************************
	* @brief	Run the internal Soar Agent
	*************************************************************/
	void MakeMove();

private:
	gSKI::IAgent* m_Agent;
	HanoiWorld* m_World;
	SoarAgent(const SoarAgent&);
	SoarAgent operator=(const SoarAgent&);
	SoarAgent();
};

#endif //GSKI_HANOI_SOAR_AGENT
