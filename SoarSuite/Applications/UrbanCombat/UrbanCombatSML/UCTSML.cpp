#include "UCTSML.h"

#include <iostream>
#include <cassert>

using std::cout; using std::endl; using std::cerr;
using sml::Identifier; using sml::Kernel; using sml::smlSystemEventId;
using sml::smlUpdateEventId; using sml::smlRunFlags; using sml::WMElement;
using std::string;

void MyUpdateEventHandler(smlUpdateEventId id, void* pUserData, Kernel* pKernel, smlRunFlags runFlags);
void MyStartSystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel);
void MyStopSystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel);


// constants used for output-link matching
const string ol_com_walk = "walk";
const string ol_com_turn = "turn";
const string ol_com_strafe = "strafe";
const string ol_com_do = "do";
const string ol_com_nothing = "nothing";
const string ol_dir_forward = "forward";
const string ol_dir_backward = "backward";
const string ol_dir_right = "right";
const string ol_dir_left = "left";


UCTSML::UCTSML(bool debugger) : m_StopNow(false), m_IsRunning(false)
{	
	// Create a new kernel
	pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML", 12121);
	
	// check for errors
	if(pKernel->HadError())
	{
		cerr << "Unable to create new kernel, something is fundamentally wrong, exiting..." << endl;
		exit(0);
	}
	
	assert(pKernel);
	
	// Create an Agent
	pAgent = pKernel->CreateAgent("CombatSoar");
	
	// check for errors
	if(pKernel->HadError())
	{
		cerr << "Unable to create Agent, something is fundamentally wrong, exiting..." << endl;
		exit(0);
	}
	
	assert(pAgent);
	
	// get the input and output links
	pInputLink = pAgent->GetInputLink();
	//pOutputLink = pAgent->GetOutputLink();
	
	assert(pInputLink);
	//assert(pOutputLink);
	
	// get the percepts
	pPercepts = UrbanCombatPercepts::Instance();
	assert(pPercepts);
	
	// get the effectors
	pEffectors = UrbanCombatEffector::Instance();
	assert(pEffectors);
	
	// TODO: add system command to lauch the debugger
	// launch the debugger
	if(debugger)
		LaunchDebugger();
	load_command_map();
}

UCTSML::~UCTSML()
{
//	cerr << "calling destructor" << endl;
	pKernel->Shutdown();
	delete pKernel;
}

bool UCTSML::LaunchDebugger()
{
	pid_t pid = fork();
    
    if (pid < 0)
        return false;
    else if (pid == 0)
    {
        char* pStr = getenv("SOAR_LIBRARY");
        assert(pStr && "SOAR_LIBRARY path not set");
        if (chdir(pStr) < 0)
            assert(false && "chdir to SOAR_LIBRARY did not work");
        system("java -jar SoarJavaDebugger.jar -remote");
        pKernel->CheckForIncomingCommands();
        exit(1); // this forked process dies
    }
    else
        return true; // parent process continues as normal
}

// Call this function to run the simulation
void UCTSML::RunSimulation()
{
	cerr << "calling RunSimulation()" << endl;
	
	CreateInputLink();
	
	// register for relevant events
	pKernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, MyUpdateEventHandler, this);
	pKernel->RegisterForSystemEvent(sml::smlEVENT_SYSTEM_START, MyStartSystemEventHandler, this);
	pKernel->RegisterForSystemEvent(sml::smlEVENT_SYSTEM_STOP, MyStopSystemEventHandler, this);
	
}

// the following 3 functions are globabl scope event handlers used for running
void MyUpdateEventHandler(smlUpdateEventId id, void* pUserData, Kernel* pKernel, smlRunFlags runFlags)
{
	UCTSML* uctsml = (UCTSML*)pUserData;
	if(uctsml->m_StopNow)
	{
		cerr << "stopping now" << endl;
		pKernel->StopAllAgents();
		uctsml->m_StopNow = false;
		uctsml->m_IsRunning = false;
	}
	//pKernel->CheckForIncomingCommands();
	if(uctsml->pAgent->GetNumberCommands() != 0)
		uctsml->GetOutputLinkCommands();
	uctsml->UpdateInputLink();
	uctsml->pAgent->Commit();
}

void MyStopSystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel)
{
	if(id == sml::smlEVENT_SYSTEM_STOP)
	{
//		UCTSML* uctsml = (UCTSML*)pUserData;
//		uctsml->m_StopNow = true;
//		cerr << "set stop to true" << endl;
	}
}

void MyStartSystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel)
{
	UCTSML* uctsml = (UCTSML*)pUserData;
	uctsml->m_IsRunning = true;
}

// this function takes commands from the output-link, parses them, and calls
// corresponding commands in UCT
void UCTSML::GetOutputLinkCommands()
{
	int numCommands = pAgent->GetNumberCommands();
	for(int i = 0; i < numCommands; i++)
	{
		Identifier* command = pAgent->GetCommand(i); // get each command
		
		// NOTE: this assumes each command will only have 1 child
		WMElement* details = command->GetChild(0);
		string com_name = command->GetAttribute();
		string com_detail = details->GetValueAsString();
		effector_fp_t action = ol_command_map[com_name+com_detail];
		//cerr << com_name+com_detail << endl;
		if(action)
			(pEffectors->*action)(ucI);
		else
			cerr << "Magic ol_command_map didn't work";
		command->AddStatusComplete();
	}
	pAgent->ClearOutputLinkChanges();
	
}

// this function is used to load the output-link command map with all possible
// UCT commands that could be called
void UCTSML::load_command_map()
{
	ol_command_map[ol_com_walk+ol_dir_forward] = &UrbanCombatEffector::WalkForward;
	ol_command_map[ol_com_walk+ol_dir_backward] = &UrbanCombatEffector::WalkBackwards;
	ol_command_map[ol_com_turn+ol_dir_right] = &UrbanCombatEffector::TurnRight;
	ol_command_map[ol_com_turn+ol_dir_left] = &UrbanCombatEffector::TurnLeft;
	ol_command_map[ol_com_strafe+ol_dir_right] = &UrbanCombatEffector::StrafeRight;
	ol_command_map[ol_com_strafe+ol_dir_left] = &UrbanCombatEffector::StrafeLeft;
	ol_command_map[ol_com_do+ol_com_nothing] = &UrbanCombatEffector::DoNothing;
}

// Uses the percepts to update the information on the input-link
void UCTSML::UpdateInputLink()
{
	float x = 0.0, y = 0.0, z = 0.0;
	
	// the order of these are not important, if you are adding something here
	// though, make sure you have kept CreateInputLink in order
	pPercepts->GetLocation(ucI, x, y, z);
	pAgent->Update(il_float_wmes[x_loc], x);
	pAgent->Update(il_float_wmes[y_loc], y);
	pAgent->Update(il_float_wmes[z_loc], z);
	
	pPercepts->GetAngles(ucI, x, y, z);
	pAgent->Update(il_float_wmes[pitch], x);
	pAgent->Update(il_float_wmes[yaw], y);
	pAgent->Update(il_float_wmes[roll], z);
	
	pPercepts->GetVelocity(ucI, x, y, z);
	pAgent->Update(il_float_wmes[x_vel], x);
	pAgent->Update(il_float_wmes[y_vel], y);
	pAgent->Update(il_float_wmes[z_vel], z);
}
// creates the structure of the input-link and saves the appropriate wmes
void UCTSML::CreateInputLink()
{
	// the order of all of these is very important, it corresponds to index enums
	// in UCTSML.h
	Identifier* location = pAgent->CreateIdWME(pInputLink, "location");
	il_float_wmes.push_back(pAgent->CreateFloatWME(location, "x", 0.0));
	il_float_wmes.push_back(pAgent->CreateFloatWME(location, "y", 0.0));
	il_float_wmes.push_back(pAgent->CreateFloatWME(location, "z", 0.0));
	
	Identifier* velocity = pAgent->CreateIdWME(pInputLink, "velocity");
	il_float_wmes.push_back(pAgent->CreateFloatWME(velocity, "x", 0.0));
	il_float_wmes.push_back(pAgent->CreateFloatWME(velocity, "y", 0.0));
	il_float_wmes.push_back(pAgent->CreateFloatWME(velocity, "z", 0.0));
	
	Identifier* angles = pAgent->CreateIdWME(pInputLink, "angles");
	il_float_wmes.push_back(pAgent->CreateFloatWME(angles, "pitch", 0.0));
	il_float_wmes.push_back(pAgent->CreateFloatWME(angles, "yaw", 0.0));
	il_float_wmes.push_back(pAgent->CreateFloatWME(angles, "roll", 0.0));	
}

int main()
{
	UCTSML uctsml(true);
	uctsml.RunSimulation();
	std::string wait;
	std::cin >> wait;
	std::cout << wait;
	
	return 0;
}
