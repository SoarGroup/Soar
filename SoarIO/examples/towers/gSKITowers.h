
#ifndef GSKI_TOWERS_HANOI_H
#define GSKI_TOWERS_HANOI_H

#include "Towers.h"

//Debugger directives
#include "TgD.h"
#include "tcl.h"
#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>
#define TGD_SLEEP Sleep
#else
#include <unistd.h>
#define TGD_SLEEP usleep
#endif

class HanoiWorld;
class TowerInputLinkProfile;

/*class GSKITower : public Tower
{
public:
	GSKITower(HanoiWorld* world, char name);

	~GSKITower();

	//will always add a smaller disk than the top, so new disk must on at end of container.
	//Disks that have just been created already have their disk beneath initialized, don't reset it
	void AddDisk(Disk* newDisk, bool justCreated);

	void RemoveTopDisk();

	Disk* GetTopDisk() const;

	int GetSize() const;

	TowerInputLinkProfile* GetInputLinkProfile() const {return m_iLinkProfile;}

	HanoiWorld* GetWorld() const {return pWorld;}

	void PrintDiskAtRow(int row) const;
};*/


#endif //GSKI_TOWERS_HANOI_H

