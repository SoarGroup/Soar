#include "Towers.h"

//standard directives
#include <cassert>
#include <iostream>
#include <string>

using std::cout;
using std::cin;
using std::endl;
using std::string;

//SGIO directives
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

//
#include "sgioTowersSoarAgent.h"
//using sgio_towers::SoarAgent;

//namespace sgio_towers
//{

class DiskInputLinkProfile
{
public:

	~DiskInputLinkProfile()
	{
		pWMemory->DestroyWME(m_pHoldsIdentifier);
		m_pHoldsIdentifier = 0;
		m_pDisk = 0;
		m_pDiskBeneath = 0;
	}

	void SetDiskBeneath(Disk* diskBeneath)
	{

//		m_pSize->GetValue()

	}

	
private:

	friend class Disk;

	DiskInputLinkProfile(WorkingMemory* inWMemory) : pWMemory(inWMemory)
	{

	}

	WorkingMemory* pWMemory;
		
	SoarId* m_pHoldsIdentifier;
		StringElement* m_pPeg; //name of peg that the disk is on
		IntElement* m_pDisk;	// size and name of disk
		IntElement* m_pDiskBeneath;

	int m_size;
};

/*************************************************************
* @brief	The IOManager class contains an SGIO
*			WorkingMemory pointer, and has wrapper functions
*			for manipulating the input link
*************************************************************/
class IOManager
{
public:
	Agent* GetAgent() const
	{
		return pAgent;
	}

	WorkingMemory* GetWorkingMemory() const
	{
		return pWMemory;
	}
private:
	IOManager(Soar* inSoar) : pSoar(inSoar)
	{
		pAgent = pSoar->CreateAgent("sgioTowersAgent");
		assert(pAgent);

		//Load agent's productions
		if(! pAgent->LoadProductions("towers-of-hanoi-SGIO.soar"))
		{
			cout << "Program ending because productions failed to load. Press any key followed by 'enter' " << endl;
			string foo;
			cin >> foo;
			exit(1);
		}

		pWMemory = new WorkingMemory(pAgent);
		assert(pWMemory);
	}

	Soar* pSoar;
	Agent* pAgent;
	WorkingMemory* pWMemory;
	friend class HanoiWorld;
};

//}//closes namespace


//using sgio_towers::DiskInputLinkProfile;

//======================================================
//============ Disk Function Definitions ===============

Disk::Disk(Tower* inTower, int inSize, Disk* inDiskBeneath) : pTower(inTower), m_size(inSize)
{
	m_iLinkProfile = new DiskInputLinkProfile(pTower->GetWorld()->GetIOManager()->GetWorkingMemory());
	assert(m_iLinkProfile);
}

void Disk::Detach()
{
	delete m_iLinkProfile;
}

	
void Disk::Update(Disk* diskBeneathMe, Tower* tower)
{
}

//======================================================
//============ Tower Function Definitions ==============

Tower::Tower(HanoiWorld* world, char name)
{

}

Tower::~Tower()
{

}

//will always add a smaller disk than the top, so new disk must on at end of container
//disks that have just been created already have their disk beneath initialized, don't reset it
void Tower::AddDisk(Disk* newDisk, bool justCreated)
{
 // fixme todo fill this in

}

void Tower::RemoveTopDisk()
{
	if(m_disks.size() != 0)
		m_disks.erase(--m_disks.end());
}

Disk* Tower::GetTopDisk() const
{
	return 0 ;//fixme
}



void Tower::PrintDiskAtRow(int row) const
{

 // fixme todo fill this in
}


//======================================================
//=========== Hanoi Function Definitions ===============

HanoiWorld::HanoiWorld(bool graphicsOn, int inNumTowers,  int inNumDisks) : drawGraphics(graphicsOn)
{
	//create Soar and agent
	Soar* soar = 0;	

	#ifdef SGIO_API_MODE
		soar = new APISoar();
	#elif defined SGIO_TSI_MODE
		soar = new SIOSoar("127.0.0.1", 6969, true);
	#endif
	assert(soar);


	ioManager = new IOManager(soar);
	assert(ioManager);

	//wrap up the Soar agent
	m_agent = ioManager->GetAgent();

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
	delete ioManager;
	delete m_agent
 // fixme todo fill this in
}

void HanoiWorld::Run()
{
	m_agent->MakeMove();
}

//remove from the source tower, add to the destination tower
bool HanoiWorld::MoveDisk(int sourceTower, int destinationTower)
{
	return false;//fixme, placeholder return
}

void HanoiWorld::Print()
{
// fixme todo fill this in
} 

bool HanoiWorld::AtGoalState()
{
	return false;//fixme, placeholder return
}


