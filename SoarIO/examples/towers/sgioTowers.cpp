#include "sgioTowers.h"




using sgio::WorkingMemory;
using sgio::SoarId;
using sgio::StringElement;
using sgio::IntElement;


//======================================================
//============ Disk Function Definitions ===============

Disk::Disk(int foo) //temp placeholder arg
{


}

void Detach()
{

}

	
	//Replace the "on" and "above" wmes for the corresponding 
	//holds wme
//void Update(IWorkingMemory* pWMemory, IWMObject* object);


void Disk::SetDiskBeneath(Disk* diskBeneath /*, IWMObject* pegObject*/)
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
void Tower::AddDisk(Disk* newDisk, bool justCreated){}

void Tower::RemoveTopDisk()
{


}

Disk* Tower::GetTopDisk()
{
	return 0 ;//fixme
}

int Tower::GetSize() const
{
	return 0 ;//fixme
}

void Tower::PrintDiskAtRow(int row) const
{


}

void Tower::PrintEntireTower()
{


}


//======================================================
//=========== Hanoi Function Definitions ===============

HanoiWorld::HanoiWorld(WorkingMemory* pWmemory, bool graphicsOn, int inNumTowers,  int inNumDisks)
{



}

HanoiWorld::~HanoiWorld()
{


}

//remove from the source tower, add to the destination tower
bool HanoiWorld::MoveDisk(int sourceTower, int destinationTower)
{
	return false;//fixme, placeholder return
}

void HanoiWorld::Print()
{

}

bool HanoiWorld::AtGoalState()
{
	return false;//fixme, placeholder return
}


