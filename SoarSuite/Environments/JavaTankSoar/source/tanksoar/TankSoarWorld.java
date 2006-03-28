package tanksoar;

import java.util.*;

import simulation.*;
import sml.*;
import utilities.*;

public class TankSoarWorld extends World implements WorldManager {
	private static final String kTagTankSoarWorld = "tanksoar-world";

	private static final String kTagCells = "cells";
	private static final String kParamWorldWidth = "world-width";
	private static final String kParamWorldHeight = "world-height";
	private static final String kParamType = "type";
	
	static final String kTypeWall = "wall";
	static final String kTypeEmpty = "empty";
	static final String kTypeEnergyRecharger = "energy";
	static final String kTypeHealthRecharger = "health";
	
	private static final int kWallPenalty = -100;
	private static final int kWinningPoints = 50;
	
	private static final int kMaxMissilePacks = 3;
	private static final double kMisslePackRespawn = 0.05;
	static final int kMissilePackSize = 7;
	private int m_NumMissilePacks = 0;
	
	Random m_Random = new Random();
	RelativeDirections m_RD = new RelativeDirections();
	int m_MaxManhattanDistance;
	private static final int kMaxSmellDistance = 7;

	private TankSoarSimulation m_Simulation;
	private TankSoarCell[][] m_World;
	private boolean m_PrintedStats;
	private Tank[] m_Tanks;
	private ArrayList m_Collisions;

	public class Missiles {
		LinkedList m_Flying;
		
		public Missiles() {
			reset();
		}
		
	   	public class Missile {
	   		private MapPoint m_CurrentLocation;
	   		private int m_FlightPhase; // 0, 1 == affects current location, 2 == affects current location + 1
	   		private int m_Direction;
	   		
	   		public Missile(MapPoint location, int direction) {
	   			m_CurrentLocation = location;
	   			m_Direction = direction;
	   			m_FlightPhase = 0;
	   		}
	   	}
	   	
	   	public void reset() {
			m_Flying = new LinkedList();
	   	}
	   	
	   	public void moveMissiles() {
			// Mark old cells as modified
			MapPoint[] missileLocations = m_Missiles.getLocations();
			if (missileLocations != null) {
				for (int i = 0; i < missileLocations.length; ++i) {
					getCell(missileLocations[i]).setModified();
				}
			}
			
	   		ListIterator iter = m_Flying.listIterator();
	   		while (iter.hasNext()) {
	   			Missile missile = (Missile)iter.next();
	   			m_RD.calculate(missile.m_Direction);
	   			MapPoint location = missile.m_CurrentLocation;
	   			location.travel(m_RD.forward);
	   			
	   			if (getCell(location).isWall()) {
	   				iter.remove();
	   				continue;
	   			}
	   			
	   			++missile.m_FlightPhase;
	   			missile.m_FlightPhase %= 3;
	   		}
	   	}

	   	public void fireMissile(MapPoint originatingLocation, int direction) {
   			m_RD.calculate(direction);
   			MapPoint location = new MapPoint(originatingLocation);
   			location.travel(m_RD.forward);
	   		m_Flying.addLast(new Missile(location, direction));
	   	}
	   	
	   	public int checkIncoming(MapPoint location) {
	   		int incoming = 0;
	   		ListIterator iter = m_Flying.listIterator();
	   		while (iter.hasNext()) {
	   			Missile missile = (Missile)iter.next();
	   			if (missile.m_CurrentLocation.x == location.x) {
	   				// We're in the same row
	   				if (missile.m_CurrentLocation.y < location.y) {
	   					// The missile is above us
	   					if (missile.m_Direction == WorldEntity.kSouthInt) {
	   						// and travelling toward us
	   						incoming |= WorldEntity.kNorthInt;
	   					}
	   				} else {
	   					// The missile is below us
	   					if (missile.m_Direction == WorldEntity.kNorthInt) {
	   						// and travelling toward us
	   						incoming |= WorldEntity.kSouthInt;
	   					}
	   				}
	   			}
	   			if (missile.m_CurrentLocation.y == location.y) {
	   				// We're in the same column
	   				if (missile.m_CurrentLocation.x < location.x) {
	   					// The missile is to our left
	   					if (missile.m_Direction == WorldEntity.kEastInt) {
	   						// and travelling toward us
	   						incoming |= WorldEntity.kWestInt;
	   					}
	   				} else {
	   					// The missile is to our right
	   					if (missile.m_Direction == WorldEntity.kWestInt) {
	   						// and travelling toward us
	   						incoming |= WorldEntity.kEastInt;
	   					}
	   				}
	   			}
	   		}
	   		return incoming;
	   	}
	   	
	   	public boolean checkHit(MapPoint location) {
	   		boolean hit = false;
	   		ListIterator iter = m_Flying.listIterator();
	   		while (iter.hasNext()) {
	   			Missile missile = (Missile)iter.next();
	   			if (location.equals(missile.m_CurrentLocation)) {
	   				hit = true;
	   				iter.remove();
	   			}
	   		}
	   		return hit;
	   	}
	   	
	   	public MapPoint[] getLocations() {
	   		if (m_Flying.size() == 0) {
	   			return null;
	   		}
	   		MapPoint[] locations = new MapPoint[m_Flying.size()];
	   		Missile[] missiles = (Missile[])m_Flying.toArray(new Missile[0]);
	   		for (int i = 0; i < locations.length; ++i) {
	   			locations[i] = missiles[i].m_CurrentLocation;
	   		}
	   		return locations;
	   	}
	}
	
	private Missiles m_Missiles = new Missiles();
	
	public MapPoint[] getMissileLocations() {
		return m_Missiles.getLocations();
	}
	
   	public TankSoarWorld(TankSoarSimulation simulation) {
		m_Simulation = simulation;
	}
	
	public boolean load(String mapFile) {
		m_PrintedStats = false;
		m_NumMissilePacks = 0;
		
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
			m_WorldWidth = cells.getAttributeIntThrows(kParamWorldWidth);
			m_WorldHeight = cells.getAttributeIntThrows(kParamWorldHeight);
			
			// Figure out maximum manhattan distance:
			// Maximum manhattan distance to another tank:
			// (Map width x 2) - 2
			// but we can subtract 2 from the width because of the outer wall:
			// ((Map width - 2) x 2) - 2
			m_MaxManhattanDistance = ((m_WorldWidth - 2) * 2) - 2;
			
			// Create map array
			m_World = new TankSoarCell[m_WorldHeight][m_WorldWidth];
			
			// generate world
			generateWorldFromXML(cells);

		} catch (Exception e) {
			m_Logger.log("Error loading map: " + e.getMessage());
			return false;
		}
		
		// Place rechargers
		getCell(findStartingLocation()).setHealth();
		getCell(findStartingLocation()).setEnergy();
		
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
		for(int row = 0; row < m_WorldHeight; ++row) {
			//String rowString = new String();
			for (int col = 0; col < m_WorldWidth; ++col) {
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
		updateTankInput();
	}

	private MapPoint findStartingLocation() {
		// set random starting location
		MapPoint location = new MapPoint(m_Random.nextInt(m_WorldWidth), m_Random.nextInt(m_WorldHeight));
		while (!getCell(location).isOpen() || getCell(location).hasContents()) {
			location.x = m_Random.nextInt(m_WorldWidth);
			location.y = m_Random.nextInt(m_WorldHeight);				
		}
		
		return location;
	}
	
	private void updateTankInput() {
		// Update tanks
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].update(this);
		}
		// Reset and ressurect dead tanks
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].getHealth() <= 0) {
				MapPoint location = findStartingLocation();
				m_Tanks[i].setLocation(location);
				// Put tank on map
				if (getCell(location).containsMissilePack()) {
					--m_NumMissilePacks;
				}
				getCell(location).setTank(m_Tanks[i]);
				m_Tanks[i].reset();
			}
		}		
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
		createTank(agent, productions, color, null, null);
	}

	void createTank(Agent agent, String productions, String color, MapPoint location) {
		createTank(agent, productions, color, location, null);
	}

	void createTank(Agent agent, String productions, String color, MapPoint location, String facing) {
		if ((location != null) && getCell(location).isBlocked()) {
			m_Logger.log("Initial location " + location + " is blocked, going random.");
			location = null;
		}
		
		if (location == null) {
			location = findStartingLocation();
		}
		
		Tank tank = new Tank(agent, productions, color, location, facing, this);
		if (getCell(location).containsMissilePack()) {
			--m_NumMissilePacks;
		}
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

		updateTankInput();
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
		// reset modified flags, skipping edges
		for (int y = 1; y < m_World.length - 1; ++y) {
			for (int x = 1; x < m_World[y].length - 1; ++x) {
				m_World[y][x].clearModified();
				if (m_World[y][x].checkCollision()) {
					m_World[y][x].setCollision(false);
				}
			}
		}			
		
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

		if (m_Tanks == null) {
			m_Logger.log("Update called with no tanks.");
			return;
		}
		
		moveTanks();
		updateMap();
		handleCollisions();	
		updateTankInput();
	}
	
	private void moveTanks() {
		for (int i = 0; i < m_Tanks.length; ++i) {
			Integer move = m_Tanks[i].getMove();
			if (move == null) {
				continue;
			}
			
			MapPoint oldLocation = m_Tanks[i].getLocation();
			MapPoint newLocation;
			if (move.intValue() == WorldEntity.kNorthInt) {
				newLocation = new MapPoint(oldLocation.x, oldLocation.y - 1);
				
			} else if (move.intValue() == WorldEntity.kEastInt) {
				newLocation = new MapPoint(oldLocation.x + 1, oldLocation.y);
				
			} else if (move.intValue() == WorldEntity.kSouthInt) {
				newLocation = new MapPoint(oldLocation.x, oldLocation.y + 1);
				
			} else if (move.intValue() == WorldEntity.kWestInt) {
				newLocation = new MapPoint(oldLocation.x - 1, oldLocation.y);
				
			} else {
				m_Logger.log("Invalid move direction: " + move);
				return;
			}
			
			if (isInBounds(newLocation) && !getCell(newLocation).isBlocked()) {
				if (!getCell(oldLocation).removeTank()) {
					m_Logger.log("Warning: moving tank " + m_Tanks[i].getName() + " not at old location " + oldLocation);
				}
				m_Tanks[i].setLocation(newLocation);
			} else {
				m_Tanks[i].adjustPoints(kWallPenalty);
			}
		}
	}
	
	private void updateMap() {
		// Move tanks
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (getCell(m_Tanks[i].getLocation()).containsMissilePack()) {
				pickUpMissiles(m_Tanks[i]);
			}
			getCell(m_Tanks[i].getLocation()).setTank(m_Tanks[i]);
		}
		
		// Move all current missiles
		m_Missiles.moveMissiles();
		
		// Fire new missiles
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].firedMissile()) {
				m_Missiles.fireMissile(m_Tanks[i].getLocation(), m_Tanks[i].getFacingInt());
			}
		}
		
		// Check for missile collisions
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Missiles.checkHit(m_Tanks[i].getLocation())) {
				m_Tanks[i].hit();
			}
		}
		
		// Spawn missile packs
		if (m_NumMissilePacks < kMaxMissilePacks) {
			if (m_Random.nextFloat() < kMisslePackRespawn) {
				spawnMissilePack();
			}
		}
	}

	private void handleCollisions() {
		// generate collision groups
		ArrayList currentCollision = null;
		
		for (int i = 0; i < m_Tanks.length; ++i) {
			for (int j = i+1; j < m_Tanks.length; ++j) {
				// only check tanks who aren't already colliding
				if (m_Tanks[i].isColliding()) {
					continue;
				}
				
				if (m_Tanks[i].getLocation().equals(m_Tanks[j].getLocation())) {
					
					// Create data structures
					if (m_Collisions == null) {
						m_Collisions = new ArrayList();
					}
					if (currentCollision == null) {
						currentCollision = new ArrayList();
						
						// Add first agent to current collision
						currentCollision.add(m_Tanks[i]);
						
						// Flipping collision flag unnecessary as first agent will not be traversed again

						// Flip collision flag for cell
						getCell(m_Tanks[i].getLocation()).setCollision(true);

						m_Logger.log("Starting collision group at " + m_Tanks[i].getLocation());
					}
					
					// Add second agent to current collision
					currentCollision.add(m_Tanks[j]);

					// Flip collision flag for second agent
					m_Tanks[j].setColliding(true);
				}
			}
			// add current collision to collisions
			if (currentCollision != null) {
				m_Collisions.add(currentCollision);
				currentCollision = null;
			}
		}
		
		// if there are not collisions, we're done
		if (m_Collisions == null) {
			return;
		}
		
		// process collision groups
		for (int group = 0; group < m_Collisions.size(); ++group) {
			// Retrieve collision group
			currentCollision = (ArrayList)m_Collisions.get(group);
			Tank[] collidees = (Tank[])currentCollision.toArray(new Tank[0]);
			
			m_Logger.log("Processing collision group " + group + " with " + collidees.length + " collidees.");
			
			// Move back to old locations, update map
			for (int i = 0; i < collidees.length; ++i) {
				collidees[i].collide();
				m_RD.calculate(collidees[i].lastMoveDirection());
				MapPoint p = collidees[i].getLocation();
				p.travel(m_RD.backward);
				collidees[i].setLocation(p);

				if (getCell(p).containsMissilePack()) {
					--m_NumMissilePacks;
				}

				getCell(p).setTank(collidees[i]);
			}
		}
		
		// clear collision groups
		m_Collisions = null;
		
		// clear colliding flags
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].setColliding(false);
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
	
	public int getBlockedByLocation(MapPoint location) {
		int blocked = 0;
		
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
		for(int row = 0; row < m_WorldHeight; ++row) {
			// String rowString = new String();
			for (int col = 0; col < m_WorldWidth; ++col) {
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
							
				if (newCell.containsTank() && newCell.getTank().recentlyMoved()) {
					
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
}
