#include "AgnosticTowers.h"
#include "gSKITowersSoarAgent.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <functional>
#include <cassert>

using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::less;

#include "IgSKI_Wme.h"
#include "IgSKI_WMObject.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_InputLink.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_WorkingMemory.h"
#include "IgSKI_KernelFactory.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_Agent.h"
#include "gSKI_Stub.h"

using namespace	gSKI;

//namespace gski_towers
//{
	class DiskInputLinkProfile : public IInputProducer
	{
	public:
		IWMObject* GetDiskIdentifierObject() const {return diskIdentifier->GetValue()->GetObject();}
		IWMObject* GetHoldsIdentifierObject() const {return holdsIdentifier->GetValue()->GetObject();}
		IWme* GetDiskIdentifier() const {return diskIdentifier;}

		void SetDiskBeneath(Disk* inDisk)
		{
			if(inDisk)
				diskBeneath = inDisk->GetDiskInputLinkProfile()->GetDiskIdentifier();
			else
				diskBeneath = 0;

			pegId = inDisk->GetDiskInputLinkProfile()->pegId;
			holdsNeedsToBeUpdated = true;
		}

		//Replace the "on" and "above" wmes for the corresponding 
		//holds wme
		void Update(IWorkingMemory* pWMemory, IWMObject* object)
		{
			if(holdsNeedsToBeUpdated == false)
				return;

			// Get List of objects referencing this object with attribute "on"
			tIWmeIterator* onItr = object->GetWMEs("on");
			if(onItr->IsValid())
			{
				pWMemory->RemoveWme(peg);
				peg = pWMemory->AddWmeObjectLink(object, "on", pegId);
			}
			// Get List of objects referencing this object with attribute "above"
			tIWmeIterator* aboveItr = object->GetWMEs("above");

			if(aboveItr->IsValid())
			{	//Get the old "above" value
				IWme* oldDiskBeneath = aboveItr->GetVal();

				if(diskBeneath)
				{
					pWMemory->RemoveWme(holdsDiskBeneath);
					holdsDiskBeneath = pWMemory->AddWmeObjectLink(object, "above", diskBeneath->GetValue()->GetObject());
				}
				else
				{
					holdsDiskBeneath = pWMemory->ReplaceStringWme(oldDiskBeneath, "none");
				}

			}
			holdsNeedsToBeUpdated = false;
		}
	private:
		DiskInputLinkProfile()
		{
			diskIdentifier = 0;
			name = 0;
			size = 0;
			diskBeneath = 0;
			pegId = 0;
			holdsIdentifier = 0;
			holdsDiskBeneath = 0;
			peg = 0;
			diskWme = 0;
			holdsNeedsToBeUpdated = true;
		}

		~DiskInputLinkProfile()
		{
			//Release children of "disk"
			if(name)
				name->Release();
			if(size)
				size->Release();
			if(diskBeneath)
				diskBeneath->Release();
			if(pegId)
				pegId->Release();

			//Release children of "holds"
			if(holdsDiskBeneath)
				holdsDiskBeneath->Release();
			if(peg)
				peg->Release();
			if(diskWme)
				diskWme->Release();

			//Release identifiers
			if(holdsIdentifier)
				holdsIdentifier->Release();

			if(diskIdentifier)
				diskIdentifier->Release();
		}

		friend class Disk;

		//"disk" wmes
		IWme* diskIdentifier;
			IWme* name;
			IWme* size;

		IWme* diskBeneath;	//wme of actual disk beneath
		IWMObject* pegId;	//object of wme of actual ped

		//"holds" wmes
		IWme* holdsIdentifier;
			IWme* holdsDiskBeneath;	//disk wme that appears on the holds structure on the input link
			IWme* peg;				//peg wme that appears on the holds structure on input link
			IWme* diskWme;

		bool holdsNeedsToBeUpdated;
	};

	class TowerInputLinkProfile
	{
	public:

		IWme* GetTowerIdentifier() const { return m_pPegIdentifier;}
		IWMObject* GetTowerIdentifierObject() const {return m_pPegIdentifier->GetValue()->GetObject();}

	private:
		TowerInputLinkProfile()
		{
			m_pPegIdentifier = 0;
			m_pPegName = 0;
		}

		IWme* m_pPegIdentifier;
		IWme* m_pPegName;

		friend class Tower;
	};


	class IOManager
	{
	public:
		IOManager(IInputLink* inILink) : m_pILink(inILink)
		{
			m_pWorkingMem = m_pILink->GetInputLinkMemory();

		}

		//if parent is null, parent should be the inputlink root object
		IWme* AddWMObject(IWMObject* parent, const string& name)
		{
			IWMObject* pILinkRootObject;
			m_pILink->GetRootObject(&pILinkRootObject);
			return m_pWorkingMem->AddWmeNewObject(pILinkRootObject, "disk");
		}

		IWme* AddIntWme(IWMObject* parent, const string& name, int value)
		{

			return 0; //fixme
		}

		IWme* AddStringWme(IWMObject* parent, const string& name, const string& value)
		{
			return 0; //fixme

		}

		IWme* AddIDWme(IWMObject* parent, const string& name, IWMObject* linkDestination)
		{

			return 0; //fixme
		}

	private:
		IInputLink* m_pILink;
		IWorkingMemory* m_pWorkingMem;

		friend class HanoiWorld;
	};


//}//closes namespace


//using namespace gski_towers;

//======================================================
//============ Disk Function Definitions ===============

Disk::Disk(Tower* tower, int inSize, Disk* diskBeneath) : 
		pTower(tower), m_size(inSize), m_pDiskBeneath(diskBeneath)
{
	m_iLinkProfile = new DiskInputLinkProfile();
	//============================
	// Initialize "disk" wmes
	//============================


	//Add the disk identifier to the input link
	IOManager* manager = pTower->GetWorld()->GetIOManager();
	m_iLinkProfile->diskIdentifier = manager->AddWMObject(0, k_diskIdentifierString);

	const gSKI::ISymbol* parentSymbol = m_iLinkProfile->diskIdentifier->GetValue();
	IWMObject* parentObject = parentSymbol->GetObject();
	//attach subordinate wmes to disk identifier;
	m_iLinkProfile->name = manager->AddIntWme(parentObject, k_nameString, m_size);
	m_iLinkProfile->size = manager->AddIntWme(parentObject, k_diskSizeString, m_size);

	//============================
	// Initialize "holds" wmes
	//============================
	//add the holds identifier
	m_iLinkProfile->holdsIdentifier	= manager->AddWMObject(0, k_holdsIdentifierString);
	const gSKI::ISymbol* holdsParentSymbol = m_iLinkProfile->holdsIdentifier->GetValue();
	IWMObject* holdsParentObject = holdsParentSymbol->GetObject();

	//add holds wmes to parent object
	m_iLinkProfile->peg = manager->AddIDWme(holdsParentObject, k_holdsOnString, m_iLinkProfile->pegId);

	//the holds wme points back to its corresponding disk
	m_iLinkProfile->diskWme = manager->AddIDWme(holdsParentObject, k_diskIdentifierString, parentObject);

	if(m_pDiskBeneath)
	{
		IWMObject* pDiskBeneathIdentifier = m_pDiskBeneath->m_iLinkProfile->GetDiskIdentifierObject();
		m_iLinkProfile->holdsDiskBeneath = manager->AddIDWme(holdsParentObject, k_holdsAboveString, pDiskBeneathIdentifier);
	}
	else
		m_iLinkProfile->holdsDiskBeneath = manager->AddStringWme(holdsParentObject, k_holdsAboveString, k_noneString);
}


void Disk::Detach()
{
	//Release everything this is touching that doesn't belong to us
/*	if(m_pDiskBeneath)
		m_pDiskBeneath->Release();
	//m_pPegId->Release();
	m_pPegId = 0;//poor substitute for releasing

	//Remove wmes that do belong to this
	m_pWMemory->RemoveWme(m_pDiskIdentifier);//parent wme
	m_pWMemory->RemoveWme(m_pName);
	m_pWMemory->RemoveWme(m_pSize);

	//"holds" wmes
	m_pWMemory->RemoveWme(m_pHoldsIdentifier);//parent wme
	m_pWMemory->RemoveWme(m_pHoldsDiskBeneath);
	m_pWMemory->RemoveWme(m_pPeg);
	m_pWMemory->RemoveWme(m_pDiskWme);
	*/ //todo move this to input link profile destructor
	delete m_iLinkProfile;
}

//Replace the "on" and "above" wmes for the corresponding 
//holds wme
/*void Disk::Update(IWorkingMemory* pWMemory, IWMObject* object)
{
	if(m_holdsNeedsToBeUpdated == false)
		return;

	// Get List of objects referencing this object with attribute "on"
	tIWmeIterator* onItr = object->GetWMEs("on");
	if(onItr->IsValid())
	{
		pWMemory->RemoveWme(m_pPeg);
		m_pPeg = pWMemory->AddWmeObjectLink(object, "on", m_pPegId);
	}
	// Get List of objects referencing this object with attribute "above"
	tIWmeIterator* aboveItr = object->GetWMEs("above");

	if(aboveItr->IsValid())
	{	//Get the old "above" value
		IWme* oldDiskBeneath = aboveItr->GetVal();

		if(m_pDiskBeneath)
		{
			pWMemory->RemoveWme(m_pHoldsDiskBeneath);
			m_pHoldsDiskBeneath = pWMemory->AddWmeObjectLink(object, "above", m_pDiskBeneath->GetValue()->GetObject());
		}
		else
		{
			m_pHoldsDiskBeneath = pWMemory->ReplaceStringWme(oldDiskBeneath, "none");
		}

	}
	m_holdsNeedsToBeUpdated = false;
}*/


//IWme* const GetIdentifierWME() const {return m_pDiskIdentifier;}

//IWme* const GetHoldsIdentifierWME() const {return m_pHoldsIdentifier;}


/*void SetDiskBeneath(Disk* diskBeneath, IWMObject* pegObject)
{
	//TODO //FIXME @TODO release ref to wme of old disk beneath (if its not zero)  ????

	if(diskBeneath)
		m_pDiskBeneath = diskBeneath->GetIdentifierWME();
	else
		m_pDiskBeneath = 0;

	m_pPegId = pegObject;
	m_holdsNeedsToBeUpdated = true;
}*/



//======================================================
//============ Tower Function Definitions ==============


Tower::Tower(HanoiWorld* inWorld, char inName) : pWorld(inWorld), m_name(inName)
{
	m_iLinkProfile = new TowerInputLinkProfile();
	IOManager* manager = pWorld->GetIOManager();

	m_iLinkProfile->m_pPegIdentifier = manager->AddWMObject(0, k_worldPegString);
	string nameString;
	nameString = m_name;
	m_iLinkProfile->m_pPegName = manager->AddStringWme(m_iLinkProfile->GetTowerIdentifierObject(), k_nameString, nameString);
}

Tower::~Tower()
{
/*	IWorkingMemory* pWorkingMem = m_pILink->GetInputLinkMemory();
	pWorkingMem->RemoveWme(m_pPegIdentifier);
	for(vector<Disk*>::iterator diskItr = m_disks.begin(); diskItr != m_disks.end(); ++diskItr)
	{
		(*diskItr)->Detach();
	}
	m_disks.clear();
	//can't release ILink ptr*/
}


//will always add a smaller disk than the top, so new disk must on at end of container
//disks that have just been created already have their disk beneath initialized, don't reset it
void Tower::AddDisk(Disk* newDisk, bool justCreated)
{
	assert(newDisk);
	if(!justCreated)
	{
		if(!m_disks.empty())
			newDisk->GetDiskInputLinkProfile()->SetDiskBeneath(m_disks.back());
		else
			newDisk->GetDiskInputLinkProfile()->SetDiskBeneath(0);
	}

	m_disks.push_back(newDisk);
}

void Tower::RemoveTopDisk()
{
	if(m_disks.size() != 0)
		m_disks.erase(--m_disks.end());
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

int Tower::GetSize() const
{
	return static_cast<int>(m_disks.size());
}


void Tower::PrintDiskAtRow(int row) const
{
	if(static_cast<int>(m_disks.size()) <= row)
		cout<<"--";
	else
	{
		cout << m_disks[row]->GetSize();
	}
}




/*void Tower::PrintEntireTower()
{
	for(vector<Disk*>::iterator fooItr = m_disks.begin(); fooItr != m_disks.end(); ++fooItr)
		cout << (*fooItr)->GetSize() << endl;
	cout << endl;
}*/


/* fixme , remove , todo
private:
	vector<Disk*> m_disks;
	char m_name;
	int m_number;

	IInputLink* m_pILink;
*/




HanoiWorld::HanoiWorld(bool graphicsOn, int inNumTowers,  int inNumDisks) : drawGraphics(graphicsOn)
{

	IKernelFactory* kFactory = gSKI_CreateKernelFactory();

	// create kernel
	IKernel* kernel = kFactory->Create();
	IAgentManager* manager = kernel->GetAgentManager();
	gSKI::IAgent* agent = manager->AddAgent("towersAgent");

	IInputLink* pILink = agent->GetInputLink();
	ioManager = new IOManager(pILink);
	m_agent = new SoarAgent(agent, this);

	//create Towers
	IWorkingMemory* pWMemory = pILink->GetInputLinkMemory();
	IWMObject* pILinkRootObject;
	pILink->GetRootObject(&pILinkRootObject);

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
			IWme* diskBeneathWME = 0;
			//Create disks
			for(int currentDiskSize = maxNumDisks; currentDiskSize > 0; --currentDiskSize)
			{
				//The disk currently at the front of the container is the "bottom" disk.  New, smaller, disks 
				//are inserted in back
				IWMObject* towerIdObject = tower->GetInputLinkProfile()->GetTowerIdentifierObject();
				assert(towerIdObject);
				IWme* towerTopDiskWme = 0;
				Disk* towerTopDisk = 0;

				if(tower->GetSize() != 0)
				{
					towerTopDisk = tower->GetTopDisk();
					towerTopDiskWme = towerTopDisk->m_iLinkProfile->GetDiskIdentifier();
				}

				Disk* disk = new Disk(tower, currentDiskSize, towerTopDisk);

				pILink->AddInputProducer(disk->m_iLinkProfile->GetHoldsIdentifierObject(), disk->m_iLinkProfile);
				assert(disk);
				tower->AddDisk(disk, true);
			}

			m_towers.push_back(tower);
			//tower->PrintEntireTower();
		}
		//==============
		//Middle tower
		//==============
		else if(towerNum == 1)
		{
			Tower* tower = new Tower(this, 'B');
			m_towers.push_back(tower);
		}

		//===============
		//"Right" tower
		//===============
		else
		{
			Tower* tower = new Tower(this, 'C');
			m_towers.push_back(tower);
		}
	}//for
//Print();

}


HanoiWorld::~HanoiWorld()
{
	for(towerItr_t towerItr = m_towers.begin(); towerItr != m_towers.end(); ++towerItr)
		(*towerItr)->~Tower();
	m_towers.clear();
}

void HanoiWorld::Run()
{
	m_agent->MakeMove();
}

	//remove from the source tower, add to the destination tower
bool HanoiWorld::MoveDisk(int sourceTower, int destinationTower)
{
	Disk* movingDisk = m_towers[sourceTower]->GetTopDisk();
	if(!movingDisk)
		return false;

	assert(movingDisk);
	m_towers[sourceTower]->RemoveTopDisk();

	m_towers[destinationTower]->AddDisk(movingDisk, false);
	return true;
}


void HanoiWorld::Print()
{

	for(int row = maxNumDisks - 1; row >= 0; --row)
	{
		for(int towerCounter = 0; towerCounter < static_cast<int>(m_towers.size()); ++towerCounter)
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



