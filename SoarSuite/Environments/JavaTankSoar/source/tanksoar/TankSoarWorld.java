package tanksoar;

import java.util.*;

import simulation.*;
import sml.*;
import utilities.*;

public class TankSoarWorld extends World implements WorldManager {
	private static final String kTagTankSoarWorld = "tanksoar-world";

	private static final String kTagCells = "cells";
	private static final String kParamWorldSize = "world-size";
	private static final String kParamType = "type";
	
	static final String kTypeWall = "wall";
	static final String kTypeEmpty = "empty";
	static final String kTypeEnergyRecharger = "energy";
	static final String kTypeHealthRecharger = "health";
	
	static final int kKillAward = 3;
	static final int kKillPenalty = -2;
	private static final int kMissileHitAward = 2;
	private static final int kMissileHitPenalty = -1;
	private static final int kWinningPoints = 50;
	
	private static final int kMaxMissilePacks = 3;
	private static final double kMisslePackRespawn = 0.05;
	static final int kMissilePackSize = 7;
	private int m_NumMissilePacks = 0;
	
	Random m_Random;
	RelativeDirections m_RD = new RelativeDirections();
	int m_MaxManhattanDistance;
	private static final int kMaxSmellDistance = 7;

	private TankSoarSimulation m_Simulation;
	private TankSoarCell[][] m_World;
	private boolean m_PrintedStats;
	private Tank[] m_Tanks;
	
	public class Missiles {
		LinkedList m_Flying;
		
		public Missiles() {
			reset();
		}
		
	   	public void reset() {
			m_Flying = new LinkedList();
	   	}
	   	
	   	public void removeWallCollisions() {
	   		ListIterator iter = m_Flying.listIterator();
	   		while (iter.hasNext()) {
	   			Missile missile = (Missile)iter.next();
	   			MapPoint location = missile.getCurrentLocation();
	   			if (getCell(location).isWall()) {
		   			getCell(location).setRedraw();
	   				iter.remove();
	   				continue;
	   			}
	   		}
	   	}
	   	
	   	public void moveMissiles() {
	   		ListIterator iter = m_Flying.listIterator();
	   		while (iter.hasNext()) {
	   			Missile missile = (Missile)iter.next();
	   			MapPoint location = missile.getCurrentLocation();
	   			m_RD.calculate(missile.getDirection());

	   			getCell(location).setRedraw();
	   			location.travel(m_RD.forward);
	   			
	   			if (missile.getFlightPhase() == 2) {
	   			}
	   			
	   			// Handle special wall collision here so that missile threatens properly
	   			// on 3rd flight phase
	   			if (missile.getFlightPhase() == 2) {
		   			getCell(location).setRedraw();
		   			if (getCell(location).isWall()) {
		   				iter.remove();
		   				continue;
		   			}
		   			location.travel(m_RD.forward);
	   			}
	   			
	   			missile.incrementFlightPhase();
	   		}
	   	}

	   	public void fireMissile(Tank owner) {
   			m_RD.calculate(owner.getFacingInt());
   			MapPoint location = new MapPoint(owner.getLocation());
   			location.travel(m_RD.forward);
			m_Flying.addLast(new Missile(location, owner.getFacingInt(), owner));
	   	}
	   	
	   	public int checkIncoming(MapPoint location) {
	   		int incoming = 0;
	   		ListIterator iter = m_Flying.listIterator();
	   		while (iter.hasNext()) {
	   			Missile missile = (Missile)iter.next();
	   			if (missile.getCurrentLocation().x == location.x) {
	   				// We're in the same row
	   				if (missile.getCurrentLocation().y < location.y) {
	   					// The missile is above us
	   					if (missile.getDirection() == WorldEntity.kSouthInt) {
	   						// and travelling toward us
	   						incoming |= WorldEntity.kNorthInt;
	   					}
	   				} else {
	   					// The missile is below us
	   					if (missile.getDirection() == WorldEntity.kNorthInt) {
	   						// and travelling toward us
	   						incoming |= WorldEntity.kSouthInt;
	   					}
	   				}
	   			}
	   			if (missile.getCurrentLocation().y == location.y) {
	   				// We're in the same column
	   				if (missile.getCurrentLocation().x < location.x) {
	   					// The missile is to our left
	   					if (missile.getDirection() == WorldEntity.kEastInt) {
	   						// and travelling toward us
	   						incoming |= WorldEntity.kWestInt;
	   					}
	   				} else {
	   					// The missile is to our right
	   					if (missile.getDirection() == WorldEntity.kWestInt) {
	   						// and travelling toward us
	   						incoming |= WorldEntity.kEastInt;
	   					}
	   				}
	   			}
	   		}
	   		return incoming;
	   	}
	   	
	   	public Tank checkSpecialHit(MapPoint location, int tankMove) {
	   		ListIterator iter = m_Flying.listIterator();
	   		while (iter.hasNext()) {
	   			Missile missile = (Missile)iter.next();
	   			if (location.equals(missile.getCurrentLocation())) {
	   				m_RD.calculate(tankMove);
	   				if (missile.getDirection() == m_RD.backward) {
		   				Tank owner = missile.getOwner();
	   					iter.remove();
		   				return owner;
	   				}
	   			}
	   		}
	   		return null;
	   	}
	   	
	   	public Tank checkHit(MapPoint location, boolean remove) {
	   		ListIterator iter = m_Flying.listIterator();
	   		while (iter.hasNext()) {
	   			Missile missile = (Missile)iter.next();
	   			if (location.equals(missile.getCurrentLocation())) {
	   				Tank owner = missile.getOwner();
	   				if (remove) {
	   					iter.remove();
	   				}
	   				return owner;
	   			}
	   			if (missile.getFlightPhase() == 2) {
		   			MapPoint missileLoc = new MapPoint(missile.getCurrentLocation());
		   			missileLoc.travel(missile.getDirection());
		   			if (location.equals(missileLoc)) {
		   				Tank owner = missile.getOwner();
		   				if (remove) {
		   					iter.remove();
		   				}
		   				return owner;
		   			}
	   			}
	   		}
	   		return null;
	   	}
	   	
	   	public Missile[] getMissiles() {
	   		if (m_Flying.size() == 0) {
	   			return null;
	   		}
	   		return (Missile[])m_Flying.toArray(new Missile[0]);
	   	}
	}
	
	private Missiles m_Missiles = new Missiles();
	
	public Missile[] getMissiles() {
		return m_Missiles.getMissiles();
	}
	
   	public TankSoarWorld(TankSoarSimulation simulation) {
		m_Simulation = simulation;
		m_Random = simulation.isRandom() ? new Random() : new Random(0);
	}
	
	public boolean load(String mapFile) {
		m_PrintedStats = false;
		m_NumMissilePacks = 0;
		TankSoarCell.s_EnergyChargerCreated = false;
		TankSoarCell.s_HealthChargerCreated = false;
		
		try {
			// Open file
			JavaElementXML root = JavaElementXML.ReadFromFile(mapFile);
			
			if (!root.getTagName().equalsIgnoreCase(kTagTankSoarWorld)) {
				throw new Exception("Not a TankSoar map!");
			}
			// TODO: Check version
			
			// Create map
			JavaElementXML cells = root.findChildByNameThrows(kTagCells);
			
			// Get dimentions
			m_WorldSize = cells.getAttributeIntThrows(kParamWorldSize);
			
			// Figure out maximum manhattan distance:
			// Maximum manhattan distance to another tank:
			// (Map width x 2) - 2
			// but we can subtract 2 from the width because of the outer wall:
			// ((Map width - 2) x 2) - 2
			m_MaxManhattanDistance = ((m_WorldSize - 2) * 2) - 2;
			
			// Create map array
			m_World = new TankSoarCell[m_WorldSize][m_WorldSize];
			
			// generate world
			generateWorldFromXML(cells);

		} catch (Exception e) {
			m_Logger.log("Error loading map: " + e.getMessage());
			return false;
		}
		
		// Place rechargers
		if (!TankSoarCell.s_HealthChargerCreated) {
			getCell(findStartingLocation()).setHealth();
		}
		if (!TankSoarCell.s_EnergyChargerCreated) {
			getCell(findStartingLocation()).setEnergy();
		}
		
		while (m_NumMissilePacks < kMaxMissilePacks) {
			spawnMissilePack();
		}
		m_Missiles.reset();
		resetTanks();
		
		m_Logger.log(mapFile + " loaded.");
		return true;
	}
	
	private void spawnMissilePack() {
		TankSoarCell cell;
		do {
			cell = getCell(findStartingLocation());
		} while (cell.hasContents() || !cell.isOpen());
		cell.setMissilePack();
		++m_NumMissilePacks;
	}
	
	void pickUpMissiles(Tank tank) {
		tank.addMissiles(TankSoarWorld.kMissilePackSize);
		--m_NumMissilePacks;
	}
	
	private void generateWorldFromXML(JavaElementXML cells) throws Exception {
		for(int row = 0; row < m_WorldSize; ++row) {
			//String rowString = new String();
			for (int col = 0; col < m_WorldSize; ++col) {
				try {
					m_World[row][col] = new TankSoarCell(cells.getChild(row).getChild(col).getAttributeThrows(kParamType));
					//rowString += m_World[row][col];
				} catch (Exception e) {
					throw new Exception("Error (generateWorldFromXML) on row: " + row + ", column: " + col);
				}
			}
			//m_Logger.log(rowString);
		}
	}
	
	public Tank[] getTanks() {
		return m_Tanks;
	}
	
	public void destroyEntity(WorldEntity entity) {
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].getName() == entity.getName()) {
				destroyTank(m_Tanks[i]);
				return;
			}
		}
		m_Logger.log("Couldn't find entity name match for " + entity.getName() + ", ignoring.");
	}
	
	public WorldEntity[] getEntities() {
		return getTanks();
	}
	
	private void resetTanks() {
		if (m_Tanks == null) {
			return;
		}
		for (int i = 0; i < m_Tanks.length; ++i) {
			MapPoint location = m_Tanks[i].getInitialLocation();
			if (location != null && getCell(location).isBlocked()) {
				m_Logger.log("Initial location " + location + " is blocked, going random.");
				location = null;
			}
			if (location == null) {
				location = findStartingLocation();
			}
			m_Tanks[i].setLocation(location);
			// Put tank on map
			if (getCell(location).containsMissilePack()) {
				--m_NumMissilePacks;
			}
			getCell(location).setTank(m_Tanks[i]);
			m_Tanks[i].setPoints(0);
			m_Tanks[i].reset();
			m_Tanks[i].initSoar();
		}
		
		// Update all Tank sensors
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].updateSensors(this);
		}
		
		// Write out input links
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].writeInputLink();
		}		
	}

	private MapPoint findStartingLocation() {
		// set random starting location
		MapPoint location = new MapPoint(m_Random.nextInt(m_WorldSize), m_Random.nextInt(m_WorldSize));
		while (getCell(location).isBlocked() 
				|| getCell(location).isEnergyRecharger() 
				|| getCell(location).isHealthRecharger() 
				|| getCell(location).hasContents() 
				|| (m_Missiles.checkHit(location, false) != null)) {
			location.x = m_Random.nextInt(m_WorldSize);
			location.y = m_Random.nextInt(m_WorldSize);				
		}
		
		return location;
	}
	
	public TankSoarCell getCell(MapPoint location) {
		return m_World[location.y][location.x];
	}
	
	public TankSoarCell getCell(int x, int y) {
		return m_World[y][x];
	}
	
	public TankSoarCell getCell(int x, int y, int direction) {
		switch (direction) {
		case WorldEntity.kNorthInt:
			return m_World[y-1][x];
		case WorldEntity.kEastInt:
			return m_World[y][x+1];
		case WorldEntity.kSouthInt:
			return m_World[y+1][x];
		case WorldEntity.kWestInt:
			return m_World[y][x-1];
		default:
			break;
		}
		return m_World[y][x];
	}
	
	public TankSoarCell getCell(MapPoint location, int direction) {
		return getCell(location.x, location.y, direction);
	}
	
	void createTank(Agent agent, String productions, String color) {
		createTank(agent, productions, color, null, null, -1, -1, -1);
	}

	void createTank(Agent agent, String productions, String color, MapPoint location) {
		createTank(agent, productions, color, location, null, -1, -1, -1);
	}

	void createTank(Agent agent, String productions, String color, MapPoint location, String facing, int energy, int health, int missiles) {
		if (location != null) {
			if (this.isInBounds(location)) {
				if (getCell(location).isBlocked()) {
					m_Logger.log("Initial location " + location + " is blocked, going random.");
					location = null;
				}
			} else {
				m_Logger.log("Initial location " + location + " is out of bounds, going random.");
				location = null;
			}
		}
		
		Tank tank = new Tank(agent, productions, color, location, facing, energy, health, missiles, this);

		if (location == null) {
			location = findStartingLocation();
		}
		
		tank.setLocation(location);
		
		getCell(location).setTank(tank);

		if (m_Tanks == null) {
			m_Tanks = new Tank[1];
			m_Tanks[0] = tank;
		} else {
			Tank[] original = m_Tanks;
			m_Tanks = new Tank[original.length + 1];
			for (int i = 0; i < original.length; ++i) {
				m_Tanks[i] = original[i];
			}
			m_Tanks[original.length] = tank;
		}

		// Update all Tank sensors
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].updateSensors(this);
		}
		
		// Write out input links
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].writeInputLink();
		}		
	}
	
	void destroyTank(Tank tank) {
		if (m_Tanks == null) {
			return;
		}
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (tank == m_Tanks[i]) {
				if (m_Tanks.length == 1) {
					m_Tanks = null;
				} else {
					Tank[] original = m_Tanks;
					m_Tanks = new Tank[original.length - 1];
					for (int j = 0; j < m_Tanks.length; ++j) {
						if (j < i) {
							m_Tanks[j] = original[j];
						} else {
							m_Tanks[j] = original[j+1];
						}
					}
				}
				getCell(tank.getLocation()).removeTank();
				if (m_Tanks == null) {
					break;
				}
			}
		}
	}
	
	public boolean getVictoryCondition() {
		return m_PrintedStats;
	}
	
	public int getWorldCount() {
		return m_Simulation.getWorldCount();
	}
	
	public void update() {
		// reset modified flags
		for (int y = 0; y < m_World.length; ++y) {
			for (int x = 0; x < m_World[y].length; ++x) {
				m_World[y][x].clearRedraw();
				m_World[y][x].clearModified();
				if (m_World[y][x].checkCollision()) {
					m_World[y][x].setCollision(false);
				}
			}
		}			
		
		// Check for goal state
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].getPoints() >= kWinningPoints) {
				// Goal acheived
				if (!m_PrintedStats) {
					m_Simulation.stopSimulation();
					m_PrintedStats = true;
					m_Logger.log(m_Tanks[i].getName() + " is the winning tank.");
					for (int j = 0; j < m_Tanks.length; ++j) {
						m_Logger.log(m_Tanks[j].getName() + ": " + m_Tanks[j].getPoints());
					}
				}
				return;
			}
		}

		// Sanity check, need tanks to make an update meaningful
		if (m_Tanks == null) {
			m_Logger.log("Update called with no tanks.");
			return;
		}
		
		// UPDATE ALGORITHM 2.0
		// Read Tank output links
		// Move all Missiles
		//   Spawn new Missiles in front of Tanks
		// Check for Missile-Wall collisions
		// For all Tanks that move, check for Tank-Tank, Tank-Wall collisions
		//   Cancel Tank moves that are not possible
		//     Assign penalties to colliding tanks
		// Check for Missile-Tank special collisions
		//   Tank and Missile swapping spaces
		//   Assign penalties to hit tanks
		// For all Tanks that move, move Tanks
		// Spawn missile packs
		// Check for Missile-Tank collisions
		//   Assign penalties to hit tanks
		// Respawn killed Tanks in safe squares
		// Update all Tank sensors
		// Write out input links
		
		// Read Tank output links
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].readOutputLink();
		}		

		// Move all Missiles
		m_Missiles.moveMissiles();
		
		//   Spawn new Missiles in front of Tanks
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].firedMissile()) {
				m_Missiles.fireMissile(m_Tanks[i]);
			}
		}
		
		// Check for Missile-Wall collisions
		m_Missiles.removeWallCollisions();
		
		// For all Tanks that move, check for Tank-Tank, Tank-Wall collisions
		//   Cancel Tank moves that are not possible
		//     Assign penalties to colliding tanks
		for (int i = 0; i < m_Tanks.length; ++i) {
			// Get the simple wall collisions out of the way
			if (m_Tanks[i].recentlyMoved()) {
				TankSoarCell destinationCell = getCell(m_Tanks[i].getLocation(), m_Tanks[i].lastMoveDirection());
				if (destinationCell.isWall()) {
					m_Tanks[i].collide();
				}
			}
		}

		if (m_Tanks.length > 1) {
			// Re-set colliding to false, a helper variable for collisions
			for (int i = 0; i < m_Tanks.length; ++i) {
				m_Tanks[i].setColliding(false);
			}
			
			// Cross-check:
			// If moving in to a square with a tank, check that tank for 
			// a move in the opposite direction
			for (int i = 0; i < m_Tanks.length; ++i) {
				if (!m_Tanks[i].isColliding() && m_Tanks[i].recentlyMoved()) {
					TankSoarCell destinationCell = getCell(m_Tanks[i].getLocation(), m_Tanks[i].lastMoveDirection());
					if (destinationCell.containsTank()) {
						Tank collidee = destinationCell.getTank();
						if (!collidee.isColliding() && collidee.recentlyMoved()) {
							m_RD.calculate(m_Tanks[i].lastMoveDirection());
							if (collidee.lastMoveDirection() == m_RD.backward) {
								m_Tanks[i].setColliding(true);
								m_Tanks[i].collide();
								collidee.setColliding(true);
								collidee.collide();
							}
						}
					}
				}
			}
	
			// Re-set colliding to false, a helper variable for collisions
			for (int i = 0; i < m_Tanks.length; ++i) {
				m_Tanks[i].setColliding(false);
			}
			
			// Meet check:
			// Compare my destination location to others', if any match, set both
			// to colliding
			for (int i = 0; i < m_Tanks.length; ++i) {
				if (m_Tanks[i].isColliding()) {
					continue;
				}
				MapPoint myDest = new MapPoint(m_Tanks[i].getLocation(), m_Tanks[i].lastMoveDirection());
				for (int j = i + 1; j < m_Tanks.length; ++j) {
					if (m_Tanks[j].isColliding()) {
						continue;
					}
					MapPoint theirDest = new MapPoint(m_Tanks[j].getLocation(), m_Tanks[j].lastMoveDirection());
					if (myDest.equals(theirDest)) {
						m_Tanks[i].setColliding(true);
						m_Tanks[i].collide();
						m_Tanks[j].setColliding(true);
						m_Tanks[j].collide();
					}
				}
			}
		}

		// Check for Missile-Tank special collisions
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].recentlyMoved()) {
				//   Tank and Missile swapping spaces
				Tank owner = m_Missiles.checkSpecialHit(m_Tanks[i].getLocation(), m_Tanks[i].lastMoveDirection());
				if (owner != null) {
					TankSoarCell cell = getCell(m_Tanks[i].getLocation());
					if (!m_Tanks[i].getShieldStatus()) {
						cell.setExplosion();
						owner.adjustPoints(kMissileHitAward);
						m_Tanks[i].adjustPoints(kMissileHitPenalty);
					}
					//   Assign penalties to hit tanks
					m_Tanks[i].hitBy(owner, cell);
				}
			}
		}

		// For all Tanks that move, move Tanks
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].recentlyMoved()) {
				if (!getCell(m_Tanks[i].getLocation()).removeTank()) {
					m_Logger.log("Warning: moving tank " + m_Tanks[i].getName() + " not at old location " + m_Tanks[i].getLocation());
				}
				MapPoint newLocation = new MapPoint(m_Tanks[i].getLocation(), m_Tanks[i].lastMoveDirection());
				m_Tanks[i].setLocation(newLocation);
				if (getCell(m_Tanks[i].getLocation()).containsMissilePack()) {
					pickUpMissiles(m_Tanks[i]);
				}
				getCell(m_Tanks[i].getLocation()).setTank(m_Tanks[i]);
			} else if (m_Tanks[i].recentlyRotated()) {
				getCell(m_Tanks[i].getLocation()).setRedraw();
			}
		}
		
		// Spawn missile packs
		if (m_NumMissilePacks < kMaxMissilePacks) {
			if (m_Random.nextFloat() < kMisslePackRespawn) {
				spawnMissilePack();
			}
		}
		
		// Check for Missile-Tank collisions
		for (int i = 0; i < m_Tanks.length; ++i) {
			Tank owner = m_Missiles.checkHit(m_Tanks[i].getLocation(), true);
			if (owner != null) {
				TankSoarCell cell = getCell(m_Tanks[i].getLocation());
				if (!m_Tanks[i].getShieldStatus()) {
					cell.setExplosion();
					owner.adjustPoints(kMissileHitAward);
					m_Tanks[i].adjustPoints(kMissileHitPenalty);
				}
				//   Assign penalties to hit tanks
				m_Tanks[i].hitBy(owner, cell);
			}
		}
		
		//  Respawn killed Tanks in safe squares
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].getHealth() <= 0) {
				getCell(m_Tanks[i].getLocation()).removeTank();
				MapPoint location = findStartingLocation();
				m_Tanks[i].setLocation(location);
				getCell(location).setTank(m_Tanks[i]);
				m_Tanks[i].reset();
			}
		}
		
		// Update all Tank sensors
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].updateSensors(this);
		}
		
		// Write out input links
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].writeInputLink();
		}		
	}
	
	public boolean noAgents() {
		return (m_Tanks == null);
	}
	
	public void shutdown() {
		while (m_Tanks != null) {
			m_Simulation.destroyTank(m_Tanks[0]);
		}
	}
	
	public int getBlockedByLocation(Tank tank) {
		int blocked = 0;
		MapPoint location = tank.getLocation();
		
		if (getCell(location, WorldEntity.kNorthInt).isBlocked()) {
			blocked |= WorldEntity.kNorthInt;
		}
		if (getCell(location, WorldEntity.kEastInt).isBlocked()) {
			blocked |= WorldEntity.kEastInt;
		}
		if (getCell(location, WorldEntity.kSouthInt).isBlocked()) {
			blocked |= WorldEntity.kSouthInt;
		}
		if (getCell(location, WorldEntity.kWestInt).isBlocked()) {
			blocked |= WorldEntity.kWestInt;
		}
		
		return blocked;
	}

	public int getIncomingByLocation(MapPoint location) {
		return m_Missiles.checkIncoming(location);
	}
	
	public int getSoundNear(Tank tank) {
		if ((m_Tanks == null) || (m_Tanks.length <= 1)) {
			return 0;
		}
		
		// if the closest tank is greater than 7 away, there is no
		// possibility of hearing anything
		if (getStinkyTankNear(tank).getLocation().getManhattanDistanceTo(tank.getLocation()) > kMaxSmellDistance) {
			return 0;
		}
		
		// Set all cells unexplored.
		for(int row = 0; row < m_WorldSize; ++row) {
			// String rowString = new String();
			for (int col = 0; col < m_WorldSize; ++col) {
				m_World[row][col].setExplored(false);
				m_World[row][col].setParent(null);
			}
		}
		
		LinkedList searchList = new LinkedList();
		searchList.addLast(tank.getLocation());
		m_Logger.log("Starting search at " + tank.getLocation());
		int relativeDirection = -1;
		
		while (searchList.size() > 0) {
			MapPoint location = (MapPoint)searchList.getFirst();
			searchList.removeFirst();
			TankSoarCell cell = getCell(location);
			cell.setExplored(true);

			// Explore cell.
			for (int i = 0; i < 4; ++i) {
				int direction = 1 << i;
				MapPoint newLocation = new MapPoint(location, direction);
				
				if (!isInBounds(newLocation)) {
					continue;
				}
				
				TankSoarCell newCell = getCell(newLocation);
				if (newCell.isExplored()) {
					continue;
				}
				newCell.setExplored(true);
				
				if (newCell.isWall()) {
					continue;
				}
							
				if (newCell.containsTank() && newCell.getTank().recentlyMovedOrRotated()) {
					
					m_Logger.log("Found recently moved tank at " + newLocation);	
					
					while(getCell(location).getParent() != null) {
						newLocation = location;
						location = getCell(location).getParent();
					}
					m_Logger.log("First cell on path is " + newLocation);	
					relativeDirection = location.directionTo(newLocation);
					break;
				}
				
				if (relativeDirection != -1) {
					break;
				}
				
				//m_Logger.log("Adding " + newLocation + " with parent " + location);				
				newCell.setParent(location);
				searchList.addLast(newLocation);
			}
			
			if (relativeDirection != -1) {
				break;
			}
		}
		
		m_Logger.log("Finished search.");
		
		if (relativeDirection == -1) {
			relativeDirection = 0;
		}
		return relativeDirection;
	}
	
	public Tank getStinkyTankNear(Tank tank) {
		if ((m_Tanks == null) || (m_Tanks.length <= 1)) {
			return null;
		}

		Tank closestTank = null;
		int closestDistance = m_MaxManhattanDistance + 1;
		
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (tank == m_Tanks[i]) {
				continue;
			}
			
			int distance = tank.getLocation().getManhattanDistanceTo(m_Tanks[i].getLocation());
			if (distance <= closestDistance) {
				closestDistance = distance;
				if (closestTank != null) {
					// More than one, pick one at random
					closestTank = m_Random.nextBoolean() ? closestTank : m_Tanks[i];
				} else {
					closestTank = m_Tanks[i];
				}
			}
		}
		
		return closestTank;
	}
	
	public int getMaxManhattanDistance() {
		return m_MaxManhattanDistance;
	}
}
