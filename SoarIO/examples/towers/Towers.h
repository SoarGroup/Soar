#ifndef TOWERS_HANOI_H
#define TOWERS_HANOI_H

#include <iostream>
#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
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

//Because of how the Towers of Hanoi productions are written, there will exactly 11 disks
const int maxNumDisks = 11;

#define USE_GSKI_DIRECT_NOT_SML

#ifdef USE_GSKI_DIRECT_NOT_SML

	#include "IgSKI_Wme.h"
	#include "IgSKI_WMObject.h"
	#include "IgSKI_Symbol.h"
	#include "IgSKI_InputLink.h"
	#include "IgSKI_OutputProcessor.h"
	#include "IgSKI_InputProducer.h"
	#include "IgSKI_WorkingMemory.h"

	using namespace gSKI;

#else //use SML layer

	#include "sml_Client.h"
	using namespace sml;
#endif



class Disk : public IInputProducer
{
public:

	Disk(IInputLink* pILink, int inSize, IWMObject* inPegIdObject, IWme* diskBeneath) : 
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
cout <<"I'm adding 'on' to holds...." << endl;
		m_pPeg = m_pWMemory->AddWmeObjectLink(holdsParentObject, "on", m_pPegId);
	//	m_actualPegWME = m_p

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


	void Detach()
	{
		//Release everything hanging off of "disk" parent wme
		const gSKI::ISymbol* parentSymbol = m_pDiskIdentifier->GetValue();
		IWMObject* parentObject = parentSymbol->GetObject();
		m_pWMemory->RemoveObject(parentObject);

		//Release everything hanging off of "holds" parent wme
		parentSymbol = m_pHoldsIdentifier->GetValue();
		parentObject = parentSymbol->GetObject();
		m_pWMemory->RemoveObject(parentObject);
	}

	//Replace the "on" and "above" wmes for the corresponding 
	//holds wme regardless of whether they have changed
	void Update(IWorkingMemory* pWMemory, IWMObject* object)
	{
		if(m_holdsNeedsToBeUpdated == false)
			return;

		// Get List of objects referencing this object with attribute "on"
		tIWmeIterator* onItr = object->GetWMEs("on");
		if(onItr->IsValid())
		{
			// Get the old "on" value
			IWme* oldTowerLink = onItr->GetVal();

			// Replace the wme attribute "on" with the new value
	//		const gSKI::ISymbol* pegParentSymbol = m_pPeg->GetValue();
	//		assert(pegParentSymbol);
	//		IWMObject* pegParentObject = pegParentSymbol->GetObject();

			m_pPeg->Release();
			m_pPeg = pWMemory->AddWmeObjectLink(object, "on", m_pPegId);
		}
		// Get List of objects referencing this object with attribute "above"
		tIWmeIterator* aboveItr = object->GetWMEs("above");

		if(aboveItr->IsValid())
		{	//Get the old "above" value
			IWme* oldDiskBeneath = aboveItr->GetVal();

			if(m_pDiskBeneath)
			{
				const gSKI::ISymbol* aboveParentSymbol = m_pHoldsDiskBeneath->GetValue();
				assert(aboveParentSymbol);
				IWMObject* aboveParentObject = aboveParentSymbol->GetObject();

				m_pHoldsDiskBeneath->Release();
				m_pHoldsDiskBeneath = pWMemory->AddWmeObjectLink(object, "above", aboveParentObject);
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

	int GetSize() const {return m_size;}

	void SetDiskBeneath(Disk* diskBeneath, IWMObject* pegObject)
	 {
		//TODO //FIXME @TODO release ref to wme of old disk beneath (if it's not zero)  ????

		if(diskBeneath)
			m_pDiskBeneath = diskBeneath->GetIdentifierWME();
		else
			m_pDiskBeneath = 0;

		m_pPegId = pegObject;
		m_holdsNeedsToBeUpdated = true;
	 }

private:
	Disk();
	Disk(const Disk&);
	Disk operator=(const Disk&);

	bool m_holdsNeedsToBeUpdated;

	IWorkingMemory* m_pWMemory;

	int m_size;//a convenience for other classes

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

typedef vector<Disk*> diskContainer_t;
typedef diskContainer_t::iterator diskItr_t;





class Tower
{
public:
	Tower(IInputLink* pILink, string name) : m_pILink(pILink)
	{
		IWorkingMemory* pWMemory = m_pILink->GetInputLinkMemory();
		IWMObject* pILinkRootObject;
		m_pILink->GetRootObject(&pILinkRootObject);
		m_pPegIdentifier = pWMemory->AddWmeNewObject(pILinkRootObject, "peg");

		m_pPegName = pWMemory->AddWmeString(GetTowerIdentifierObject(), "name", name.c_str());
	}

	~Tower()
	{
		m_pPegIdentifier->Release();
		m_disks.clear();//TODO @TODO release these too?
	}

	//will always add a smaller disk than the top, so new disk must on at end of container
	//disks that have just been created already have their disk beneath initialized, don't reset it
	void AddDisk(Disk* newDisk, bool justCreated)
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

	void RemoveTopDisk()
	{
		if(m_disks.size() != 0)
			m_disks.erase(--m_disks.end());
	}

	Disk* GetTopDisk()
	{
		if(m_disks.size() != 0)
		{
			assert(m_disks.back());
			return m_disks.back();
		}
		return 0;
	}

	IWMObject* GetTowerIdentifierObject() const
	{
		const gSKI::ISymbol* parentSymbol = m_pPegIdentifier->GetValue();
		IWMObject* parentObject = parentSymbol->GetObject();
		assert(parentObject);
		return parentObject;
	}

	int GetSize() const
	{
		return static_cast<int>(m_disks.size());
	}

	void PrintDiskAtRow(int row) const
	{
		if(static_cast<int>(m_disks.size()) <= row)
			cout<<"--";
		else
		{
			cout << m_disks[row]->GetSize();
		}
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

	IWme* m_pPegIdentifier;
		IWme* m_pPegName;
};





class HanoiWorld
{

public:
	HanoiWorld(IInputLink* pILink, bool graphicsOn = true, int inNumTowers = 3,  int inNumDisks = 11) : 
	   m_pILink(pILink), drawGraphics(graphicsOn)
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


	~HanoiWorld()
	{
//fixme call a destroy function on the towers first 
//TODO @TODO
		m_towers.clear();
	}


	//remove from the source tower, add to the destination tower
	bool MoveDisk(int sourceTower, int destinationTower)
	{
		Disk* movingDisk = m_towers[sourceTower]->GetTopDisk();
		if(!movingDisk)
			return false;

		assert(movingDisk);
		m_towers[sourceTower]->RemoveTopDisk();

		m_towers[destinationTower]->AddDisk(movingDisk, false);
		return true;
	}


	void Print()
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
		cout<<"======================" << endl;
	}

	bool AtGoalState()
	{
		if(m_towers[1]->GetSize() == maxNumDisks)
			return true;
		return false;
	}

private:
	typedef vector<Tower*> towerContainer_t;
	typedef towerContainer_t::iterator towerItr_t;
	towerContainer_t m_towers;

	IInputLink* m_pILink;
	bool drawGraphics;
};


#endif //TOWERS_HANOI_H

