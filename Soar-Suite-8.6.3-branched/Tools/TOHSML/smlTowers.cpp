#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

#include "smlTowers.h"

//standard directives
#include <cassert>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>

using std::cout;
using std::cerr;
using std::cin;
using std::endl;
using std::string;
using std::vector;

// SML directives
#include "sml_Client.h"

//SGIO directives
/*
#include "sgio_wmemem.h"
#include "sgio_soar.h"
#include "sgio_siosoar.h"
#include "sgio_apisoar.h"

using sgio::Soar;
using sgio::SIOSoar;
using sgio::APISoar;
using sgio::Agent;
using sgio::WorkingMemory;
using sgio::SoarId;
using sgio::StringElement;
using sgio::IntElement;
*/

//
#include "smlTowersSoarAgent.h"
//using sgio_towers::SoarAgent;



/*************************************************************
* @brief	The TowerInputLinkProfile class contains all of the  
*			wmes corresponding to the Tower
*************************************************************/
class TowerInputLinkProfile
{
	/*************************************************************
	* @brief	Initialize private data to zero.  This constructor
	*			can only be called by friends
	*************************************************************/
	TowerInputLinkProfile(Agent* inWorkingMemory, char inName) : pWorkingMemory(inWorkingMemory)
	{
		string tempName;
		tempName += inName;

		// SML takes char const*
		m_pPegName = pWorkingMemory->CreateStringWME(pWorkingMemory->GetILink(), k_worldPegString, tempName.c_str());
		pWorkingMemory->Commit();
	}

	~TowerInputLinkProfile()
	{
		//Release the top WME, all the rest go automatically
		pWorkingMemory->DestroyWME(m_pPegName);
		pWorkingMemory->Commit();

		//this object doesn't own WM, can't delete it
		pWorkingMemory = 0;
	}

	// SGIO has a separate WorkingMemory object.  In SML this is part of the Agent.
	Agent* pWorkingMemory; //working memory is not owned by this object
	StringElement* m_pPegName;

	friend class Tower;
};

class DiskInputLinkProfile
{
public:

	~DiskInputLinkProfile()
	{
		pWMemory->DestroyWME(m_pHoldsIdentifier);
		pWMemory->DestroyWME(m_pDiskIdentifier);
		Commit();
		m_pHoldsIdentifier	= 0;
			m_pDisk			= 0;
			m_pDiskBeneathAsInteger	= 0;
			m_pDiskBeneathAsString	= 0;
		m_pDiskIdentifier	= 0;
	}

	void SetDiskBeneath(Disk* inDisk, Tower* inTower)
	{
		pActualDiskBeneath = inDisk;
		currentPeg = inTower->GetName();
		holdsNeedsToBeUpdated = true;
	}


	void Commit()
	{
		pWMemory->Commit();
	}

	void Update()
	{
		if(!holdsNeedsToBeUpdated)
			return;

		pWMemory->Update(m_pPeg, currentPeg.c_str());

		//if there's no disk currently beneath
		if(pActualDiskBeneath == 0)
		{
			//and we already have a "none" string, don't update
			if(m_pDiskBeneathAsString != 0)
				assert(m_pDiskBeneathAsInteger == 0);			

			//remove the "none string", put the integer for size there
			else
			{
				assert(m_pDiskBeneathAsInteger);// we had to have an integer here before
				pWMemory->DestroyWME(m_pDiskBeneathAsInteger);
				m_pDiskBeneathAsInteger = 0;
				assert(m_pDiskBeneathAsString == 0);
				m_pDiskBeneathAsString = pWMemory->CreateStringWME(m_pHoldsIdentifier, k_holdsAboveString, k_noneString);
				assert(m_pDiskBeneathAsString);
			}
		}

		else // there is a disk beneath
		{
			//and we already have a "none" string, get rid of it
			if(m_pDiskBeneathAsString != 0)
			{
				assert(m_pDiskBeneathAsInteger == 0);
				pWMemory->DestroyWME(m_pDiskBeneathAsString);
				m_pDiskBeneathAsString = 0;
				assert(m_pDiskBeneathAsInteger == 0);
				m_pDiskBeneathAsInteger = pWMemory->CreateIntWME(m_pHoldsIdentifier, k_holdsAboveString, pActualDiskBeneath->GetSize());
				assert(m_pDiskBeneathAsInteger);
			}

			//update the int that's already there
			else
			{
				assert(m_pDiskBeneathAsInteger);
				assert(m_pDiskBeneathAsString == 0);

				//do the update only if we're actually on top of a different disk than last time
				if(m_pDiskBeneathAsInteger->GetValue() != pActualDiskBeneath->GetSize())
					pWMemory->Update(m_pDiskBeneathAsInteger, pActualDiskBeneath->GetSize());
			}
		}

		Commit();
	}

private:

	DiskInputLinkProfile(Agent* inWMemory, Disk* inDisk, Disk* inDiskBeneath) : pWMemory(inWMemory), 
																						pActualDiskBeneath(inDiskBeneath)
	{
		m_size = inDisk->GetSize();

		currentPeg = inDisk->GetTower()->GetName();

		m_pHoldsIdentifier	= pWMemory->CreateIdWME(pWMemory->GetILink(), k_holdsIdentifierString);
		m_pPeg			    = pWMemory->CreateStringWME(m_pHoldsIdentifier, k_holdsOnString, currentPeg.c_str());
			m_pDisk			= pWMemory->CreateIntWME(m_pHoldsIdentifier, k_diskIdentifierString, m_size);
			if(inDiskBeneath)
			{
				m_pDiskBeneathAsInteger	= pWMemory->CreateIntWME(m_pHoldsIdentifier, k_holdsAboveString, inDiskBeneath->GetSize());
				m_pDiskBeneathAsString = 0;
			}
			else
			{
				m_pDiskBeneathAsString = pWMemory->CreateStringWME(m_pHoldsIdentifier, k_holdsAboveString, k_noneString);
				m_pDiskBeneathAsInteger = 0;
			}
		
		m_pDiskIdentifier	= pWMemory->CreateIntWME(pWMemory->GetILink(), k_diskIdentifierString, m_size);

		Commit();
		holdsNeedsToBeUpdated = false;
	}

	// SML does not have a separate WorkingMemory object
	Agent* pWMemory;

	// SML uses Identifier instead of SoarId
	Identifier* m_pHoldsIdentifier;
		StringElement* m_pPeg;					//name of peg that the disk is on
		IntElement* m_pDisk;					//size and name of disk
		IntElement* m_pDiskBeneathAsInteger;	// size/name of disk beneath this one
		StringElement* m_pDiskBeneathAsString;	// will be "none" when this disk has nothing beneath it

	IntElement* m_pDiskIdentifier;				//size and name of disk

	int m_size;
	Disk* pActualDiskBeneath;
	string currentPeg;
	bool holdsNeedsToBeUpdated;

	friend class Disk;
};

/*************************************************************
* @brief	The IOManager class contains WorkingMemory
*			and Soar pointers, which it is responsible for 
*			cleaning up, and has wrapper functions for
*			manipulating the input link
*************************************************************/
class IOManager
{
public:
	Agent* GetAgent() const
	{
		return pAgent;
	}

	Agent* GetWorkingMemory() const
	{
		return pWMemory;
	}
private:
	IOManager(Kernel* inSoar) : pSoar(inSoar)
	{
		pAgent = pSoar->CreateAgent("sgioTowersAgent");

		// SML uses a more explicit error model, so we can get details about what failed.
		// We check the Kernel object in a create agent call, because we may have a null agent
		// returned.
		if (pSoar->HadError())
		{
			// This string will contain a description of the error
			string error = pSoar->GetLastErrorDescription() ;
			assert (!pSoar->HadError()) ;
		}

		assert(pAgent);

		//Load agent's productions
		pAgent->LoadProductions("../Tests/test-towers-of-hanoi-SML.soar");

		// SML uses a more explicit error model, so we can get details about what failed.
		if (pAgent->HadError())
		{
			// This string will contain a description of the error.
			string error = pAgent->GetLastErrorDescription() ;
			cerr << error << endl;
			//if this asserts, check your working directory
			assert (!pAgent->HadError()) ;
		}

		// SML does not use a separate WorkingMemory object.  We just have an Agent instead.
		// pWMemory = new WorkingMemory(pAgent);
		pWMemory = pAgent ;
		assert(pWMemory);

	}
	~IOManager()
	{
		// In SML there is no separate WorkingMemory object and Agents are (as with SGIO) owned by the Kernel.
		// delete pWMemory;
		delete pSoar;
	}

	// SML uses Kernel instead of Soar as the type name
	Kernel* pSoar;		//this object assumes ownership of Soar, and must clean it up
	Agent* pAgent;		//this object does not own the agent and does not clean it up
	Agent* pWMemory;	//this object assumes ownership of WM, and must clean it up

	friend class HanoiWorld;
};




//======================================================
//============ Disk Function Definitions ===============

Disk::Disk(Tower* inTower, int inSize, Disk* inDiskBeneath) : m_size(inSize), pTower(inTower)
{
	assert(pTower->GetWorld());
	assert(pTower->GetWorld()->GetIOManager());
	m_iLinkProfile = new DiskInputLinkProfile(pTower->GetWorld()->GetIOManager()->GetWorkingMemory(), this, inDiskBeneath);
	assert(m_iLinkProfile);	
}

void Disk::Detach()
{
	delete m_iLinkProfile;
}


void Disk::Update(Disk* diskBeneathMe, Tower* tower)
{
	assert(tower);
	pTower = tower;
	m_iLinkProfile->SetDiskBeneath(diskBeneathMe, tower);
	m_iLinkProfile->Update();
}

//======================================================
//============ Tower Function Definitions ==============

Tower::Tower(HanoiWorld* world, char name) : m_name(name), pWorld(world)
{
	m_iLinkProfile = new TowerInputLinkProfile(world->GetIOManager()->GetWorkingMemory(), m_name);
	assert(m_iLinkProfile);
}

Tower::~Tower()
{
	for(diskItr_t diskItr = m_disks.begin(); diskItr != m_disks.end(); ++diskItr)
	{
		(*diskItr)->Detach();
		delete (*diskItr);
	}
	m_disks.clear();

	delete m_iLinkProfile;
	pWorld = 0;
}

//will always add a smaller disk than the top, so new disk must at end of container
//disks that have just been created already have their 'disk beneath' initialized, don't reset it
void Tower::AddDisk(Disk* newDisk, bool justCreated)
{
	assert(newDisk);
	if(!justCreated)
	{
		if(!m_disks.empty())
			newDisk->Update(m_disks.back(), this);
		else
			newDisk->Update(0, this);
	}

	m_disks.push_back(newDisk);
}

void Tower::RemoveTopDisk()
{
	if(m_disks.size() != 0)
	{
		vector<Disk*>::iterator eraseItr = m_disks.end();
		--eraseItr;
		m_disks.erase(eraseItr);
	}
}

Disk* Tower::GetTopDisk() const
{
	if(m_disks.size() != 0)
	{
		assert(m_disks.back());
		return m_disks.back();
	}
	return 0;
}


void Tower::PrintDiskAtRow(int row) const
{
	if(static_cast<int>(m_disks.size()) <= row)
		cout<<"--";
	else
		cout << m_disks[row]->GetSize();
}


//======================================================
//=========== Hanoi Function Definitions ===============

HanoiWorld::HanoiWorld(bool remoteConnection, bool graphicsOn, int inNumTowers, int inNumDisks) : drawGraphics(graphicsOn)
{
	//create Soar and agent
	// SML uses Kernel instead of Soar
	// Soar* soar = 0 ;
	Kernel* kernel = 0;

	if (!remoteConnection)
	{
		// Fastest method, but need to call "GetIncomingCommands" from time to time.
		// This is the "advanced" method for power SML users and we include it
		// in test code to make sure everything works correctly.
		kernel = Kernel::CreateKernelInCurrentThread("SoarKernelSML", true) ;

		// Slightly slower, but polls for incoming remote commands automatically
		// This is "normal" method to use for most SML users.
		// kernel = Kernel::CreateKernelInNewThread("SoarKernelSML") ;

		// SGIO equivalent
		// soar = new APISoar();
	}
	else
	{
		kernel = Kernel::CreateRemoteConnection(true, NULL) ;
		// SGIO equivalent
		// soar = new SIOSoar("127.0.0.1", 6969, true);
	}

	assert(kernel);

	// SML uses a more explicit error model, so we can get details about what failed.
	if (kernel->HadError())
	{
		// This string will contain a description of the error
		std::string error = kernel->GetLastErrorDescription() ;
		assert (!kernel->HadError()) ;

		// Shutdown if the kernel failed to initialize correctly.
		return ;
	}
	
	// SML allows us to trace remote connections in more detail if we wish
	//kernel->SetTraceCommunications(true) ;

	ioManager = new IOManager(kernel);
	assert(ioManager);

	//wrap up the Soar agent
	m_agent = new SoarAgent(ioManager->GetAgent(), ioManager->GetWorkingMemory(), this);
	assert(m_agent);


	//create Towers

	//Name each tower and store for later
	for(int towerNum = 0; towerNum < inNumTowers; ++towerNum)
	{
		//==============
		//"Left" tower
		//==============
		if(towerNum == 0)
		{
			Tower* tower = new Tower(this, 'A');
			assert(tower);

			//Create disks
			for(int currentDiskSize = maxNumDisks; currentDiskSize > 0; --currentDiskSize)
			{
				//The disk currently at the front of the container is the "bottom" disk.  New, smaller, disks
				//are inserted in back

				Disk* towerTopDisk = 0;

				if(tower->GetSize() != 0)
					towerTopDisk = tower->GetTopDisk();

				Disk* disk = new Disk(tower, currentDiskSize, towerTopDisk);
				assert(disk);

				tower->AddDisk(disk, true);
			}

			m_towers.push_back(tower);
		}
		//==============
		//Middle tower
		//==============
		else if(towerNum == 1)
		{
			Tower* tower = new Tower(this, 'B');
			assert(tower);
			m_towers.push_back(tower);
		}

		//===============
		//"Right" tower
		//===============
		else
		{
			Tower* tower = new Tower(this, 'C');
			assert(tower);
			m_towers.push_back(tower);
		}
	}//end-for-loop

}

HanoiWorld::~HanoiWorld()
{
	for(towerItr_t towerItr = m_towers.begin(); towerItr != m_towers.end(); ++towerItr)
		delete *towerItr;

	m_towers.clear();

	delete m_agent;
	m_agent = 0;
	delete ioManager; //This will clean up Soar and working memory
}

void HanoiWorld::Run()
{
	m_agent->MakeMove();
}

//remove from the source tower, add to the destination tower
bool HanoiWorld::MoveDisk(int sourceTower, int destinationTower)
{
	Disk* movingDisk = m_towers[sourceTower]->GetTopDisk();

	assert(movingDisk);

	if(!movingDisk)
		return false;

	m_towers[sourceTower]->RemoveTopDisk();
	m_towers[destinationTower]->AddDisk(movingDisk, false);

	return true;
}

void HanoiWorld::Print()
{
	for(int row = maxNumDisks - 1; row >= 0; --row)
	{
		for(unsigned int towerCounter = 0; towerCounter < m_towers.size(); ++towerCounter)
		{
			cout << "(";
			m_towers[towerCounter]->PrintDiskAtRow(row);
			cout << ")    ";
		}
		cout << endl;
	}
	cout<<"======================" << endl << endl;
}

bool HanoiWorld::AtGoalState()
{
	if(m_towers[2]->GetSize() == maxNumDisks)
		return true;
	return false;
}

void  HanoiWorld::EndGameAction()
{
	//Run();
//#ifdef GSKI_DAG_TOWERS_USE_TIMER
//	cout.precision(6);
//	cout <<  "Total run time: " <<  gSKITowersDAGTimer / static_cast<double>(CLK_TCK) << endl;
//#endif
}

