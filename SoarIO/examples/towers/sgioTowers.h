
#ifndef SGIO_TOWERS_HANOI_H
#define SGIO_TOWERS_HANOI_H

#include <vector>
/*#include <iostream>
#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <string>
#include <map>
#include <cassert>*/

/*using std::cin;
using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::less;
using std::for_each;*/

//SGIO directives
#include "sgio_wmemem.h"



//Because of how the Towers of Hanoi productions are written, there will exactly 11 disks
const int maxNumDisks = 11;

class Disk
{
public:

	Disk(int fooPlaceholderFixme);

	void Detach();

	//Replace the "on" and "above" wmes for the corresponding 
	//holds wme
	//void Update(IWorkingMemory* pWMemory, IWMObject* object);

	int GetSize() const {return m_size;}

	void SetDiskBeneath(Disk* diskBeneath /*, IWMObject* pegObject*/);

private:
//	Disk();
	Disk(const Disk&);
	Disk operator=(const Disk&);

	bool m_holdsNeedsToBeUpdated;

	//IWorkingMemory* m_pWMemory;

	int m_size;//a convenience for other classes
/*
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
	IWme* m_pDiskWme;*/
};

typedef std::vector<Disk*> diskContainer_t;
typedef diskContainer_t::iterator diskItr_t;


class Tower
{
public:
	Tower();

	~Tower();

	//will always add a smaller disk than the top, so new disk must on at end of container
	//disks that have just been created already have their disk beneath initialized, don't reset it
	void AddDisk(Disk* newDisk, bool justCreated);

	void RemoveTopDisk();

	Disk* GetTopDisk();

	int GetSize() const;

	void PrintDiskAtRow(int row) const;

	void PrintEntireTower();

private:
	std::vector<Disk*> m_disks;
	char m_name;
	int m_number;

	sgio::SoarId* m_pPegIdentifier;
		sgio::StringElement* m_pPegName;
};





class HanoiWorld
{

public:
	HanoiWorld(WorkingMemory* wmem, bool graphicsOn = true, int inNumTowers = 3,  int inNumDisks = 11);

	~HanoiWorld();

	//remove from the source tower, add to the destination tower
	bool MoveDisk(int sourceTower, int destinationTower);

	void Print();

	bool AtGoalState();

private:
	typedef std::vector<Tower*> towerContainer_t;
	typedef towerContainer_t::iterator towerItr_t;
	towerContainer_t m_towers;

	//IInputLink* m_pILink;
	bool drawGraphics;
};


#endif //SGIO_TOWERS_HANOI_H

