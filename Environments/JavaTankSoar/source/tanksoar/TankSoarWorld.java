package tanksoar;

import java.util.*;
import java.util.logging.*;

import simulation.*;
import sml.*;
import utilities.*;

public class TankSoarWorld extends World implements WorldManager {
	
	private static Logger logger = Logger.getLogger("simulation");
	
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
	
	int m_MaxManhattanDistance;
	static final int kMaxSmellDistance = 7;

	private TankSoarSimulation m_Simulation;
	private TankSoarCell[][] m_World;
	private boolean m_PrintedStats;
	private Tank[] m_Tanks;
	private boolean quiet;
	
	private LinkedList missiles = new LinkedList();
	
   	public TankSoarWorld(TankSoarSimulation simulation, boolean quiet) {
		m_Simulation = simulation;
		this.quiet = quiet;
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
		missiles.clear();
		resetTanks();
		return true;
	}
	
	private void spawnMissilePack() {
		TankSoarCell cell;
		java.awt.Point p;
		do {
			p = findStartingLocation();
			cell = getCell(p);
		} while (cell.hasContents() || !cell.isOpen());
		logger.info("Spawning missile pack at (" + p.x + "," + p.y + ")");
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
		if (entity == null) {
			logger.warning("Asked to destroy null entity.");
			return;
		}
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].getName().equals(entity.getName())) {
				removeMissilesOwnedBy(m_Tanks[i]);
				destroyTank(m_Tanks[i]);
				return;
			}
		}
		logger.warning("Couldn't find entity name match for " + entity.getName() + ", ignoring.");
	}
	
	public WorldEntity[] getEntities() {
		return getTanks();
	}
	
	public void setStopping(boolean status) {
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].setStopping(status);
		}
	}
	
	private void resetTanks() {
		if (m_Tanks == null) {
			return;
		}
		for (int i = 0; i < m_Tanks.length; ++i) {
			java.awt.Point location = m_Tanks[i].getInitialLocation();
			if (location != null && getCell(location).isBlocked()) {
				logger.warning(m_Tanks[i].getName() + ": Initial location (" + location.x + "," + location.y + ") is blocked, going random.");
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

	private java.awt.Point findStartingLocation() {
		// set random starting location
		java.awt.Point location = new java.awt.Point(Simulation.random.nextInt(m_WorldSize), Simulation.random.nextInt(m_WorldSize));
		while (getCell(location).isBlocked() 
				|| getCell(location).isEnergyRecharger() 
				|| getCell(location).isHealthRecharger() 
				|| getCell(location).hasContents()
				|| (checkForMissileThreat(location) != null)) {
			location.x = Simulation.random.nextInt(m_WorldSize);
			location.y = Simulation.random.nextInt(m_WorldSize);				
		}
		
		return location;
	}
	
	public TankSoarCell getCell(java.awt.Point location) {
		return m_World[location.y][location.x];
	}
	
	public TankSoarCell getCell(int x, int y) {
		return m_World[y][x];
	}
	
	void createTank(Agent agent, String productions, String color) {
		createTank(agent, productions, color, null, null, -1, -1, -1);
	}

	void createTank(Agent agent, String productions, String color, java.awt.Point location) {
		createTank(agent, productions, color, location, null, -1, -1, -1);
	}

	void createTank(Agent agent, String productions, String color, java.awt.Point locationIn, String facing, int energy, int health, int missiles) {
		String name;
		if (agent == null) {
			name = new String(productions);
		} else {
			name = new String(agent.GetAgentName());
		}
		
		java.awt.Point location = null;
		if (locationIn != null) {
			if (this.isInBounds(location)) {
				if (getCell(location).isBlocked()) {
					logger.warning(name + ": Initial location " + location + " is blocked, going random.");
				} else {
					location = new java.awt.Point(locationIn);
				}
			} else {
				logger.warning(name + ": Initial location " + location + " is out of bounds, going random.");
			}
		}
		
		Tank tank = new Tank(agent, productions, color, location, facing, energy, health, missiles, this);

		if (location == null) {
			location = findStartingLocation();
		}
		
		tank.setLocation(location);
		getCell(location).setTank(tank);

		logger.info(tank.getName() + ": Spawning at (" + location.x + "," + location.y + ")");
		
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
		if (tank == null) {
			logger.warning("Asked to destroy null tank.");
			return;
		}
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (tank.equals(m_Tanks[i])) {
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
				getCell(tank.getLocation()).removeTank(tank);
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
	
	private void printEndingStats(int[] scores) {
		if (m_PrintedStats) {
			return;
		}
		m_PrintedStats = true;
		
		boolean draw = false;
		if (scores.length > 1) {
			if (scores[scores.length - 1] ==  scores[scores.length - 2]) {
				logger.finer("Draw detected.");
				draw = true;
			}
		}
		
		for (int j = 0; j < m_Tanks.length; ++j) {
			String status = null;
			if (m_Tanks[j].getPoints() == scores[scores.length - 1]) {
				status = draw ? "draw" : "winner";
			} else {
				status = "loser";
			}
			logger.info(m_Tanks[j].getName() + ": " + m_Tanks[j].getPoints() + " (" + status + ").");
		}
	}
	
	private int[] getSortedScores() {
		int[] scores = new int[m_Tanks.length];
		for (int i = 0; i < m_Tanks.length; ++i) {
			scores[i] = m_Tanks[i].getPoints();
		}
		Arrays.sort(scores);
		return scores;
	}
	
	public void handleSNC(Tank deadTank) {
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (!deadTank.equals(m_Tanks[i])) {
				if (deadTank.getPoints() >= m_Tanks[i].getPoints()) {
					deadTank.adjustPoints((m_Tanks[i].getPoints() - deadTank.getPoints()) - 1, "SNC penalty");
				}
			}
		}
		m_Simulation.stopSimulation();
	}
	
	public void update() {
		if (!quiet) {
			logger.finest("Resetting modified flags on map.");
			for (int y = 0; y < m_World.length; ++y) {
				for (int x = 0; x < m_World[y].length; ++x) {
					m_World[y][x].clearRedraw();
					m_World[y][x].clearModified();
					// UNUSED in tanksoar
					//if (m_World[y][x].checkCollision()) {
					//	m_World[y][x].setCollision(false);
					//}
				}
			}			
		}

		// Sanity check, need tanks to make an update meaningful
		if (m_Tanks == null || m_Tanks.length == 0) {
			logger.warning("Update called with no tanks.");
			return;
		}
		
		// Check for goal state
		int[] scores = getSortedScores();
		if (scores[scores.length - 1] >= m_Simulation.getWinningScore()) {
			// We have a winner (or a draw)
			m_Simulation.notificationMessage("At least one tank has achieved at least " + Integer.toString(m_Simulation.getWinningScore()) + " points.");
			m_Simulation.stopSimulation();
			printEndingStats(scores);
			return;
		}
		if (m_Simulation.reachedMaxUpdates()) {
			// We have a winner (or a draw)
			m_Simulation.notificationMessage("Reached maximum updates, stopping.");
			m_Simulation.stopSimulation();
			printEndingStats(scores);
			return;
		}
		
		// Read Tank output links
		logger.finest("Gettng tank input");
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].getAgent() != null) {
				m_Tanks[i].readOutputLink();
			} else {
				m_Simulation.readHumanInput();
				m_Tanks[i].humanInput(m_Simulation.getHumanInput());
			}
		}		

		// For all Tanks that move, check for Tank-Tank, Tank-Wall collisions
		//   Cancel Tank moves that are not possible
		//     Assign penalties to colliding tanks
		logger.finest("Checking for tank-tank, tank-wall collisions");
		for (int i = 0; i < m_Tanks.length; ++i) {
			// Get the simple wall collisions out of the way
			if (m_Tanks[i].recentlyMoved()) {
				int xTemp = m_Tanks[i].getLocation().x;
				int yTemp = m_Tanks[i].getLocation().y;
				xTemp += Direction.xDelta[m_Tanks[i].getLastMoveDirection()];
				yTemp += Direction.yDelta[m_Tanks[i].getLastMoveDirection()];
				TankSoarCell destinationCell = getCell(xTemp, yTemp);
				
				// Collide with walls OR non-moving tanks!
				if (destinationCell.isWall()) {
					logger.fine(m_Tanks[i].getName() + ": hit a wall.");
					m_Tanks[i].collide();
				} else if (destinationCell.containsTank()) {
					Tank collidee = destinationCell.getTank();
					if (collidee.recentlyMoved() == false) {
						logger.fine(m_Tanks[i].getName() + ": hit a tank.");
						m_Tanks[i].collide();
						collidee.collide();
					}
				}
			}
		}

		if (m_Tanks.length > 1) {
			// Re-set colliding to false, a helper variable for collisions
			for (int i = 0; i < m_Tanks.length; ++i) {
				m_Tanks[i].setColliding(false);
			}
			
			logger.finest("Doing cross-check");
			// Cross-check:
			// If moving in to a square with a tank, check that tank for 
			// a move in the opposite direction
			for (int i = 0; i < m_Tanks.length; ++i) {
				if (!m_Tanks[i].isColliding() && m_Tanks[i].recentlyMoved()) {
					int xTemp = m_Tanks[i].getLocation().x;
					int yTemp = m_Tanks[i].getLocation().y;
					xTemp += Direction.xDelta[m_Tanks[i].getLastMoveDirection()];
					yTemp += Direction.yDelta[m_Tanks[i].getLastMoveDirection()];
					TankSoarCell destinationCell = getCell(xTemp, yTemp);
					
					if (destinationCell.containsTank()) {
						Tank collidee = destinationCell.getTank();
						if (!collidee.isColliding() && collidee.recentlyMoved()) {
							if (collidee.getLastMoveDirection() == Direction.backwardOf[m_Tanks[i].getLastMoveDirection()]) {
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
			logger.finest("Doing meet-check");
			for (int i = 0; i < m_Tanks.length; ++i) {
				if (m_Tanks[i].isColliding() || !m_Tanks[i].recentlyMoved()) {
					continue;
				}
				java.awt.Point myDest = new java.awt.Point(m_Tanks[i].getLocation());
				myDest = Direction.translate(myDest, m_Tanks[i].getLastMoveDirection());
				for (int j = i + 1; j < m_Tanks.length; ++j) {
					if (!m_Tanks[j].recentlyMoved()) {
						continue;
					}
					java.awt.Point theirDest = new java.awt.Point(m_Tanks[j].getLocation());
					theirDest = Direction.translate(theirDest, m_Tanks[j].getLastMoveDirection());
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
		} else {
			logger.finest("Cross and meet checks impossible with 1 tank");
		}

		// Apply collisions and re-set colliding to false, a helper variable for collisions
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].isColliding()) {
				m_Tanks[i].collide();
			}
			m_Tanks[i].setColliding(false);
		}
		
		// For all Tanks that move, move Tanks
		logger.finest("Moving all tanks that move");
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].recentlyMoved()) {
				getCell(m_Tanks[i].getLocation()).removeTank(m_Tanks[i]);
				java.awt.Point newLocation = new java.awt.Point(m_Tanks[i].getLocation());
				newLocation = Direction.translate(newLocation, m_Tanks[i].getLastMoveDirection());
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
		
		// Move all Missiles
		if (missiles.size() > 0) {
			logger.finest("Moving all missiles");
			moveMissiles();
			
			// Check for Missile-Tank special collisions
			logger.finest("Checking for missile-tank special collisions");
			for (int i = 0; i < m_Tanks.length; ++i) {
				if (m_Tanks[i].recentlyMoved()) {
					//   Tank and Missile swapping spaces
					Integer[] ids = checkMissilePassThreat(m_Tanks[i]);
					if (ids != null) {
						logger.fine(m_Tanks[i].getName() + ": moved through " + Integer.toString(ids.length) + " missile(s).");
						//   Assign penalties and awards
						m_Tanks[i].hitBy(ids);
						removeMissilesByID(ids);
						if (!m_Tanks[i].getShieldStatus()) {
							getCell(m_Tanks[i].getLocation()).setExplosion();
						}
					}
				}
			}
		} else {
			logger.finest("Skipping missile movement, no missiles");
		}

		//   Spawn new Missiles in front of Tanks
		logger.finest("Spawning newly fired missiles");
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].firedMissile()) {
				addMissile(m_Tanks[i]);
			}
		}
		
		// Check for Missile-Tank collisions
		if (missiles.size() > 0) {
			logger.finest("Checking for missile-tank collisions");
			for (int i = 0; i < m_Tanks.length; ++i) {
				Integer[] ids = checkForMissileThreat(m_Tanks[i].getLocation());
				if (ids != null) {
					logger.fine(m_Tanks[i].getName() + ": hit by " + Integer.toString(ids.length) + " missile(s).");
					//   Assign penalties and awards
					m_Tanks[i].hitBy(ids);
					removeMissilesByID(ids);
					if (!m_Tanks[i].getShieldStatus()) {
						getCell(m_Tanks[i].getLocation()).setExplosion();
					}
				}
			}
		} else {
			logger.finest("Skipping missile-tank collision check, no missiles");
		}

		// Spawn missile packs
		logger.finest("Spawning missile packs");
		if (m_NumMissilePacks < kMaxMissilePacks) {
			if (Simulation.random.nextFloat() < kMisslePackRespawn) {
				spawnMissilePack();
			}
		}
		
		// Tick resource drain 1 energy until 0 energy left, then 1 health until dead
//		for (int i = 0; i < m_Tanks.length; ++i) {
//			m_Tanks[i].resourceDrain();
//		}

		//  Respawn killed Tanks in safe squares
		logger.finest("Respawning killed tanks");
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].getHealth() <= 0) {
				getCell(m_Tanks[i].getLocation()).removeTank(m_Tanks[i]);
				removeMissilesOwnedBy(m_Tanks[i]);
				java.awt.Point location = findStartingLocation();
				logger.info(m_Tanks[i].getName() + ": spawning at (" + location.x + "," + location.y + ")");
				m_Tanks[i].setLocation(location);
				getCell(location).setTank(m_Tanks[i]);
				m_Tanks[i].reset();
			}
		}
		
		// Update all Tank sensors
		logger.finest("Updating tank sensors");
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].updateSensors(this);
		}
		
		// Write out input links
		logger.finest("Writing output links");
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].writeInputLink();
		}		
	}
	
	public boolean noAgents() {
		return (m_Tanks == null);
	}
	
	public void shutdown() {
		if (m_Tanks != null && m_Tanks.length > 0) {
			int[] scores = getSortedScores();
			printEndingStats(scores);
		}
		while (m_Tanks != null) {
			m_Simulation.destroyTank(m_Tanks[0]);
		}
	}
	
	public int getBlockedByLocation(Tank tank) {
		int blocked = 0;
		java.awt.Point location = tank.getLocation();
		
		if (getCell(location.x + Direction.xDelta[Direction.kNorthInt], location.y + Direction.yDelta[Direction.kNorthInt]).isBlocked()) {
			blocked |= Direction.kNorthIndicator;
		}
		if (getCell(location.x + Direction.xDelta[Direction.kEastInt], location.y + Direction.yDelta[Direction.kEastInt]).isBlocked()) {
			blocked |= Direction.kEastIndicator;
		}
		if (getCell(location.x + Direction.xDelta[Direction.kSouthInt], location.y + Direction.yDelta[Direction.kSouthInt]).isBlocked()) {
			blocked |= Direction.kSouthIndicator;
		}
		if (getCell(location.x + Direction.xDelta[Direction.kWestInt], location.y + Direction.yDelta[Direction.kWestInt]).isBlocked()) {
			blocked |= Direction.kWestIndicator;
		}

		return blocked;
	}

	public int getIncomingByLocation(java.awt.Point location) {
		return checkMissileIncoming(location);
	}
	
	public int getSoundNear(Tank tank) {
		if ((m_Tanks == null) || (m_Tanks.length <= 1)) {
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
		searchList.addLast(new java.awt.Point(tank.getLocation()));
		//logger.finest("Starting search at " + tank.getLocation());
		int relativeDirection = -1;
		
		int x = 0;
		int y = 0;
		int distance;
		java.awt.Point location;
		while (searchList.size() > 0) {
			location = (java.awt.Point)searchList.getFirst();
			searchList.removeFirst();
			getCell(location).setExplored(true);

			// Explore cell.
			for (int i = 1; i < 5; ++i) {
				x = location.x;
				y = location.y;
				x += Direction.xDelta[i];
				y += Direction.yDelta[i];
				//java.awt.Point newLocation = new java.awt.Point(location);
				//newLocation = Direction.translate(newLocation, i);

				if (!isInBounds(x, y)) {
					continue;
				}
				//if (!isInBounds(newLocation)) {
				//	continue;
				//}
				
				//TankSoarCell newCell = getCell(newLocation);
				TankSoarCell newCell = getCell(x, y);
				if (newCell.isExplored()) {
					continue;
				}
				newCell.setExplored(true);
				
				if (newCell.isWall()) {
					continue;
				}
							
				Tank targetTank = newCell.getTank();
				if ((targetTank != null) && targetTank.recentlyMovedOrRotated()) {
				//if (newCell.containsTank() && newCell.getTank().recentlyMovedOrRotated()) {
					
					//logger.finest("Found recently moved tank at " + newLocation);	
					
					distance = 1;
					while(getCell(location).getParent() != null) {
						++distance;
						x = location.x;
						y = location.y;
						location = getCell(location).getParent();
					}
					//logger.finest("Distance to loud tank is " + Integer.toString(distance));	
					//logger.finest("First cell on path is " + newLocation);	
					if (distance  <= kMaxSmellDistance) {
						if (x < location.x) {
							relativeDirection = Direction.kWestInt;
						} else if (x > location.x) {
							relativeDirection = Direction.kEastInt;
						} else if (y < location.y) {
							relativeDirection = Direction.kNorthInt;
						} else if (y > location.y) {
							relativeDirection = Direction.kSouthInt;
						} else {
							assert false;
							relativeDirection = 0;
						}
					}
					break;
				}
				
				if (relativeDirection != -1) {
					break;
				}
				
				//logger.finest("Adding " + newLocation + " with parent " + location);				
				newCell.setParent(location);
				searchList.addLast(new java.awt.Point(x, y));
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
	
	public int getManhattanDistanceTo(java.awt.Point a, java.awt.Point b) {
		return Math.abs(a.x - b.x) + Math.abs(a.y - b.y);
	}

	
	public Tank getStinkyTankNear(Tank tank) {
		if ((m_Tanks == null) || (tank == null) || (m_Tanks.length <= 1)) {
			return null;
		}

		Tank closestTank = null;
		int closestDistance = m_MaxManhattanDistance + 1;
		
		logger.finest("Sniffing for stinky tank...");
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (tank.equals(m_Tanks[i])) {
				continue;
			}
			
			int distance = getManhattanDistanceTo(tank.getLocation(), m_Tanks[i].getLocation());
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
	
	private int removeMissilesOwnedBy(Tank owner) {
		int count = 0;
		ListIterator iter = missiles.listIterator();
		while (iter.hasNext()) {
			Missile missile = (Missile)iter.next();
			if (missile.getOwner().equals(owner)) {
				logger.finer("Removing missile id " + Integer.toString(missile.getID()) + " owned by " + owner.getName());
				++count;
				iter.remove();
			}
		}
		return count;
	}
	
	private void removeMissilesByID(Integer[] ids) {
		assert ids != null;
		for (int i = 0; i < ids.length; ++i) {
			removeMissileByID(ids[i].intValue());
		}
	}
	
	private void removeMissileByID(int id) {
		ListIterator iter = missiles.listIterator();
		while (iter.hasNext()) {
			Missile missile = (Missile)iter.next();
			if (missile.getID() == id) {
				logger.finer("Removing missile by id " + Integer.toString(id));
				iter.remove();
				return;
			}
		}
		assert false;
	}
	
	Missile getMissileByID(int id) {
		ListIterator iter = missiles.listIterator();
		while (iter.hasNext()) {
			Missile missile = (Missile)iter.next();
			if (missile.getID() == id) {
				return missile;
			}
		}
		assert false;
		return null;
	}
	
   	private Integer[] checkForMissileThreat(java.awt.Point location) {
   		ArrayList ids = new ArrayList();
   		
   		ListIterator iter = missiles.listIterator();
   		while (iter.hasNext()) {
   			Missile missile = (Missile)iter.next();
   			java.awt.Point[] threats = missile.getThreatenedLocations();
   			for (int i = 0; i < threats.length; ++i) {
   				if (location.equals(threats[i])) {
   					logger.finer("Missile id " + Integer.toString(missile.getID()) + " threat detected for " + location.toString());
   					ids.add(new Integer(missile.getID()));
   				}
   			}
   		}
   		if (ids.size() > 0) {
   			return (Integer[])ids.toArray(new Integer[0]);
   		}
   		return null;
   	}
   	
   	private void moveMissiles() {
   		ListIterator iter = missiles.listIterator();
   		while (iter.hasNext()) {
   			Missile missile = (Missile)iter.next();
   			
  			getCell(missile.getLocation()).setRedraw();
   			if (missile.getFlightPhase() == 2) {
   				java.awt.Point newLocation = new java.awt.Point(missile.getLocation()); 
   				newLocation = Direction.translate(newLocation, missile.getDirection());
   				getCell(newLocation).setRedraw();
   			}

   			java.awt.Point[] threats = missile.getThreatenedLocations();
   			if (threats.length == 2) {
   				if (getCell(threats[1]).isWall()) {
   					logger.finer("Removing missile id " + Integer.toString(missile.getID()) + " on move due to wall impact (flight phase 3)");
   					iter.remove();
   					continue;
   				}
   			}

   			missile.move();
   			
   			if (getCell(missile.getLocation()).isWall()) {
				logger.finer("Removing missile id " + Integer.toString(missile.getID()) + " on move due to wall impact");
					iter.remove();
   					continue;
   			}

   			threats = missile.getThreatenedLocations();
   			for (int i = 0; i < threats.length; ++i) {
   				getCell(threats[i]).setRedraw();
   			}
   		}
   	}
   	
   	private Integer[] checkMissilePassThreat(Tank tank) {
   		// The tank and missiles have already moved.
   		java.awt.Point lastTankLocation = new java.awt.Point(tank.getLocation());
   		lastTankLocation = Direction.translate(lastTankLocation, Direction.backwardOf[tank.getLastMoveDirection()]);
   		
   		ArrayList ids = new ArrayList();

   		ListIterator iter = missiles.listIterator();
   		while (iter.hasNext()) {
   			Missile missile = (Missile)iter.next();
   			// Must be moving the opposite direction
   			if (missile.getDirection() != Direction.backwardOf[tank.getLastMoveDirection()]) {
   				continue;
   			}
   			
   			if (missile.getLocation().equals(lastTankLocation)) {
				logger.finer("Missile id " + Integer.toString(missile.getID()) + " pass threat detected for " + tank.getName());
				ids.add(new Integer(missile.getID()));
   			}
   		}
   		if (ids.size() > 0) {
   			return (Integer[])ids.toArray(new Integer[0]);
   		}
   		return null;
  	}
   	
   	public void addMissile(Tank owner) {
		java.awt.Point location = new java.awt.Point(owner.getLocation());
		location = Direction.translate(location, owner.getFacingInt());
		if (getCell(location).isWall()) {
			logger.fine(owner.getName() + " fired a missile in to a wall");
			return;
		}
		Missile missile = new Missile(location, owner.getFacingInt(), owner);
		missiles.add(missile);
		logger.info(owner.getName() + " fired missile id " + Integer.toString(missile.getID()));
   	}
   	
   	public int checkMissileIncoming(java.awt.Point location) {
   		int incoming = 0;
   		ListIterator iter = missiles.listIterator();
   		while (iter.hasNext()) {
   			Missile missile = (Missile)iter.next();
   			if (missile.getLocation().x == location.x) {
   				// We're in the same row
   				if (missile.getLocation().y < location.y) {
   					// The missile is above us
   					if (missile.getDirection() == Direction.kSouthInt) {
   						// and travelling toward us
   						incoming = Direction.kNorthIndicator;
   					}
   				} else {
   					// The missile is below us
   					if (missile.getDirection() == Direction.kNorthInt) {
   						// and travelling toward us
   						incoming = Direction.kSouthIndicator;
   					}
   				}
   			}
   			if (missile.getLocation().y == location.y) {
   				// We're in the same column
   				if (missile.getLocation().x < location.x) {
   					// The missile is to our left
   					if (missile.getDirection() == Direction.kEastInt) {
   						// and travelling toward us
   						incoming = Direction.kWestIndicator;
   					}
   				} else {
   					// The missile is to our right
   					if (missile.getDirection() == Direction.kWestInt) {
   						// and travelling toward us
   						incoming = Direction.kEastIndicator;
   					}
   				}
   			}
   		}
   		return incoming;
   	}
   	
   	public LinkedList getMissiles() {
  		return missiles;
   	}
}
