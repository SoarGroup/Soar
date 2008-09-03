/////////////////////////////////////////////////////////////////
// gSKITowers class file.
//
// Author: Devvan Stokes, University of Michigan
// Date  : October 2004
//
// These classes define the disks, towers (or pegs), and the 
// Towers of Hanoi game world that contains them
//
/////////////////////////////////////////////////////////////////
#ifndef TOWERS_HANOI_H
#define TOWERS_HANOI_H

#include <vector>

// helps quell warnings
#ifndef unused
#define unused(x) (void)(x)
#endif

//Because of how the Towers of Hanoi productions are written, there will exactly 11 disks
const int maxNumDisks = 11;

// SML expects const char* more than std::string, so let's just define these as string constants.
// The alternative is to call .c_str() where we use these strings which would also be easy to do.
static const char* k_diskIdentifierString	= "disk";
static const char* k_diskSizeString			= "size";
static const char* k_holdsIdentifierString	= "holds";
static const char* k_holdsOnString			= "on";
static const char* k_holdsAboveString		= "above";
static const char* k_noneString				= "none";
static const char* k_worldPegString			= "peg";
static const char* k_nameString				= "name";

/*************************************************************
* @brief	forward declaration for class that wraps up the 
*			wmes associated with a disk
*************************************************************/
class DiskInputLinkProfile;
class Tower;

class Disk
{
public:
	/*************************************************************
	* @brief	Public constructor initializes state  
	* @param	tower		pointer to tower that this disk is on
	* @param	inSize		size of disk
	* @param	diskBeneath	pointer to disk underneath, zero if none
	*************************************************************/
	Disk(Tower* tower, int size, Disk* diskBeneath);

	/*************************************************************
	* @brief	Removes the disk's wmes from working memory
	*************************************************************/
	virtual void Detach();

	virtual int GetSize() const {return m_size;}

	virtual DiskInputLinkProfile* GetDiskInputLinkProfile() const { return m_iLinkProfile;}

	Tower* GetTower() const { return pTower; }

	/*************************************************************
	* @brief	Updates the disk's tower and disk pointers 
	* @param	diskBeneath	pointer to disk directly beneath this 
	*						one, zero if none
	* @param	tower	pointer to tower that disk is currently on
	*************************************************************/
	virtual void Update(Disk* diskBeneathMe, Tower* tower);

protected:

	Disk();
	Disk(const Disk&);
	Disk operator=(const Disk&);

	DiskInputLinkProfile* m_iLinkProfile;
	int m_size;
	Tower* pTower;

	friend class HanoiWorld;
private:
	void dummy() {
		unused(k_diskIdentifierString);
		unused(k_diskSizeString);
		unused(k_holdsIdentifierString);
		unused(k_holdsOnString);
		unused(k_holdsAboveString);
		unused(k_noneString);
		unused(k_worldPegString);
		unused(k_nameString);

	}
};

typedef std::vector<Disk*> diskContainer_t;
typedef diskContainer_t::iterator diskItr_t;

class HanoiWorld;

/*************************************************************
* @brief	forward declaration for class that wraps up the 
*			wmes associated with a Tower
*************************************************************/
class TowerInputLinkProfile;

class Tower
{
public:

	/*************************************************************
	* @brief	Public constructor initializes state
	* @param	inWorld		pointer to the game world
	* @param	inName		the name of this tower
	*************************************************************/
	Tower(HanoiWorld* world, char name);

	/*************************************************************
	* @brief	Clean up the tower's state as well as any contained
	*			disks
	*************************************************************/
	virtual ~Tower();

	/*************************************************************
	* @brief	Add disk to top of tower. Added disk
	*			is always smaller than the current top.  Disks that 
	*			have just been created do not need to be updated.
	* @param	newDisk		pointer to disk to be added
	* @param	justCreated	whether or not this disk was just constructed
	*************************************************************/
	virtual void AddDisk(Disk* newDisk, bool justCreated);

	/*************************************************************
	* @brief	Remove the top disk, which will be at the back
	*			of the container
	*************************************************************/
	virtual void RemoveTopDisk();

	/*************************************************************
	* @returns	pointer to the disk at top of tower
	*************************************************************/
	virtual Disk* GetTopDisk() const;

	char GetName() const { return m_name; }

	int GetSize() const { return static_cast<int>(m_disks.size()); }

	virtual TowerInputLinkProfile* GetInputLinkProfile() const {return m_iLinkProfile;}

	virtual HanoiWorld* GetWorld() const {return pWorld;}

	virtual void PrintDiskAtRow(int row) const;

protected:
	std::vector<Disk*> m_disks;
	char m_name;
	int m_number;
	HanoiWorld* pWorld;
	TowerInputLinkProfile* m_iLinkProfile;
};

/*************************************************************
* @brief	forward declaration for class that wraps up the 
*			working memory and/or the input link
*************************************************************/
class IOManager;

/*************************************************************
* @brief	forward declaration for class that wraps up the 
*			actual Soar Kernel agent
*************************************************************/
class SoarAgent;

class HanoiWorld
{

public:
	/*************************************************************
	* @brief	Public constructor initializes state
	* @param	graphicsOn	optional bool for whether or not to print
	*						graphics during game execution
	* @param	inNunTowers	optional amount of towers for game
	* @param	inNumDisks	optional amount of disks to play with
	*************************************************************/
	HanoiWorld(bool remoteConnection = false, bool graphicsOn = true, int inNumTowers = 3,  int inNumDisks = 11);

	/*************************************************************
	* @brief	cleans up state, including contained Towers
	*************************************************************/
	virtual ~HanoiWorld();

	/*************************************************************
	* @brief	calls an interface-specific Run function in agent
	*************************************************************/
	virtual void Run();

	/*************************************************************
	* @brief	move disk from source tower to destination tower
	* @param	sourceTower			numerical ID of source tower
	* @param	destinationTower	numerical ID of destination tower
	* @returns	true if there is a disk to move, false otherwise
	*************************************************************/
	virtual bool MoveDisk(int sourceTower, int destinationTower);

	/*************************************************************
	* @brief	prints all towers and disks
	*************************************************************/
	virtual void Print();

	/*************************************************************
	* @returns	true if all disks are on goal tower, else false 
	*************************************************************/
	virtual bool AtGoalState();

	virtual IOManager* GetIOManager() const {return ioManager;}

	/*************************************************************
	* @brief	some interfaces may need to perform additional actions
	*			after the goal state has been reached.	Those	
	*			operations can be performed from here
	*************************************************************/
	virtual void EndGameAction();

protected:
	typedef std::vector<Tower*> towerContainer_t;
	typedef towerContainer_t::iterator towerItr_t;
	towerContainer_t m_towers;

	IOManager* ioManager;
	SoarAgent* m_agent;
	bool drawGraphics;
};


#endif //TOWERS_HANOI_H

