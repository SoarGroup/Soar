#include "gSKITowers.h"
#include "gSKITowersSoarAgent.h"

//standard directives
#include <iostream>
#include <string.h>
#include <string>
#include <functional>
#include <cassert>

using std::string;
using std::cout;
using std::endl;

//gSKI directives
#include "IgSKI_Wme.h"
#include "IgSKI_WMObject.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_InputLink.h"
#include "IgSKI_OutputLink.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_WorkingMemory.h"
#include "IgSKI_KernelFactory.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_Agent.h"
#include "gSKI_Stub.h"

using namespace	gSKI;

//commandline directives
#include "cli_CommandLineInterface.h"	
using namespace cli;

TgD::TgD* debugger;

//namespace gski_towers
//{

/*************************************************************
* @brief	The TowerInputLinkProfile class contains all of the  
*			wmes corresponding to theTower
*************************************************************/
class TowerInputLinkProfile
{
public:

	/*************************************************************
	* @returns	The Iwme* of the identifier that all tower child
	*			Iwmes hang off of
	*************************************************************/
	IWme* GetTowerIdentifier() const { return m_pPegIdentifier;}

	/*************************************************************
	* @returns	The IWMObject* of the identifier that all tower 
	*			child Iwmes use as a parent
	*************************************************************/
	IWMObject* GetTowerIdentifierObject() const {return m_pPegIdentifier->GetValue()->GetObject();}

	~TowerInputLinkProfile()
	{
		if(m_pPegIdentifier)
		{
			m_pPegIdentifier->Release();
			m_pPegIdentifier = 0;
		}
		
		if(m_pPegName)
		{
			m_pPegName->Release();
			m_pPegName = 0;
		}
	}
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



/*************************************************************
* @brief	The DiskInputLinkProfile class contains all of the
*			Iwmes for the "disk" and "holds" structures on the
*			input link, as well as pointers to the tower/peg
*			that the disk sits on, and a pointer to any disk
*			directly beneath this profile's corresponding disk
*
*************************************************************/
class DiskInputLinkProfile : public IInputProducer
{
public:
	/*************************************************************
	* @returns	The IWMObject* that a disk's child wmes use as a 
	*			parent
	*************************************************************/
	IWMObject* GetDiskIdentifierObject() const {return diskIdentifier->GetValue()->GetObject();}
	/*************************************************************
	* @returns	The IWMObject* that a "holds" structure's child  
	*			wmes use as a parent
	*************************************************************/
	IWMObject* GetHoldsIdentifierObject() const {return holdsIdentifier->GetValue()->GetObject();}

	/*************************************************************
	* @returns	The IWme* for this disk's identifier
	*************************************************************/
	IWme* GetDiskIdentifier() const {return diskIdentifier;}

	/*************************************************************
	* @brief	Updates the pointers to the peg and disk beneath
	*			Sets flag so that Update knows an update needs
	*			to be done
	* @param	inDisk	The new disk beneath, or zero, if none
	* @param	intower	The new Tower that this disk has moved to
	*************************************************************/
	void SetDiskBeneath(Disk* inDisk, Tower* inTower)
	{
		if(inDisk)
			diskBeneath = inDisk->GetDiskInputLinkProfile()->GetDiskIdentifier();
		else
			diskBeneath = 0;

		pegId = inTower->GetInputLinkProfile()->GetTowerIdentifierObject();
		holdsNeedsToBeUpdated = true;
	}

	/*************************************************************
	* @brief	Updates the wmes for "disk" and "holds" if the disk
	*			had been moved or created.  This is a callback 
	*			function that will automatically be called when 
	*			the Soar agent has initiated a run
	*			
	* @param	pWMemory	pointer to the agent's working memory
	*						Not client-owned, don't release.
	* @param	object		input link parent object that the Disk
	*						registered to update. Also not client-
	*						owned, don't release
	*************************************************************/
	void Update(IWorkingMemory* pWMemory, IWMObject* object)
	{
cout << "Update called on disk " << this->size->GetValue()->GetInt() << endl;
		if(holdsNeedsToBeUpdated == false)
			return;

		// Get List of objects referencing this object with attribute "on"
		tIWmeIterator* onItr = object->GetWMEs(k_holdsOnString.c_str());
		if(onItr->IsValid())
		{
			pWMemory->RemoveWme(peg);
			//peg->Release();
			peg = pWMemory->AddWmeObjectLink(object, k_holdsOnString.c_str(), pegId);

			onItr->Release();
		}
		else
			assert(false);
/*		// Get List of objects referencing this object with attribute "above"
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
				holdsDiskBeneath = pWMemory->ReplaceStringWme(oldDiskBeneath, "none");

			aboveItr->Release();
		}
		else
			assert(false);
		holdsNeedsToBeUpdated = false;
*/	}

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
		{
			name->Release();
			name = 0;
		}
		if(size)
		{
			size->Release();
			size = 0;
		}

		//Don't own these so just set pointers to zero
		//diskBeneath = 0;
		if(diskBeneath)
		{
//			diskBeneath->Release();
			diskBeneath = 0;
		}
		pegId = 0;

		//Release children of "holds"
		if(holdsDiskBeneath)
		{
			holdsDiskBeneath->Release();
			holdsDiskBeneath = 0;
		}
		if(peg)
		{
cout << "\tI had to release peg, which was at: " << peg << endl;
			peg->Release();
			peg = 0;
		}
		if(diskWme)
		{
			diskWme->Release();
			diskWme = 0;
		}

		//Release identifiers
		if(holdsIdentifier)
		{
			holdsIdentifier->Release();
			holdsIdentifier = 0;
		}

		if(diskIdentifier)
		{
			diskIdentifier->Release();
			diskIdentifier = 0;
		}
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


class IOManager
{
public:
	IOManager(IInputLink* inILink) : m_pILink(inILink)
	{
		m_pWorkingMem = m_pILink->GetInputLinkMemory();
	}

	//if parent is null, parent should be the input link root object
	IWme* AddWMObject(IWMObject* parent, const string& name)
	{
		IWMObject* parentToAttachTo = 0;
		bool touchedInputLinkRootObject = false;

		if(parent)
			parentToAttachTo = parent;
		else
		{
			m_pILink->GetRootObject(&parentToAttachTo);
			touchedInputLinkRootObject = true;
		}

		IWme* newWme = m_pWorkingMem->AddWmeNewObject(parentToAttachTo, name.c_str());

		if(touchedInputLinkRootObject)
			parentToAttachTo->Release();

		return newWme;
	}

	IWme* AddIntWme(IWMObject* parent, const string& name, int value)
	{
		IWMObject* parentToAttachTo;
		bool touchedInputLinkRootObject = false;

		if(parent)
			parentToAttachTo = parent;
		else
		{
			m_pILink->GetRootObject(&parentToAttachTo);
			touchedInputLinkRootObject = true;
		}

		IWme* newWme = m_pWorkingMem->AddWmeInt(parentToAttachTo, name.c_str(), value);

		if(touchedInputLinkRootObject)
			parentToAttachTo->Release();

		return newWme;
	}

	IWme* AddStringWme(IWMObject* parent, const string& name, const string& value)
	{
		IWMObject* parentToAttachTo;
		bool touchedInputLinkRootObject = false;

		if(parent)
			parentToAttachTo = parent;
		else
		{
			m_pILink->GetRootObject(&parentToAttachTo);
			touchedInputLinkRootObject = true;
		}

		IWme* newWme = m_pWorkingMem->AddWmeString(parentToAttachTo, name.c_str(), value.c_str());

		if(touchedInputLinkRootObject)
			parentToAttachTo->Release();
		return newWme;
	}

	IWme* AddIDWme(IWMObject* parent, const string& name, IWMObject* linkDestination)
	{
		IWMObject* parentToAttachTo;
		bool touchedInputLinkRootObject = false;

		if(parent)
			parentToAttachTo = parent;
		else
		{
			IWMObject* pILinkRootObject;
			m_pILink->GetRootObject(&pILinkRootObject);
			parentToAttachTo = pILinkRootObject;
			touchedInputLinkRootObject = true;
		}
		IWme* newWme = m_pWorkingMem->AddWmeObjectLink(parentToAttachTo, name.c_str(), linkDestination);

		if(touchedInputLinkRootObject)
			parentToAttachTo->Release();
		return newWme;
	}

	void RemoveWme(IWme* victimWme)
	{	//These are equivalent
		//victimWme->Release();
		m_pWorkingMem->RemoveWme(victimWme);
	}

private:
	IInputLink* m_pILink;
	IWorkingMemory* m_pWorkingMem;

	friend class HanoiWorld;
};





//using namespace gski_towers;

//======================================================
//============ Disk Function Definitions ===============

Disk::Disk(Tower* tower, int inSize, Disk* diskBeneath) : 
		pTower(tower), m_size(inSize)
{
	m_iLinkProfile = new DiskInputLinkProfile();
	assert(m_iLinkProfile);
	m_iLinkProfile->pegId = tower->GetInputLinkProfile()->GetTowerIdentifierObject();
	if(diskBeneath)
		m_iLinkProfile->diskBeneath = diskBeneath->m_iLinkProfile->diskIdentifier;
	else
		m_iLinkProfile->diskBeneath = 0;

	//============================
	// Initialize "disk" wmes
	//============================


	//Add the disk identifier to the input link
	IOManager* manager = pTower->GetWorld()->GetIOManager();
	assert(manager);
	m_iLinkProfile->diskIdentifier = manager->AddWMObject(0, k_diskIdentifierString);

	IWMObject* parentObject = m_iLinkProfile->diskIdentifier->GetValue()->GetObject();
	//attach subordinate wmes to disk identifier;
	m_iLinkProfile->name = manager->AddIntWme(parentObject, k_nameString, m_size);
	m_iLinkProfile->size = manager->AddIntWme(parentObject, k_diskSizeString, m_size);

	//============================
	// Initialize "holds" wmes
	//============================
	//add the holds identifier
	m_iLinkProfile->holdsIdentifier	= manager->AddWMObject(0, k_holdsIdentifierString);
	IWMObject* holdsParentObject =  m_iLinkProfile->holdsIdentifier->GetValue()->GetObject();

	//add holds wmes to parent object

	//the "on" wme points to the peg that this disk is on
	m_iLinkProfile->peg = manager->AddIDWme(holdsParentObject, k_holdsOnString, m_iLinkProfile->pegId);

	//the "disk" wme points back to its corresponding disk
	m_iLinkProfile->diskWme = manager->AddIDWme(holdsParentObject, k_diskIdentifierString, parentObject);

	//the "above" wme points to the disk beneath this one, else "none"
	if(diskBeneath)
	{
		m_iLinkProfile->diskBeneath = diskBeneath->GetDiskInputLinkProfile()->GetDiskIdentifier();
		IWMObject* pDiskBeneathIdentifier = diskBeneath->GetDiskInputLinkProfile()->GetDiskIdentifierObject();
		m_iLinkProfile->holdsDiskBeneath = manager->AddIDWme(holdsParentObject, k_holdsAboveString, pDiskBeneathIdentifier);
	}
	else
		m_iLinkProfile->holdsDiskBeneath = manager->AddStringWme(holdsParentObject, k_holdsAboveString, k_noneString);
}



void Disk::Detach()
{
	delete m_iLinkProfile;
	pTower = 0;
cout << "Disk " << m_size << " was detached" << endl;
}


void Disk::Update(Disk* diskBeneath, Tower* tower)
{
	pTower = tower;
	m_iLinkProfile->SetDiskBeneath(diskBeneath, tower);
}

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
	for(diskItr_t diskItr = m_disks.begin(); diskItr != m_disks.end(); ++diskItr)
	{
		(*diskItr)->Detach();
		delete (*diskItr);
	}
	m_disks.clear();

	delete m_iLinkProfile;
	pWorld = 0;
}


//will always add a smaller disk than the top, so new disk must on at end of container
//disks that have just been created already have their disk beneath initialized, don't reset it
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



//======================================================
//============ Hanoi Function Definitions ==============


HanoiWorld::HanoiWorld(bool graphicsOn, int inNumTowers,  int inNumDisks) : drawGraphics(graphicsOn)
{
	// create kernel
	IKernelFactory* kFactory = gSKI_CreateKernelFactory();
	IKernel* kernel = kFactory->Create();
	IAgentManager* manager = kernel->GetAgentManager();
	gSKI::IAgent* agent = manager->AddAgent("towersAgent");
	assert(agent);

//create debugger
//TgD::TSI_VERSION tsiVersion = TgD::TSI40;
//debugger = CreateTgD(agent, kernel, kFactory->GetKernelVersion(), tsiVersion, "Towers.exe");
//debugger->Init();

	//Source the agent's productions
	CommandLineInterface* commLine = new CommandLineInterface();
	commLine->SetKernel(kernel);
	char* pResponse = "\0";//we don't need to allocate space for this
	gSKI::Error* pError = new Error();
	assert(commLine->DoCommand(agent, "pushd ../examples/towers", pResponse, pError));
	assert(commLine->DoCommand(agent, "source towers-of-hanoi-86.soar", pResponse, pError));
	assert(commLine->DoCommand(agent, "popd", pResponse, pError));

	IInputLink* pILink = agent->GetInputLink();
	ioManager = new IOManager(pILink);
	assert(ioManager);

	//wrap up the Soar agent
	m_agent = new SoarAgent(agent, this);
	//register the SoarAgent as the output processor
	IOutputLink* oLink = agent->GetOutputLink();
	oLink->AddOutputProcessor("move-disk", m_agent);

	//create Towers
	IWorkingMemory* pWMemory = pILink->GetInputLinkMemory();
	IWMObject* pILinkRootObject;
	pILink->GetRootObject(&pILinkRootObject);

/*	Tower tower(this, 'A');
	
	Disk disk(&tower, 2, 0);

	disk.Detach();*/


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


	delete commLine;
	delete pError;
	kFactory->Release();//   <--program actually doesn't leak (gski objs)if you don't do this
	kFactory = 0;
	pILinkRootObject->Release();
	pILinkRootObject = 0;
}


HanoiWorld::~HanoiWorld()
{
	for(towerItr_t towerItr = m_towers.begin(); towerItr != m_towers.end(); ++towerItr)
		delete *towerItr;
	m_towers.clear();

	delete ioManager;
	delete m_agent;
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

void  HanoiWorld::EndGameAction()
{
	Run();
}

//}//closes namespace

