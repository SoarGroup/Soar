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

/*string TowerNumToName(int towerNum)
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

	Disk(IInputLink* pILink, int inSize, int inTowerNum, IWme* inPegWme , IWme* diskBeneath) : 
		/*m_pInputLink(pILink),*/ m_size(inSize), m_towerNumber(inTowerNum), m_pPeg(inPegWme) , m_pDiskBeneath(diskBeneath)
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
		//add the holds identifier
		m_pHoldsIdentifier = pWMemory->AddWmeNewObject(pILinkRootObject, "holds");
		const gSKI::ISymbol* holdsParentSymbol = m_pHoldsIdentifier->GetValue();
		IWMObject* holdsParentObject = holdsParentSymbol->GetObject();
		
		//add holds wmes to parent object
		const gSKI::ISymbol* pegParentSymbol = m_pPeg->GetValue();
		IWMObject* pegParentObject = pegParentSymbol->GetObject();
		m_pPeg = pWMemory->AddWmeObjectLink(holdsParentObject, "on", pegParentObject);

		//the holds wme points back to its corresponding disk
		m_pDiskWme = pWMemory->AddWmeObjectLink(holdsParentObject, "disk", parentObject);

		if(m_pDiskBeneath)
		{
			IWMObject* pDiskBeneathIdentifier = m_pDiskBeneath->GetValue()->GetObject();
			m_pHoldsDiskBeneath = pWMemory->AddWmeObjectLink(holdsParentObject, "above", pDiskBeneathIdentifier);
		}
		else
			m_pHoldsDiskBeneath = pWMemory->AddWmeString(holdsParentObject, "above", "none");
	
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

			const gSKI::ISymbol* aboveParentSymbol = m_pHoldsDiskBeneath->GetValue();
			IWMObject* aboveParentObject = aboveParentSymbol->GetObject();

			if(m_pDiskBeneath)
			{
			//	m_pHoldsDiskBeneath = pWMemory->ReplaceIntWme(oldDiskBeneath, m_pDiskBeneath->GetValue()->GetInt());
				m_pHoldsDiskBeneath = pWMemory->ReplaceWmeObjectLink(oldDiskBeneath, aboveParentObject);
			}
			else
			{
				m_pHoldsDiskBeneath = pWMemory->ReplaceStringWme(oldDiskBeneath, "none");
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
		if(diskBeneath)
			m_pDiskBeneath = diskBeneath->GetIdentifierWME();

	 }

private:
	Disk();
	Disk(const Disk&);
	Disk operator=(const Disk&);

	//IInputLink* m_pInputLink;

	int m_towerNumber;
	int m_size;//a convenience for other classes
	IWme* m_pDiskIdentifier;
	IWme* m_pName;
	IWme* m_pSize;

	IWme* m_pDiskBeneath;

	//"holds" wmes
	IWme* m_pHoldsIdentifier;
	IWme* m_pHoldsDiskBeneath;
	IWme* m_pPeg;
	IWme* m_pDiskWme;
};

typedef vector<Disk*> diskContainer_t;
typedef diskContainer_t::iterator diskItr_t;


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
cout << "\t\tLooking at disk of size: " << disk->GetSize() << " on tower " << m_targetTower << endl;
			int size = disk->GetSize();

			if(size < m_smallestDiskSize)
			{
cout << "\t\t\t and it became the smallest so far... " <<endl;
				m_smallestDiskSize = size;
				m_pTopDisk = disk;
			}
			//TODO @TODO //FIXME, release this ref?
		}
	}

	Disk* GetTopDisk() const
	{ 
		cout << "When getting the top disk for tower " << m_targetTower << ", the top size was " << m_smallestDiskSize << endl;	
		return m_pTopDisk;
	}

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
		diskSizes.clear();
	}

	void operator() (Disk* inDisk)
	{
		if(inDisk->GetTowerNumber() == m_towerNumber)
		{	//The map will be ordered by size
			//Since no small disks can be under big disks, iterating through the map in one 
			//direction will always give you increasing/decreasing disks by size
			m_disks[inDisk->GetSize()] = inDisk;
			diskSizes.insert(diskSizes.begin(), inDisk->GetSize());
		}
	}

	/*void operator() (Disk* inDisk, int towerNumber)
	{
		m_towerNumber = towerNumber;

		if(inDisk->GetTowerNumber() == m_towerNumber)
		{	//The map will be ordered by size
			//Since no small disks can be under big disks, iterating through the map in one 
			//direction will always give you increasing/decreasing disks by size
			m_disks[inDisk->GetSize()] = inDisk;
		}
	}*/

	void PrintDisksAtRow(int row)
	{
		if(static_cast<int>(/*m_disks.size()*/ diskSizes.size()) <= row)
			cout<<"--";
		else
		{
			/*map<int, Disk*, std::less<int> >::iterator diskItr = m_disks.begin();
			for(int counter = 0; counter <= row; ++counter, ++diskItr)
			{
				if(counter == row)
					cout << ((*diskItr).second)->GetSize();
			}*/
			cout << diskSizes[row];
		}
	}

	int GetNumDisks()
	{
		return static_cast<int>(diskSizes.size());
	}
private:
	int m_towerNumber;
	map<int, Disk*, std::less<int> > m_disks;
	vector<int> diskSizes;
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
			{
				pWMemory->AddWmeString(parentObject, "name", "A");

				IWme* diskBeneathWME = 0;
				//Create disks
				for(int currentDiskSize = maxNumDisks; currentDiskSize > 0; --currentDiskSize)
				{
					//The disk currently at the back of the container is the "top" disk.  New, smaller, disks 
					//are inserted in front

					if(!m_disks.empty())
						diskBeneathWME = m_disks.front()->GetIdentifierWME();

					//create disks as belonging to left tower, above "none"
					Disk* disk = new Disk(pILink, currentDiskSize, towerNum, towerWME , diskBeneathWME);

					m_disks.insert(m_disks.begin(), disk);
					//m_disks.push_back()
				}

			}
			//==============
			//Middle tower
			//==============
			else if(towerNum == 1)
			{
				pWMemory->AddWmeString(parentObject, "name", "B");
			}

			//===============
			//"Right" tower
			//===============
			else
				pWMemory->AddWmeString(parentObject, "name", "C");

		}

cout << "World initialized.  This is what I look like:" << endl;
Print();
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

		topDiskOnSourceTower = for_each(m_disks.begin(), m_disks.end(), topDiskOnSourceTower);
cout << "Tower " << sourceTower << " has been initialized" <<endl;
topDiskOnSourceTower.GetTopDisk();
cout << "Tower " << destinationTower << " has been initialized" <<endl;
topDiskonDestinationTower.GetTopDisk();
cout << endl;


		topDiskonDestinationTower = for_each(m_disks.begin(), m_disks.end(), topDiskonDestinationTower);

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

		for(int row = 1; row <= maxNumDisks; ++row)
		{
			cout<<"(";
			towerA.PrintDisksAtRow(maxNumDisks -  row);
			cout<<")   (";
			towerB.PrintDisksAtRow(row);
			cout<<")   (";
			towerC.PrintDisksAtRow(row);
			cout<<")"<<endl;
		}
		cout<<"======================"<<endl;
	}

	bool AtGoalState()
	{
		CollectTowerDisks towerB(1);
		towerB = for_each(m_disks.begin(), m_disks.end(), towerB);
		if(towerB.GetNumDisks() == maxNumDisks)
			return true;
		return false;
	}

private:
	typedef vector<IWme*> towerContainer_t;
	typedef towerContainer_t::iterator towerItr_t;
	towerContainer_t m_towers;

	diskContainer_t m_disks;

	IInputLink* m_pILink;
	bool drawGraphics;
};

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

	void AddDisk(Disk* newDisk)
	{

	}

	void RemoveTopDisk()
	{


	}

	Disk* GetTopDisk()
	{
		//fixme
		return 0;
	}

	IWMObject* GetTowerIdentifierObject() const
	{
		const gSKI::ISymbol* parentSymbol = m_pPegIdentifier->GetValue();
		IWMObject* parentObject = parentSymbol->GetObject();
		return parentObject;
	}

private:
	std::vector<Disk*> m_disks;
	char m_name;
	int m_number;

	IInputLink* m_pILink;

	IWme* m_pPegIdentifier;
		IWme* m_pPegName;
};


#endif //TOWERS_HANOI_H

