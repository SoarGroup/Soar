package tanksoar;

import java.util.*;
import java.util.logging.*;

import simulation.*;
import sml.*;
import utilities.*;

public class TankSoarWorld extends World implements WorldManager {
	
	private static Logger logger = Logger.getLogger("tanksoar");
	
	private static final String kTagTankSoarWorld = "tanksoar-world";

	private static final String kTagCells = "cells";
	private static final String kParamWorldSize = "world-size";
	private static final String kParamType = "type";
	
	static final String kTypeWall = "wall";
	static final String kTypeEmpty = "empty";
	static final String kTypeEnergyRecharger = "energy";
	static final String kTypeHealthRecharger = "health";
	
	private static final int kMaxMissilePacks = 3;
	private static final double kMisslePackRespawn = 0.05;
	static final int kMissilePackSize = 7;
	private int m_NumMissilePacks = 0;
	
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
	   		logger.finest("Resetting missiles.");
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
	   		Tank owner = null;
	   		ListIterator iter = m_Flying.listIterator();
	   		while (iter.hasNext()) {
	   			Missile missile = (Missile)iter.next();
	   			if (location.equals(missile.getCurrentLocation())) {
	   				m_RD.calculate(tankMove);
	   				if (missile.getDirection() == m_RD.backward) {
	   					if (owner == null) {
	   						owner = missile.getOwner();
	   					}
	   					iter.remove();
	   				}
	   			}
	   		}
	   		return owner;
	   	}
	   	
	   	public Tank checkHit(MapPoint location, boolean remove) {
	   		Tank owner = null;
	   		ListIterator iter = m_Flying.listIterator();
	   		while (iter.hasNext()) {
	   			Missile missile = (Missile)iter.next();
	   			if (location.equals(missile.getCurrentLocation())) {
	   				if (owner == null) {
	   					owner = missile.getOwner();
	   				}
	   				if (remove) {
	   					iter.remove();
	   				} else {
	   					return owner;
	   				}
	   			}
	   			if (missile.getFlightPhase() == 2) {
		   			MapPoint missileLoc = new MapPoint(missile.getCurrentLocation());
		   			missileLoc.travel(missile.getDirection());
		   			if (location.equals(missileLoc)) {
		   				if (owner == null) {
		   					owner = missile.getOwner();
		   				}
		   				if (remove) {
		   					iter.remove();
		   				} else {
		   					return owner;
		   				}
		   			}
	   			}
	   		}
	   		return owner;
	   	}
	   	
	   	public Missile[] getMissiles() {
	   		if (m_Flying.size() == 0) {
	   			return null;
	   		}
	   		return (Missile[])m_Flying.toArray(new Missile[0]);
	   	}
	   	
	   	public void removeMissilesOwnedBy(Tank owner) {
	   		ListIterator iter = m_Flying.listIterator();
	   		while (iter.hasNext()) {
	   			Missile missile = (Missile)iter.next();
	   			if (missile.getOwner() == owner) {
	   				logger.finest("Removing missile at " + missile.getCurrentLocation().toString() + " owned by " + owner.getName() + " due to death");
	   				iter.remove();
	   			}
	   		}
	   	}
	}
	
	private Missiles m_Missiles = new Missiles();
	
	public Missile[] getMissiles() {
		return m_Missiles.getMissiles();
	}
	
   	public TankSoarWorld(TankSoarSimulation simulation) {
		m_Simulation = simulation;
	}
	
	public boolean load(String mapFile) {
		m_PrintedStats = false;
		m_NumMissilePacks = 0;
		TankSoarCell.s_EnergyChargerCreated = false;
		TankSoarCell.s_HealthChargerCreated = false;

		logger.info("Loading map: " + mapFile);
		
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
			assert false;
			logger.warning("Error loading map: " + e.getMessage());
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
		return true;
	}
	
	private void spawnMissilePack() {
		TankSoarCell cell;
		MapPoint p;
		do {
			p = findStartingLocation();
			cell = getCell(p);
		} while (cell.hasContents() || !cell.isOpen());
		logger.info("Spawning missile pack at " + p);
		cell.setMissilePack();
		++m_NumMissilePacks;
	}
	
	void pickUpMissiles(Tank tank) {
		tank.addMissiles(TankSoarWorld.kMissilePackSize);
		--m_NumMissilePacks;
	}
	
	private void generateWorldFromXML(JavaElementXML cells) throws Exception {
		for(int row = 0; row < m_WorldSize; ++row) {
			String rowString = new String();
			for (int col = 0; col < m_WorldSize; ++col) {
				try {
					m_World[row][col] = new TankSoarCell(cells.getChild(row).getChild(col).getAttributeThrows(kParamType));
					rowString += m_World[row][col];
				} catch (Exception e) {
					throw new Exception("Error (generateWorldFromXML) on row: " + row + ", column: " + col);
				}
			}
			logger.finest(rowString);
		}
	}
	
	public Tank[] getTanks() {
		return m_Tanks;
	}
	
	public void destroyEntity(WorldEntity entity) {
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].getName() == entity.getName()) {
				destroyTank(m_Tanks[i]);
				m_Missiles.removeMissilesOwnedBy(m_Tanks[i]);
				return;
			}
		}
		logger.warning("Couldn't find entity name match for " + entity.getName() + ", ignoring.");
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
				logger.warning(m_Tanks[i].getName() + ": Initial location " + location + " is blocked, going random.");
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
		MapPoint location = new MapPoint(Simulation.random.nextInt(m_WorldSize), Simulation.random.nextInt(m_WorldSize));
		while (getCell(location).isBlocked() 
				|| getCell(location).isEnergyRecharger() 
				|| getCell(location).isHealthRecharger() 
				|| getCell(location).hasContents() 
				|| (m_Missiles.checkHit(location, false) != null)) {
			location.x = Simulation.random.nextInt(m_WorldSize);
			location.y = Simulation.random.nextInt(m_WorldSize);				
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
		String name = new String();
		if (agent == null) {
			name = productions;
		} else {
			name = agent.GetAgentName();
		}
		
		if (location != null) {
			if (this.isInBounds(location)) {
				if (getCell(location).isBlocked()) {
					logger.warning(name + ": Initial location " + location + " is blocked, going random.");
					location = null;
				}
			} else {
				logger.warning(name + ": Initial location " + location + " is out of bounds, going random.");
				location = null;
			}
		}
		
		Tank tank = new Tank(agent, productions, color, location, facing, energy, health, missiles, this);

		if (location == null) {
			location = findStartingLocation();
		}
		
		tank.setLocation(location);
		
		getCell(location).setTank(tank);

		logger.info(tank.getName() + ": Spawning at " + location.toString());
		
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
			logger.finer("Asked to destroy a tank when none exist.");
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
		logger.finest("Resetting modified flags on map.");
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
		int[] scores = null;
		boolean draw = false;
		int highScore = (m_Tanks.length > 0) ? m_Tanks[0].getPoints() : 0;
		
		if (m_Tanks.length > 1) {
			scores = new int[m_Tanks.length];
			
			for (int i = 0; i < m_Tanks.length; ++i) {
				scores[i] = m_Tanks[i].getPoints();
			}
			Arrays.sort(scores);
			highScore = scores[scores.length - 1];
			if (scores[scores.length - 1] ==  scores[scores.length - 2]) {
				draw = true;
			}
			
			if (scores[scores.length - 1] >= m_Simulation.getWinningScore()) {
				// We have a winner (or a draw)
				if (draw) {
					logger.finer("Draw detected.");
				}
				
				if (!m_PrintedStats) {
					m_Simulation.notificationMessage("At least one tank has achieved at least " + Integer.toString(m_Simulation.getWinningScore()) + " points.");
					m_Simulation.stopSimulation();
					m_PrintedStats = true;
					for (int j = 0; j < m_Tanks.length; ++j) {
						String status = null;
						if (m_Tanks[j].getPoints() == highScore) {
							status = draw ? "draw" : "winner";
						} else {
							status = "loser";
						}
						logger.info(m_Tanks[j].getName() + ": " + m_Tanks[j].getPoints() + " (" + status + ").");
					}
				}
				return;
			}
		}
		
		// Check for max updates
		if (m_Simulation.reachedMaxUpdates()) {
			if (draw) {
				logger.finer("Draw detected.");
			}

			if (!m_PrintedStats) {
				m_Simulation.notificationMessage("Reached maximum updates, stopping.");
				m_Simulation.stopSimulation();
				m_PrintedStats = true;
				for (int j = 0; j < m_Tanks.length; ++j) {
					String status = null;
					if (m_Tanks[j].getPoints() == highScore) {
						status = draw ? "draw" : "winner";
					} else {
						status = "loser";
					}
					logger.info(m_Tanks[j].getName() + ": " + m_Tanks[j].getPoints() + " (" + status + ").");
				}
			}
			return;
		}
		
		// Sanity check, need tanks to make an update meaningful
		if (m_Tanks == null) {
			logger.warning("Update called with no tanks.");
			return;
		}
		
		// UPDATE ALGORITHM 2.0
		// Read Tank output links
		// Move all Missiles
		// BUGBUG: Moving this!:  Spawn new Missiles in front of Tanks
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
			if (m_Tanks[i].getAgent() != null) {
				m_Tanks[i].readOutputLink();
			} else {
				m_Simulation.readHumanInput();
				m_Tanks[i].humanInput(m_Simulation.getHumanInput());
			}
		}		

		// Move all Missiles
		m_Missiles.moveMissiles();
		
		// For all Tanks that move, check for Tank-Tank, Tank-Wall collisions
		//   Cancel Tank moves that are not possible
		//     Assign penalties to colliding tanks
		for (int i = 0; i < m_Tanks.length; ++i) {
			// Get the simple wall collisions out of the way
			if (m_Tanks[i].recentlyMoved()) {
				TankSoarCell destinationCell = getCell(m_Tanks[i].getLocation(), m_Tanks[i].lastMoveDirection());
				if (destinationCell.isWall()) {
					logger.fine(m_Tanks[i].getName() + ": hit a wall.");
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
								logger.fine(m_Tanks[i].getName() + ": cross-collided with " + collidee.getName());
								m_Tanks[i].setColliding(true);
								collidee.setColliding(true);
							}
						}
					}
				}
			}
	
			// Apply collisions and re-set colliding to false, a helper variable for collisions
			for (int i = 0; i < m_Tanks.length; ++i) {
				if (m_Tanks[i].isColliding()) {
					m_Tanks[i].collide();
				}
				m_Tanks[i].setColliding(false);
			}
			
			// Meet check:
			// Compare my destination location to others', if any match, set both
			// to colliding
			for (int i = 0; i < m_Tanks.length; ++i) {
				if (m_Tanks[i].isColliding() || !m_Tanks[i].recentlyMoved()) {
					continue;
				}
				MapPoint myDest = new MapPoint(m_Tanks[i].getLocation(), m_Tanks[i].lastMoveDirection());
				for (int j = i + 1; j < m_Tanks.length; ++j) {
					if (!m_Tanks[j].recentlyMoved()) {
						continue;
					}
					MapPoint theirDest = new MapPoint(m_Tanks[j].getLocation(), m_Tanks[j].lastMoveDirection());
					if (myDest.equals(theirDest)) {
						logger.fine(m_Tanks[i].getName() + ": meet-collided with " + m_Tanks[j].getName());
						// FIXME: Both should collide, but that causes a SNC!
						m_Tanks[i].setColliding(true);
						logger.warning(m_Tanks[j].getName() + ": should be penalized for a collision but isn't because of bug 779.");
						//m_Tanks[j].setColliding(true);
						break;
					}
				}
			}
		}

		// Apply collisions and re-set colliding to false, a helper variable for collisions
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].isColliding()) {
				m_Tanks[i].collide();
			}
			m_Tanks[i].setColliding(false);
		}
		
		// Check for Missile-Tank special collisions
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].recentlyMoved()) {
				//   Tank and Missile swapping spaces
				Tank owner = m_Missiles.checkSpecialHit(m_Tanks[i].getLocation(), m_Tanks[i].lastMoveDirection());
				if (owner != null) {
					logger.fine(m_Tanks[i].getName() + ": moved through a missile.");
					TankSoarCell cell = getCell(m_Tanks[i].getLocation());
					//   Assign penalties to hit tanks
					m_Tanks[i].hitBy(owner, cell);
					if (!m_Tanks[i].getShieldStatus()) {
						cell.setExplosion();
					}
				}
			}
		}

		// For all Tanks that move, move Tanks
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].recentlyMoved()) {
				if (!getCell(m_Tanks[i].getLocation()).removeTank()) {
					assert false;
					logger.warning("Moving tank " + m_Tanks[i].getName() + " not at old location " + m_Tanks[i].getLocation());
				}
				MapPoint newLocation = new MapPoint(m_Tanks[i].getLocation(), m_Tanks[i].lastMoveDirection());
				m_Tanks[i].setLocation(newLocation);
				if (getCell(m_Tanks[i].getLocation()).containsMissilePack()) {
					pickUpMissiles(m_Tanks[i]);
				}
				getCell(m_Tanks[i].getLocation()).setTank(m_Tanks[i]);
			} else {
				// REDRAW no matter what.
				getCell(m_Tanks[i].getLocation()).setRedraw();
			}
		}
		
		//   Spawn new Missiles in front of Tanks
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].firedMissile()) {
				m_Missiles.fireMissile(m_Tanks[i]);
			}
		}
		
		// Check for Missile-Wall collisions
		m_Missiles.removeWallCollisions();
		
		// Spawn missile packs
		if (m_NumMissilePacks < kMaxMissilePacks) {
			if (Simulation.random.nextFloat() < kMisslePackRespawn) {
				spawnMissilePack();
			}
		}
		
		// Check for Missile-Tank collisions
		for (int i = 0; i < m_Tanks.length; ++i) {
			Tank owner = m_Missiles.checkHit(m_Tanks[i].getLocation(), true);
			if (owner != null) {
				TankSoarCell cell = getCell(m_Tanks[i].getLocation());
				//   Assign penalties to hit tanks
				m_Tanks[i].hitBy(owner, cell);
				if (!m_Tanks[i].getShieldStatus()) {
					cell.setExplosion();
				}
			}
		}
		
		//  Respawn killed Tanks in safe squares
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].getHealth() <= 0) {
				getCell(m_Tanks[i].getLocation()).removeTank();
				m_Missiles.removeMissilesOwnedBy(m_Tanks[i]);
				MapPoint location = findStartingLocation();
				logger.info(m_Tanks[i].getName() + ": Spawning at " + location.toString());
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
		//logger.finest("Starting search at " + tank.getLocation());
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
					
					//logger.finest("Found recently moved tank at " + newLocation);	
					
					int distance = 1;
					while(getCell(location).getParent() != null) {
						++distance;
						newLocation = location;
						location = getCell(location).getParent();
					}
					//logger.finest("Distance to loud tank is " + Integer.toString(distance));	
					//logger.finest("First cell on path is " + newLocation);	
					if (distance  <= kMaxSmellDistance) {
						relativeDirection = location.directionTo(newLocation);
					}
					break;
				}
				
				if (relativeDirection != -1) {
					break;
				}
				
				//logger.finest("Adding " + newLocation + " with parent " + location);				
				newCell.setParent(location);
				searchList.addLast(newLocation);
			}
			
			if (relativeDirection != -1) {
				break;
			}
		}
		
		//logger.finest("Finished search.");
		
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
		
		logger.finest("Sniffing for stinky tank...");
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (tank == m_Tanks[i]) {
				continue;
			}
			
			int distance = tank.getLocation().getManhattanDistanceTo(m_Tanks[i].getLocation());
			//logger.finest(tank.getColor() + tank.getLocation() + " is " + distance + " from " + m_Tanks[i].getColor() + m_Tanks[i].getLocation());
			if (distance < closestDistance) {
				closestDistance = distance;
				closestTank = m_Tanks[i];
			} else if (distance == closestDistance) {
				if (closestTank != null) {
					// More than one, pick one at random
					closestTank = Simulation.random.nextBoolean() ? closestTank : m_Tanks[i];
					logger.finest("Picked " + closestTank.getName() + " randomly.");
				}
			}
		}
		logger.finest(closestTank.getName() + " is closest.");
		return closestTank;
	}
	
	public int getMaxManhattanDistance() {
		return m_MaxManhattanDistance;
	}
}
