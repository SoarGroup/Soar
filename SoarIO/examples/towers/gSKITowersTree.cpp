#include "gSKITowers.h"
#include "gSKITowersSoarAgent.h"

//standard directives
#include <iostream>
#include <string.h>
#include <string>
#include <functional>
#include <cassert>
#include <time.h>

using std::string;
using std::cout;
using std::cin;
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
clock_t	gSKITowersTreeTimer;

//namespace gski_towers
//{

/*************************************************************
* @brief	The TowerInputLinkProfile class contains all of the  
*			wmes corresponding to the Tower
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

	/*************************************************************
	* @brief	Release reference to wmes actually owned by this   
	*			class, set pointers to zero
	*************************************************************/
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
	/*************************************************************
	* @brief	Initialize private data to zero.  This constructor
	*			can only be called by friends
	*************************************************************/
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
	*			to be done.
	* @param	inDisk	The new disk beneath, or zero, if none
	* @param	intower	The new Tower that this disk has moved to
	*************************************************************/
	void SetDiskBeneath(Disk* inDisk, Tower* inTower)
	{
		if(inDisk)
			diskBeneath = inDisk->GetDiskInputLinkProfile()->GetDiskIdentifier();
		else
			diskBeneath = 0;

		//fixfix: should probably update peg here (not pegID)
		//pegId = inTower->GetInputLinkProfile()->GetTowerIdentifierObject();
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
		if(holdsNeedsToBeUpdated == false)
			return;

		// Get List of objects referencing this object with attribute "on"
		tIWmeIterator* onItr = object->GetWMEs(k_holdsOnString.c_str());
		if(onItr->IsValid())
		{
			//peg->Release(); //BADBAD - this plugs the leak, but we should not be using this peg ptr
							//again having just called Release on it (according to the gSKI doc) //fixme
			//pWMemory->RemoveWme(peg);
			//fixfix: this should update the ^on attrib with the peg name string, not id
			//peg = pWMemory->AddWmeObjectLink(object, k_holdsOnString.c_str(), pegId);
			string newTowerName = peg->GetValue()->GetString();
			peg = pWMemory->ReplaceStringWme(peg, newTowerName.c_str());
		}
		//fixfix: replace this assert when above fix is done
		//else
		//	assert(false);
		onItr->Release();

		// Get List of objects referencing this object with attribute "above"
		tIWmeIterator* aboveItr = object->GetWMEs("above");

		if(aboveItr->IsValid())
		{	//Get the old "above" value

			if(diskBeneath)
			{
				holdsDiskBeneath->Release();//BADBAD - this plugs the leak, but we should not be using this peg ptr
				//again having just called Release on it (according to the gSKI doc) //fixme
				pWMemory->RemoveWme(holdsDiskBeneath);
				//fixfix: this should update the ^above attrib with an int wme, not id
				//holdsDiskBeneath = pWMemory->AddWmeObjectLink(object, "above", diskBeneath->GetValue()->GetObject());
			}
			else
			{
				IWme* oldDiskBeneath = aboveItr->GetVal();
				holdsDiskBeneath->Release();//SO SHADY!  BADBAD fixme.  plugs leak, though
				holdsDiskBeneath = pWMemory->ReplaceStringWme(oldDiskBeneath, "none");
				oldDiskBeneath->Release();
			}
		}
		//fixfix: replace this assert when above fix is done
		//else
		//	assert(false);
		aboveItr->Release();

		holdsNeedsToBeUpdated = false;
	}

private:

	/*************************************************************
	* @brief	Initializes all of the IWme and WMObject pointers 
	*			to zero.  This constructor can only be called by
	*			friends
	*************************************************************/
	DiskInputLinkProfile()
	{
		diskIdentifier = 0;

		diskBeneath = 0;
		//pegId = 0;
		holdsIdentifier = 0;
		holdsDiskBeneath = 0;
		peg = 0;
		diskWme = 0;
		holdsNeedsToBeUpdated = true;
	}

	/*************************************************************
	* @brief	Destructor releases all owned wmes, sets everything 
	*			to zero.  
	*************************************************************/
	~DiskInputLinkProfile()
	{
		//=============================================
		//This object doesn't own these, so just set pointers to zero
		//These copies of wmes/wmobjs did not increase the reference count, so we shouldn't
		//decrease the count by calling Release()
		if(diskBeneath)
		{
			//diskBeneath->Release();  //don't do it!
			diskBeneath = 0;
		}
		/*if(pegId)
		{
			//pegId->Release();		  //don't do it!
			pegId = 0;
		}*///============================================

		//Release children of "holds"
		if(holdsDiskBeneath)
		{
			holdsDiskBeneath->Release();
			holdsDiskBeneath = 0;
		}
		if(peg)
		{
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
//		IWme* name;
//		IWme* size;

	IWme* diskBeneath;	//wme of actual disk beneath
	//IWMObject* pegId;	//object of wme of actual peg

	//"holds" wmes
	IWme* holdsIdentifier;
		IWme* holdsDiskBeneath;	//disk wme that appears on the holds structure on the input link
		IWme* peg;				//peg wme that appears on the holds structure on input link
		IWme* diskWme;

	bool holdsNeedsToBeUpdated;
};

/*************************************************************
* @brief	The IOManager class contains gSKI-specific
*			WorkingMemory and InputLink specific pointers
*			Has wrapper functions for manipulating the input
*			link
*************************************************************/
class IOManager
{
public:

	/*************************************************************
	* @brief	Adds an object to an agent's working memory  
	* @param	parent	The WMObject to add the new ojbect to
	*					If parent is zero, the ILink root obj is used
	* @param	name	the name of the new WMObject
	*************************************************************/
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

	/*************************************************************
	* @brief	Adds an wme to an agent's working memory  
	* @param	parent	The WMObject to add the new wme to
	*					If parent is zero, the ILink root obj is used
	* @param	name	the name of the new WMObject
	* @param	value	value of wme triplet
	*************************************************************/
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

	/*************************************************************
	* @brief	Adds an wme to an agent's working memory  
	* @param	parent	The WMObject to add the new wme to
	*					If parent is zero, the ILink root obj is used
	* @param	name	the name of the new WMObject
	* @param	value	value of wme triplet
	*************************************************************/
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

	/*************************************************************
	* @brief	Adds an wme to an agent's working memory  
	* @param	parent	The WMObject to add the new wme to
	*					If parent is zero, the ILink root obj is used
	* @param	name	the name of the new WMObject
	* @param	value	value of wme triplet
	*************************************************************/
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

	/*************************************************************
	* @brief	Removes a wme from working memory  
	* @param	victimWme	pointer to wme to be removed
	*					If parent is zero, the ILink root obj is used
	*************************************************************/
	void RemoveWme(IWme* victimWme)
	{
		m_pWorkingMem->RemoveWme(victimWme);
	}

private:
	/*************************************************************
	* @brief	Private constructor initializes the input link and
	*			working memory pointers. can only be called by
	*			friends
	*************************************************************/
	IOManager(IInputLink* inILink) : m_pILink(inILink)
	{
		m_pWorkingMem = m_pILink->GetInputLinkMemory();
	}


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
	//m_iLinkProfile->pegId = tower->GetInputLinkProfile()->GetTowerIdentifierObject();
	if(diskBeneath)
	{
		m_iLinkProfile->diskBeneath = diskBeneath->m_iLinkProfile->diskIdentifier;
		//fixfix: Do we even need the diskBeneath IWme* anymore?
cout<<"This disk of size "<<inSize<<"has a diskBeneath"<<endl;
cout<<"It's size is: "<<diskBeneath->GetSize()<<endl;
	}
	else
	{
		m_iLinkProfile->diskBeneath = 0;
cout<<"This disk of size "<<inSize<<"has NO diskBeneath"<<endl;
	}

	//============================
	// Initialize "disk" wmes
	//============================


	//Add the disk identifier to the input link
	IOManager* manager = pTower->GetWorld()->GetIOManager();
	assert(manager);
	//****** Change this to initialize an int wme ******//
	//m_iLinkProfile->diskIdentifier = manager->AddWMObject(0, k_diskIdentifierString);
	m_iLinkProfile->diskIdentifier = manager->AddIntWme(0, k_diskIdentifierString, inSize);

	//IWMObject* parentObject = m_iLinkProfile->diskIdentifier->GetValue()->GetObject();
	//attach subordinate wmes to disk identifier;
//	m_iLinkProfile->name = manager->AddIntWme(parentObject, k_nameString, m_size);
//	m_iLinkProfile->size = manager->AddIntWme(parentObject, k_diskSizeString, m_size);

	//============================
	// Initialize "holds" wmes
	//============================
	//add the holds identifier
    m_iLinkProfile->holdsIdentifier	= manager->AddWMObject(0, k_holdsIdentifierString);
	IWMObject* holdsParentObject =  m_iLinkProfile->holdsIdentifier->GetValue()->GetObject();

	//add holds wmes to parent object

	//the "on" wme points to the peg that this disk is on
	//fixedfixed: this should be a string wme, peg name
	//m_iLinkProfile->peg = manager->AddIDWme(holdsParentObject, k_holdsOnString, m_iLinkProfile->pegId);
	string newTowerName;
	newTowerName = pTower->GetName();
	m_iLinkProfile->peg = manager->AddStringWme(holdsParentObject, k_holdsOnString, newTowerName);

	//the "disk" wme points back to its corresponding disk
	//fixedfixed: this should be an int wme
	//m_iLinkProfile->diskWme = manager->AddIDWme(holdsParentObject, k_diskIdentifierString, parentObject);
	m_iLinkProfile->diskWme = manager->AddIntWme(holdsParentObject, k_diskIdentifierString, inSize);

	//the "above" wme points to the disk beneath this one, else "none"
	if(diskBeneath)
	{
		m_iLinkProfile->diskBeneath = diskBeneath->GetDiskInputLinkProfile()->GetDiskIdentifier();
		//fixfix: this should add an int wme, not ID
		//bugbug: why doesn't this intwme show up??
		//IWMObject* pDiskBeneathIdentifier = diskBeneath->GetDiskInputLinkProfile()->GetDiskIdentifierObject();
		//m_iLinkProfile->holdsDiskBeneath = manager->AddIDWme(holdsParentObject, k_holdsAboveString, pDiskBeneathIdentifier);
		m_iLinkProfile->holdsDiskBeneath = manager->AddIntWme(holdsParentObject, k_holdsAboveString, diskBeneath->GetSize());
	}
	else
		m_iLinkProfile->holdsDiskBeneath = manager->AddStringWme(holdsParentObject, k_holdsAboveString, k_noneString);
}


void Disk::Detach()
{
	delete m_iLinkProfile;
	pTower = 0;
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

	//****** Change this to init to a string wme *******//
	//m_iLinkProfile->m_pPegIdentifier = manager->AddWMObject(0, k_worldPegString);
	string towerName;
	towerName = inName;
	m_iLinkProfile->m_pPegIdentifier = manager->AddStringWme(0, k_worldPegString, towerName);
	//string nameString;
	//nameString = m_name;
	//m_iLinkProfile->m_pPegName = manager->AddStringWme(m_iLinkProfile->GetTowerIdentifierObject(), k_nameString, nameString);
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



void Tower::PrintDiskAtRow(int row) const
{
	if(static_cast<int>(m_disks.size()) <= row)
		cout<<"--";
	else
	{
		cout << m_disks[row]->GetSize();
	}
}




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
	TgD::TSI_VERSION tsiVersion = TgD::TSI40;
	debugger = CreateTgD(agent, kernel, kFactory->GetKernelVersion(), tsiVersion, "Towers.exe");
	debugger->Init();

	//Source the agent's productions
	CommandLineInterface* commLine = new CommandLineInterface();
	commLine->SetKernel(kernel);
	char* pResponse = "\0";//we don't need to allocate space for this
	gSKI::Error* pError = new Error();

	commLine->DoCommand(agent, "pushd ../examples/towers", pResponse, pError);

	if(!commLine->DoCommand(agent, "source towers-of-hanoi-tree.soar", pResponse, pError))
	{
		cout << "current working directory is: ";
		commLine->DoCommand(agent, "pwd", pResponse, pError);
		cout << endl;
		cout << "Error in sourcing productions, press non-whitespace char and 'enter' to exit --------" << endl;
		cout << endl;
		string garbage;
		cin >> garbage;
		exit(1);
	}

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
	}//end-for-loop


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

