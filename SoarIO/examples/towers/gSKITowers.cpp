#include "AgnosticTowers.h"

#include <iostream>
#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <string>
#include <map>
#include <cassert>

using std::cin;
using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::less;
using std::for_each;



#include "IgSKI_Wme.h"
#include "IgSKI_WMObject.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_InputLink.h"
#include "IgSKI_OutputProcessor.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_WorkingMemory.h"

using namespace	gSKI;

namespace gski_towers
{
	class DiskInputLinkProfile
	{
		friend class Disk;

		//"disk" wmes
		IWme* m_pDiskIdentifier;
			IWme* m_pName;
			IWme* m_pSize;

		IWme* m_pDiskBeneath;	//wme of actual disk beneath
		IWMObject* m_pPegId;	//object of wme of actual ped

		//"holds" wmes
		IWme* m_pHoldsIdentifier;
			IWme* m_pHoldsDiskBeneath;	//disk wme that appears on the holds structure on the input link
			IWme* m_pPeg;				//peg wme that appears on the holds structure on input link
			IWme* m_pDiskWme;
	};

	class TowerInputLinkProfile
	{
		IWme* m_pPegIdentifier;
		IWme* m_pPegName;
	};
}//closes namespace



//======================================================
//============ Disk Function Definitions ===============

Disk::Disk(IInputLink* pILink, int inSize, IWMObject* inPegIdObject, IWme* diskBeneath) : 
		m_size(inSize), m_pPegId(inPegIdObject), m_pDiskBeneath(diskBeneath)
{
	
		//============================
		// Initialize "disk" wmes
		//============================
		m_pWMemory = pILink->GetInputLinkMemory();
		IWMObject* pILinkRootObject;
		pILink->GetRootObject(&pILinkRootObject);
		//Add the disk identifier to the input link
		m_pDiskIdentifier = m_pWMemory->AddWmeNewObject(pILinkRootObject, "disk");

		const gSKI::ISymbol* parentSymbol = m_pDiskIdentifier->GetValue();
		IWMObject* parentObject = parentSymbol->GetObject();
		//attach subordinate wmes to disk identifier;
		m_pName = m_pWMemory->AddWmeInt(parentObject, "name", m_size);
		m_pSize = m_pWMemory->AddWmeInt(parentObject, "size", m_size);

		//============================
		// Initialize "holds" wmes
		//============================
		//add the holds identifier
		m_pHoldsIdentifier = m_pWMemory->AddWmeNewObject(pILinkRootObject, "holds");
		const gSKI::ISymbol* holdsParentSymbol = m_pHoldsIdentifier->GetValue();
		IWMObject* holdsParentObject = holdsParentSymbol->GetObject();
		
		//add holds wmes to parent object
		m_pPeg = m_pWMemory->AddWmeObjectLink(holdsParentObject, "on", m_pPegId);

		//the holds wme points back to its corresponding disk
		m_pDiskWme = m_pWMemory->AddWmeObjectLink(holdsParentObject, "disk", parentObject);

		if(m_pDiskBeneath)
		{
			IWMObject* pDiskBeneathIdentifier = m_pDiskBeneath->GetValue()->GetObject();
			m_pHoldsDiskBeneath = m_pWMemory->AddWmeObjectLink(holdsParentObject, "above", pDiskBeneathIdentifier);
		}
		else
			m_pHoldsDiskBeneath = m_pWMemory->AddWmeString(holdsParentObject, "above", "none");

		m_holdsNeedsToBeUpdated = false;
}


void Disk::Detach()
{
	//Release everything this is touching that doesn't belong to us
	if(m_pDiskBeneath)
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
}

	//Replace the "on" and "above" wmes for the corresponding 
	//holds wme
	void Update(IWorkingMemory* pWMemory, IWMObject* object)
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
}


IWme* const GetIdentifierWME() const {return m_pDiskIdentifier;}

IWme* const GetHoldsIdentifierWME() const {return m_pHoldsIdentifier;}

int Disk::GetSize() const {return m_size;}

void SetDiskBeneath(Disk* diskBeneath, IWMObject* pegObject)
{
	//TODO //FIXME @TODO release ref to wme of old disk beneath (if its not zero)  ????

	if(diskBeneath)
		m_pDiskBeneath = diskBeneath->GetIdentifierWME();
	else
		m_pDiskBeneath = 0;

	m_pPegId = pegObject;
	m_holdsNeedsToBeUpdated = true;
}



//======================================================
//============ Tower Function Definitions ==============


Tower::Tower(/*IInputLink* pILink, string name*/) //: m_pILink(pILink)
{
	IWorkingMemory* pWMemory = m_pILink->GetInputLinkMemory();
	IWMObject* pILinkRootObject;
	m_pILink->GetRootObject(&pILinkRootObject);
	m_pPegIdentifier = pWMemory->AddWmeNewObject(pILinkRootObject, "peg");

	m_pPegName = pWMemory->AddWmeString(GetTowerIdentifierObject(), "name", name.c_str());
}

Tower::~Tower()
{

	IWorkingMemory* pWorkingMem = m_pILink->GetInputLinkMemory();
	pWorkingMem->RemoveWme(m_pPegIdentifier);
	for(vector<Disk*>::iterator diskItr = m_disks.begin(); diskItr != m_disks.end(); ++diskItr)
	{
		(*diskItr)->Detach();
	}
	m_disks.clear();
	//can't release ILink ptr
}


//will always add a smaller disk than the top, so new disk must on at end of container
//disks that have just been created already have their disk beneath initialized, don't reset it
void Tower::AddDisk(Disk* newDisk, bool justCreated)
{
	assert(newDisk);
	if(!justCreated)
	{
		if(!m_disks.empty())
			newDisk->SetDiskBeneath(m_disks.back(), m_pPegIdentifier->GetValue()->GetObject());
		else
			newDisk->SetDiskBeneath(0, m_pPegIdentifier->GetValue()->GetObject());
	}

	m_disks.push_back(newDisk);
}

void Tower::RemoveTopDisk()
{
	if(m_disks.size() != 0)
		m_disks.erase(--m_disks.end());
}


Disk* Tower::GetTopDisk()
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



IWMObject* GetTowerIdentifierObject() const
{
	const gSKI::ISymbol* parentSymbol = m_pPegIdentifier->GetValue();
	IWMObject* parentObject = parentSymbol->GetObject();
	assert(parentObject);
	return parentObject;
}

	void PrintEntireTower()
	{
		for(vector<Disk*>::iterator fooItr = m_disks.begin(); fooItr != m_disks.end(); ++fooItr)
			cout << (*fooItr)->GetSize() << endl;
		cout << endl;
	}

private:
	vector<Disk*> m_disks;
	char m_name;
	int m_number;

	IInputLink* m_pILink;







HanoiWorld::HanoiWorld(/*IInputLink* pILink,*/ bool graphicsOn = true, int inNumTowers = 3,  int inNumDisks = 11) : 
	/*m_pILink(pILink),*/ drawGraphics(graphicsOn)
{
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
			Tower* tower = new Tower(pILink, "A");
			assert(tower);
			IWme* diskBeneathWME = 0;
			//Create disks
			for(int currentDiskSize = maxNumDisks; currentDiskSize > 0; --currentDiskSize)
			{
				//The disk currently at the front of the container is the "bottom" disk.  New, smaller, disks 
				//are inserted in back
				IWMObject* towerIdObject = tower->GetTowerIdentifierObject();
				assert(towerIdObject);
				IWme* towerTopDiskWme = 0;

				if(tower->GetSize() != 0)
					towerTopDiskWme = tower->GetTopDisk()->GetIdentifierWME();

				Disk* disk = new Disk(pILink, currentDiskSize, towerIdObject, towerTopDiskWme);

				pILink->AddInputProducer(disk->GetHoldsIdentifierWME()->GetValue()->GetObject(), disk);
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
			Tower* tower = new Tower(pILink, "B");
			m_towers.push_back(tower);
		}

		//===============
		//"Right" tower
		//===============
		else
		{
			Tower* tower = new Tower(pILink, "C");
			m_towers.push_back(tower);
		}
	}
//Print();

}


HanoiWorld::~HanoiWorld()
{
	for(towerItr_t towerItr = m_towers.begin(); towerItr != m_towers.end(); ++towerItr)
		(*towerItr)->~Tower();
	m_towers.clear();
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

bool HanoiWorld::()AtGoalState()
{
	if(m_towers[2]->GetSize() == maxNumDisks)
		return true;
	return false;
}



