#include "AgnosticTowers.h"

#include "sgio_wmemem.h"


using sgio::WorkingMemory;
using sgio::SoarId;
using sgio::StringElement;
using sgio::IntElement;

namespace sgio_towers
{

class DiskInputLinkProfile
{
	friend class Disk;

	SoarId* m_pDiskIdentifier;
		IntElement* m_pSize;
		StringElement* m_pName;

	SoarId* m_pHoldsIdentifier;
		SoarId* m_pDiskBeneath;
		SoarId* m_pPeg;
		SoarId* m_pDisk;

	int m_size;
};
}//closes namespace

//======================================================
//============ Disk Function Definitions ===============

Disk::Disk(int inSize) : m_size(inSize) //temp placeholder arg
{
	/*  fixme todo must have access to working memory here
	//Initialize "disk" wmes
	m_pDiskIdentifier = m_pWMem->CreateIdWME(m_pWMem->GetILink(), k_diskIdentifierString);
		m_pSize = m_pWMem->CreateIntWME(m_pDiskIdentifier, k_diskSizeString);
		m_pName = m_pWMem->CreateStringWME(m_pDiskIdentifier, k_diskNameString);

	//Initialize corresponding "holds" wmes
	m_pHoldsIdentifier = m_pWMem->CreateIdWME(m_pWMem->GetILink(), k_holdsIdentifierString);
		//m_pDiskBeneath = m_pWMem->CreateIdWME(m_pHoldsIdentifier, )
		*/
}

void Detach()
{

}

	
	//Replace the "on" and "above" wmes for the corresponding 
	//holds wme
//void Update(IWorkingMemory* pWMemory, IWMObject* object);


void Disk::SetDiskBeneath(Disk* diskBeneath)
{


}

//======================================================
//============ Tower Function Definitions ==============

Tower::Tower()
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

 // fixme todo fill this in
}

Disk* Tower::GetTopDisk() const
{
	return 0 ;//fixme
}

int Tower::GetSize() const
{
	return 0 ;//fixme
}

void Tower::PrintDiskAtRow(int row) const
{

 // fixme todo fill this in
}


//======================================================
//=========== Hanoi Function Definitions ===============

HanoiWorld::HanoiWorld(/*WorkingMemory* pWmemory,*/ bool graphicsOn, int inNumTowers,  int inNumDisks)
{

 // fixme todo fill this in

}

HanoiWorld::~HanoiWorld()
{

 // fixme todo fill this in
}

void HanoiWorld::Run()
{
 // fixme todo fill this in

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


