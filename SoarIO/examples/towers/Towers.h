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
const int middleTowerNumber = 1;// towers are 0,1,2

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
/*
string TowerNumToName(int towerNum)
{
	string tempString;

	switch(towerNum)
	{

		case 0:
			tempString = "A";
			break;
		case 1:
			tempString = "B";
			break;
		case 2:
			tempString = "C";
			break;
		default:
			break;
	}
	return tempString;
}*/

class Disk : public IInputProducer
{
public:

	Disk(IInputLink* pILink, int inSize, int inTowerNum, IWme* inPegWme, IWme* diskBeneath) : 
		m_size(inSize), m_towerNumber(inTowerNum), m_pPeg(inPegWme), m_pDiskBeneath(diskBeneath)
	{
		//============================
		// Initialize "disk" wmes
		//============================
		IWorkingMemory* pWMemory = pILink->GetInputLinkMemory();
		IWMObject* pILinkRootObject;
		pILink->GetRootObject(&pILinkRootObject);
		//Add the disk identifier to the input link
		m_pDiskIdentifier = pWMemory->AddWmeNewObject(pILinkRootObject, "disk");

		const gSKI::ISymbol* parentSymbol = m_pDiskIdentifier->GetValue();
		IWMObject* parentObject = parentSymbol->GetObject();
		//attach subordinate wmes to disk identifier;
		m_pName = pWMemory->AddWmeInt(parentObject, "name", m_size);
		m_pSize = pWMemory->AddWmeInt(parentObject, "size", m_size);

		//============================
		// Initialize "holds" wmes
		//============================
		//add the holds identifer
		m_pHoldsIdentifier = pWMemory->AddWmeNewObject(pILinkRootObject, "holds");
		const gSKI::ISymbol* holdsParentSymbol = m_pHoldsIdentifier->GetValue();
		IWMObject* holdsParentObject = holdsParentSymbol->GetObject();
		
		//add holds wmes to parent object
		m_pPeg = pWMemory->AddWmeObjectLink(holdsParentObject, "on", m_pHoldsIdentifier->GetValue()->GetObject());

		if(m_pDiskBeneath)
			m_pHoldsDiskBeneath = pWMemory->AddWmeObjectLink(holdsParentObject, "above", m_pDiskBeneath->GetValue()->GetObject());
		else
			m_pDiskBeneath = pWMemory->AddWmeString(holdsParentObject, "above", "none");
	}


	void Detach(IWorkingMemory* pWMemory)
	{
		//Release everything hanging off of "disk" parent wme
		const gSKI::ISymbol* parentSymbol = m_pDiskIdentifier->GetValue();
		IWMObject* parentObject = parentSymbol->GetObject();
		pWMemory->RemoveObject(parentObject);

		//Release everything hanging off of "holds" parent wme
		parentSymbol = m_pHoldsIdentifier->GetValue();
		parentObject = parentSymbol->GetObject();
		pWMemory->RemoveObject(parentObject);
	}

	//Replace the "on" and "above" wmes for the corresponding 
	//holds wme regardless of whether they have changed
	void Update(IWorkingMemory* pWMemory, IWMObject* object)
	{
		// Get List of objects referencing this object with attribute "on"
		tIWmeIterator* onItr = object->GetWMEs("on");
		if(onItr->IsValid())
		{
			// Get the old "on" value
			IWme* oldTowerNumber = onItr->GetVal();

			// Replace the wme attribute "content" with the new value
			m_pPeg = pWMemory->ReplaceIntWme(oldTowerNumber, m_towerNumber);
		}

		// Get List of objects referencing this object with attribute "above"
		tIWmeIterator* aboveItr = object->GetWMEs("above");

		if(aboveItr->IsValid())
		{	//Get the old "above" value
			IWme* oldDiskBeneath = aboveItr->GetVal();

			if(m_pDiskBeneath)
			{
				m_pHoldsDiskBeneath = pWMemory->ReplaceIntWme(oldDiskBeneath, m_pDiskBeneath->GetValue()->GetInt());
			}
			//in the else case, there should already be a string "none" for this wme, so there's no need to change it
		}
	}


	 IWme* const GetIdentifierWME() const {return m_pDiskIdentifier;}

	 int GetTowerNumber() const {return m_towerNumber;}

	 int GetSize() const {return m_size;}

	 void SetTowerInfo(int newTowerNumber, Disk* diskBeneath)
	 {
		m_towerNumber = newTowerNumber;
		//TODO //FIXME @TODO release ref to wme of old disk beneath (if it's not zero)  ????
		m_pDiskBeneath = diskBeneath->GetIdentifierWME();
	 }

private:
	Disk();
	Disk(const Disk&);
	Disk operator=(const Disk&);

	int m_towerNumber;
	int m_size;//a convenience for other classes
	IWme* m_pDiskIdentifier;
	IWme* m_pName;
	IWme* m_pSize;

	IWme* m_pDiskBeneath;

	//from "holds" class
	IWme* m_pHoldsIdentifier;
	IWme* m_pHoldsDiskBeneath;
	IWme* m_pPeg;
};


//Function object for finding the top disk on a tower
//This is a smaller object that CollectTowerDisks, which can be used to get the top disk as well
class TopDiskOnTower
{
public:
	TopDiskOnTower(int towerNumber) : m_targetTower(towerNumber), m_pTopDisk(0), m_smallestDiskSize(maxNumDisks+1) {}

	//Iterate through all disks passed in, store those with matching tower number,
	//then find one with smallest size - that must be the top disk
	void operator()(Disk* disk)
	{
		if(disk->GetTowerNumber() == m_targetTower)
		{
			int size = disk->GetSize();
			if(size < m_smallestDiskSize)
			{
				size = m_smallestDiskSize;
				m_pTopDisk = disk;
			}
			//TODO @TODO //FIXME, release this ref?
		}
	}

	Disk* GetTopDisk() const { return m_pTopDisk; }

private:
	int m_targetTower;
	int m_smallestDiskSize;
	Disk* m_pTopDisk;
};





//used to store the disks in the tower
//can be used to get the top disk, to print the tower, etc
class CollectTowerDisks
{
public:
	CollectTowerDisks(int towerNumber) : m_towerNumber(towerNumber)	{}

	~CollectTowerDisks()
	{
		//TODO @TODO //FIXME release refs?
		m_disks.clear();
	}

	void operator() (Disk* inDisk)
	{
		if(inDisk->GetTowerNumber() == m_towerNumber)
		{	//The map will be ordered by size
			//Since no small disks can be under big disks, iterating through the map in one 
			//direction will always give you increasing/decreasing disks by size
			m_disks[inDisk->GetSize()] = inDisk;
		}
	}

	void operator() (Disk* inDisk, int towerNumber)
	{
		m_towerNumber = towerNumber;

		if(inDisk->GetTowerNumber() == m_towerNumber)
		{	//The map will be ordered by size
			//Since no small disks can be under big disks, iterating through the map in one 
			//direction will always give you increasing/decreasing disks by size
			m_disks[inDisk->GetSize()] = inDisk;
		}
	}

	void PrintDisksAtRow(int row)
	{
		if(static_cast<int>(m_disks.size()) <= row)
			cout<<"--";
		else
		{
			map<int, Disk*, std::less<int> >::iterator diskItr = m_disks.begin();
			for(int counter = 0; counter <= row; ++counter, ++diskItr)
			{
				if(counter == row)
					cout << ((*diskItr).second)->GetSize();
			}
		}
	}
private:
	int m_towerNumber;
	map<int, Disk*, std::less<int> > m_disks;
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
			IWme* towerWME = pWMemory->AddWmeNewObject(pILinkRootObject, "peg");
			m_towers.push_back(towerWME);
			const gSKI::ISymbol* parentSymbol = towerWME->GetValue();
			IWMObject* parentObject = parentSymbol->GetObject();

			//==============
			//"Left" tower
			//==============
			if(towerNum == 0)
				pWMemory->AddWmeString(parentObject, "name", "A");

			//==============
			//Middle tower
			//==============
			else if(towerNum == 1)
			{
				pWMemory->AddWmeString(parentObject, "name", "B");

				IWme* diskBeneathWME = 0;
				//Create disks
				for(int currentDiskSize = 1; currentDiskSize <= maxNumDisks; ++currentDiskSize)
				{
					//The disk currently at the back of the container is the "top" disk, and will be beneath
					//the newly created one
					if(!m_disks.empty())
						diskBeneathWME = m_disks.back()->GetIdentifierWME();

					//create disks as belonging to middle tower
					Disk* disk = new Disk(pILink, currentDiskSize, towerNum, towerWME, diskBeneathWME);

					IWme* diskUnderneathWME = 0;
					if(m_disks.size() != 0)
					{
						diskUnderneathWME = (m_disks.back())->GetIdentifierWME();
					}

					m_disks.push_back(disk);
				}

			}

			//===============
			//"Right" tower
			//===============
			else
				pWMemory->AddWmeString(parentObject, "name", "C");

		}
	}


	~HanoiWorld()
	{
		for(towerItr_t towerItr = m_towers.begin(); towerItr != m_towers.end(); ++towerItr)
		{
			//Release tower wme and all disk wmes attached to it
			(*towerItr)->Release();
		}
//fixme, remove parent objects instead of releaseing //TODO @TODO
		m_towers.clear();
		m_disks.clear();//TODO @TODO release these too?
	}


	//remove from the source tower, add to the destination tower
	void MoveDisk(int sourceTower, int destinationTower)
	{
		TopDiskOnTower topDiskOnSourceTower(sourceTower);
		TopDiskOnTower topDiskonDestinationTower(destinationTower);

		for_each(m_disks.begin(), m_disks.end(), topDiskOnSourceTower);
		for_each(m_disks.begin(), m_disks.end(), topDiskonDestinationTower);

		Disk* movingDisk = topDiskOnSourceTower.GetTopDisk();
		Disk* destinationTowerTopDisk = topDiskonDestinationTower.GetTopDisk();
		//update the holds wme corresponding to the moving disk.  This affectively moves the disk
		movingDisk->SetTowerInfo(destinationTower, destinationTowerTopDisk);
	}


	void Print()
	{ 
		CollectTowerDisks towerA(0);
		CollectTowerDisks towerB(1);
		CollectTowerDisks towerC(2);

		towerA = for_each(m_disks.begin(), m_disks.end(), towerA);
		towerB = for_each(m_disks.begin(), m_disks.end(), towerB);
		towerC = for_each(m_disks.begin(), m_disks.end(), towerC);

		for(int row = maxNumDisks - 1; row >= 0; row--)
		{
			cout<<"(";
			towerA.PrintDisksAtRow(row);
			cout<<")   (";
			towerB.PrintDisksAtRow(row);
			cout<<")   (";
			towerC.PrintDisksAtRow(row);
			cout<<")"<<endl;
		}
		cout<<"======================"<<endl;
	}

private:
	typedef vector<IWme*> towerContainer_t;
	typedef towerContainer_t::iterator towerItr_t;
	towerContainer_t m_towers;

	typedef vector<Disk*> diskContainer_t;
	typedef diskContainer_t::iterator diskItr_t;
	diskContainer_t m_disks;

	IInputLink* m_pILink;
	bool drawGraphics;
};

#endif //TOWERS_HANOI_H

