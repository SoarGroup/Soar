
#ifndef AGNOSTIC_TOWERS_HANOI_H
#define AGNOSTIC_TOWERS_HANOI_H

#include <vector>

/*
#include <string.h>
#include <math.h>
#include <string>
#include <map>
*/

//Because of how the Towers of Hanoi productions are written, there will exactly 11 disks
const int maxNumDisks = 11;

const std::string k_diskIdentifierString	= "disk";
const std::string k_diskSizeString			= "size";
const std::string k_holdsIdentifierString	= "holds";
const std::string k_holdsOnString			= "on";
const std::string k_holdsAboveString		= "above";
const std::string k_noneString				= "none";
const std::string k_worldPegString			= "peg";
const std::string k_nameString				= "name";

class DiskInputLinkProfile;
class Tower;
struct DiskWMEs;

class Disk
{
public:

	Disk(Tower* tower, int size, Disk* diskBeneath);

	void Detach();

	//Replace the "on" and "above" wmes for the corresponding 
	//holds wme
	//void Update(IWorkingMemory* pWMemory, IWMObject* object);

	int GetSize() const {return m_size;}

	DiskInputLinkProfile* GetDiskInputLinkProfile() const { return m_iLinkProfile;}

	void SetDiskBeneath(Disk* diskBeneath);

private:

	//	Disk();
	Disk(const Disk&);
	Disk operator=(const Disk&);

	Disk* m_pDiskBeneath;
	DiskInputLinkProfile* m_iLinkProfile;
	int m_size;//a convenience for other classes
	Tower* pTower;

	friend class HanoiWorld;
};

typedef std::vector<Disk*> diskContainer_t;
typedef diskContainer_t::iterator diskItr_t;

class HanoiWorld;
class TowerInputLinkProfile;

class Tower
{
public:
	Tower(HanoiWorld* world, char name);

	~Tower();

	//will always add a smaller disk than the top, so new disk must on at end of container.
	//Disks that have just been created already have their disk beneath initialized, don't reset it
	void AddDisk(Disk* newDisk, bool justCreated);

	void RemoveTopDisk();

	Disk* GetTopDisk() const;

	int GetSize() const;

	TowerInputLinkProfile* GetInputLinkProfile() const {return m_iLinkProfile;}

	HanoiWorld* GetWorld() const {return pWorld;}

	void PrintDiskAtRow(int row) const;

private:
	std::vector<Disk*> m_disks;
	char m_name;
	int m_number;
	HanoiWorld* pWorld;
	TowerInputLinkProfile* m_iLinkProfile;
};


class IOManager;
class SoarAgent;

class HanoiWorld
{

public:
	HanoiWorld(bool graphicsOn = true, int inNumTowers = 3,  int inNumDisks = 11);

	~HanoiWorld();

	void Run();

	//remove from the source tower, add to the destination tower
	bool MoveDisk(int sourceTower, int destinationTower);

	void Print();

	bool AtGoalState();

	IOManager* GetIOManager() const {return ioManager;}

private:
	typedef std::vector<Tower*> towerContainer_t;
	typedef towerContainer_t::iterator towerItr_t;
	towerContainer_t m_towers;

	IOManager* ioManager;
	SoarAgent* m_agent;
	bool drawGraphics;
};


#endif //AGNOSTIC_TOWERS_HANOI_H

