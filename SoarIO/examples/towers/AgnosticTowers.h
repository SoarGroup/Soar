
#ifndef AGNOSTIC_TOWERS_HANOI_H
#define AGNOSTIC_TOWERS_HANOI_H

#include <vector>
/*

#include <string.h>

#include <math.h>

#include <string>
#include <map>
*/

/*using std::cin;
using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::less;
using std::for_each;*/




//Because of how the Towers of Hanoi productions are written, there will exactly 11 disks
const int maxNumDisks = 11;

class DiskInputLinkProfile;


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
	DiskInputLinkProfile* m_iLinkProfile;
	int m_size;//a convenience for other classes
};

typedef std::vector<Disk*> diskContainer_t;
typedef diskContainer_t::iterator diskItr_t;


class Tower
{
public:
	Tower();

	~Tower();

	//will always add a smaller disk than the top, so new disk must on at end of container.
	//Disks that have just been created already have their disk beneath initialized, don't reset it
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
};





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

private:
	typedef std::vector<Tower*> towerContainer_t;
	typedef towerContainer_t::iterator towerItr_t;
	towerContainer_t m_towers;

	//IInputLink* m_pILink;
	bool drawGraphics;
};


#endif //AGNOSTIC_TOWERS_HANOI_H

