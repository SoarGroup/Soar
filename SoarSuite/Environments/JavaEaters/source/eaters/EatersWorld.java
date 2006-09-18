package eaters;

import java.util.*;
import java.util.logging.*;

import simulation.*;
import sml.*;
import utilities.*;

public class EatersWorld extends World implements WorldManager {
	private static Logger logger = Logger.getLogger("simulation");
	
	private static final String kTagEatersWorld = "eaters-world";

	private static final String kTagFood = "food";
	private static final String kParamDecay = "decay";
	private static final String kParamName = "name";
	private static final String kParamValue = "value";
	private static final String kParamShape = "shape";
	private static final String kParamColor = "color";

	private static final String kTagCells = "cells";
	private static final String kParamWorldSize = "world-size";
	private static final String kParamRandomWalls = "random-walls";
	private static final String kParamRandomFood = "random-food";
	private static final String kParamType = "type";
	
	private static final int kWallPenalty = -5;
	private static final int kJumpPenalty = -5;

	private static final double kLowProbability = .15;
	private static final double kHigherProbability = .65;
	
	private boolean m_Decay = false;
	
	private int scoreCount = 0;
	
	public void setStopping(boolean status) {
		// FIXME: not implemented (this is used to detect unforced interrupts)
	}
	
	private EatersCell[][] m_World = null;
	private EatersSimulation m_Simulation;
	private Eater[] m_Eaters;
	private boolean m_PrintedStats = false;
	private ArrayList m_Collisions;
	
	public EatersWorld(EatersSimulation simulation) {
		m_Simulation = simulation;
	}
	
	public boolean load(String mapFile) {
		
		m_PrintedStats = false;
		scoreCount = 0;
		
		ElementXML rootTag = ElementXML.ParseXMLFromFile(mapFile);
		if (rootTag == null) {
			m_Simulation.errorMessageWarning("Error parsing file: " + ElementXML.GetLastParseErrorDescription());
			return false;
		}
		if (rootTag.IsTag(kTagEatersWorld)) {
			ElementXML mainTag = null;
			for (int rootTagIndex = 0 ; rootTagIndex < rootTag.GetNumberChildren() ; ++rootTagIndex) {
				mainTag = new ElementXML();
				rootTag.GetChild(mainTag, rootTagIndex);
				if (mainTag == null) {
					assert false;
					continue;
				}
				
				if (mainTag.IsTag(kTagFood)) {
					if (mainTag.GetNumberAttributes() > 0) {
						if (mainTag.GetAttributeName(0).equalsIgnoreCase(kParamDecay)) {
							if (mainTag.GetAttributeValue(0).equalsIgnoreCase("true")) {
								m_Decay = true;
							}
						}
					}
					
					ElementXML foodTypeTag = null;
					for (int foodTagIndex = 0; foodTagIndex < mainTag.GetNumberChildren(); ++foodTagIndex) {
						foodTypeTag = new ElementXML();
						mainTag.GetChild(foodTypeTag, foodTagIndex);
						if (foodTypeTag == null) {
							assert false;
							continue;
						}
						
						String foodName = null;
						int foodValue = 0;
						Shape foodShape = Shape.ROUND;
						String foodColor = "red";
						
						for (int attrIndex = 0; attrIndex < foodTypeTag.GetNumberAttributes(); ++attrIndex) {
							String attribute = foodTypeTag.GetAttributeName(attrIndex);
							if (attribute == null) {
								assert false;
								continue;
							}
							
							String value = foodTypeTag.GetAttributeValue(attrIndex);
							if (value == null) {
								assert false;
								continue;
							}
							
							if (attribute.equalsIgnoreCase(kParamName)) {
								foodName = value;
								
							} else if (attribute.equalsIgnoreCase(kParamValue)) {
								foodValue = Integer.parseInt(value);
								
							} else if (attribute.equalsIgnoreCase(kParamShape)) {
								foodShape = Shape.getShape(value);
								
							} else if (attribute.equalsIgnoreCase(kParamColor)) {
								foodColor = value;
							}
						}
						
						if (foodName != null) {
							Food.addFood(foodName, foodShape, foodColor, foodValue);
						} else {
							logger.warning("Ignoring food " + foodTagIndex + " because a name attribute is missing.");
						}
						foodTypeTag.delete();
						foodTypeTag = null;
					}
				} else if (mainTag.IsTag(kTagCells)) {
					m_WorldSize = 0;
					
					boolean randomWalls = true;
					boolean randomFood = true;
					
					for (int attrIndex = 0; attrIndex < mainTag.GetNumberAttributes(); ++attrIndex) {
						String attribute = mainTag.GetAttributeName(attrIndex);
						if (attribute == null) {
							assert false;
							continue;
						}
						
						String value = mainTag.GetAttributeValue(attrIndex);
						if (value == null) {
							assert false;
							continue;
						}
						
						if (attribute.equalsIgnoreCase(kParamWorldSize)) {
							m_WorldSize = Integer.parseInt(value);
							
						} else if (attribute.equalsIgnoreCase(kParamRandomWalls)) {
							if (value.equalsIgnoreCase("false")) {
								randomWalls = false;
							}
							
						} else if (attribute.equalsIgnoreCase(kParamRandomFood)) {
							if (value.equalsIgnoreCase("false")) {
								randomFood = false;
							}
						}
					}
					
					if (m_WorldSize < 3) {
						m_Simulation.errorMessageWarning("Illegal or missing world size.");
						return false;
					}
					
					// Reset food
					EatersCell.resetFoodCount();
					
					// Create map array
					m_World = new EatersCell[m_WorldSize][m_WorldSize];
					
					if (!randomWalls || !randomFood) {
						if (!generateMapFromXML(mainTag)) {
							return false;
						}
					}
					
					if (randomWalls) {
						if (!generateRandomWalls()) {
							return false;
						}
					}
					
					if (randomFood) {
						generateRandomFood();
					}
				} else {
					logger.warning("Unknown tag: " + mainTag.GetTagName());
				}
				mainTag.delete();
				mainTag = null;
			}
			
			if (Food.foodTypeCount() <= 0) {
				assert false;
				m_Simulation.errorMessageWarning("No food tag.");
				return false;
			}
			if (m_World == null) {
				assert false;
				m_Simulation.errorMessageWarning("No cells tag.");
				return false;
			}
			
		} else {
			logger.warning("Unknown tag: " + rootTag.GetTagName());
			m_Simulation.errorMessageWarning("No root eaters-world tag.");
			return false;
		}			
		
		// BADBAD: voigtjr: don't even get me started on this:
		assert rootTag.GetRefCount() == 1;
		rootTag.ReleaseRefOnHandle();
		rootTag = null;
		System.gc();
		// BADBAD: Yes, we just called the garbage collector.
		
		resetEaters();
		recalculateScoreCount();
		
		logger.info(mapFile + " loaded.");
		return true;
	}
	
	private boolean generateMapFromXML(ElementXML cells) {
		if (cells.GetNumberChildren() != m_WorldSize) {
			assert false;
			m_Simulation.errorMessageWarning("Row count different than world size.");
			return false;
		}
		
		ElementXML rowElement = new ElementXML();
		for(int row = 0; row < m_WorldSize; ++row) {
			cells.GetChild(rowElement, row);
			if (rowElement == null) {
				assert false;
				m_Simulation.errorMessageWarning("Error with row " + row);
				return false;
			}
			
			if (rowElement.GetNumberChildren() != m_WorldSize) {
				assert false;
				m_Simulation.errorMessageWarning("Column count different than world size.");
				return false;
			}
			
			ElementXML cellElement = new ElementXML();
			//String rowString = new String();
			for (int col = 0; col < m_WorldSize; ++col) {
				rowElement.GetChild(cellElement, col);
				if (cellElement == null) {
					assert false;
					m_Simulation.errorMessageWarning("Error with row " + row + ", col " + col);
					return false;
				}

				if (cellElement.GetNumberAttributes() > 0) {
					if (cellElement.GetAttributeName(0).equalsIgnoreCase(kParamType)) {
						m_World[row][col] = new EatersCell();
						m_World[row][col].set(cellElement.GetAttributeValue(0));
						//rowString += m_World[row][col];
					} else {
						m_Simulation.errorMessageWarning("Error with type of cell on row " + row + ", col " + col);
						return false;
					}
				}
				//if (logger.isLoggable(Level.FINEST)) logger.finest(rowString);
			}
		}
		return true;
	}
	
	private boolean generateRandomWalls() {
		// Generate perimiter wall
		for (int row = 0; row < m_WorldSize; ++row) {
			if (m_World[row][0] == null) {
				m_World[row][0] = new EatersCell();
			}
			m_World[row][0].setType(CellType.WALL);
			if (m_World[row][m_WorldSize - 1] == null) {
				m_World[row][m_WorldSize - 1] = new EatersCell();
			}
			m_World[row][m_WorldSize - 1].setType(CellType.WALL);
		}
		for (int col = 1; col < m_WorldSize - 1; ++col) {
			if (m_World[0][col] == null) {
				m_World[0][col] = new EatersCell();
			}
			m_World[0][col].setType(CellType.WALL);
			if (m_World[m_WorldSize - 1][col] == null) {
				m_World[m_WorldSize - 1][col] = new EatersCell();
			}
			m_World[m_WorldSize - 1][col].setType(CellType.WALL);
		}
		
		double probability = kLowProbability;
		for (int row = 2; row < m_WorldSize - 2; ++row) {
			for (int col = 2; col < m_WorldSize - 2; ++col) {
				if (noWallsOnCorners(row, col)) {
					if (wallOnAnySide(row, col)) {
						probability = kHigherProbability;					
					}
					if (Simulation.random.nextDouble() < probability) {
						if (m_World[row][col] == null) {
							m_World[row][col] = new EatersCell();
						}
						m_World[row][col].setType(CellType.WALL);
					}
					probability = kLowProbability;
				}
			}
		}
		return true;
	}
	
	private boolean noWallsOnCorners(int row, int col) {
		EatersCell cell = m_World[row + 1][col + 1];
		if (cell != null && cell.isWall()) {
			return false;
		}
		
		cell = m_World[row - 1][col - 1];
		if (cell != null && cell.isWall()) {
			return false;
		}
		
		cell = m_World[row + 1][col - 1];
		if (cell != null && cell.isWall()) {
			return false;
		}
		
		cell = m_World[row - 1][col + 1];
		if (cell != null && cell.isWall()) {
			return false;
		}
		return true;
	}
	
	private void recalculateScoreCount() {
		for (int i = 1; i < m_WorldSize - 1; ++i) {
			for (int j = 1; j < m_WorldSize - 1; ++j) {
				if (m_World[i][j].getFood() != null) {
					scoreCount += m_World[i][j].getFood().value();
				}
			}
		}
	}
	
	private boolean wallOnAnySide(int row, int col) {
		EatersCell cell = m_World[row + 1][col];
		if (cell != null && cell.isWall()) {
			return true;
		}
		
		cell = m_World[row][col + 1];
		if (cell != null && cell.isWall()) {
			return true;
		}
		
		cell = m_World[row - 1][col];
		if (cell != null && cell.isWall()) {
			return true;
		}
		
		cell = m_World[row][col - 1];
		if (cell != null && cell.isWall()) {
			return true;
		}
		return false;
	}
	
	private void generateRandomFood() {
		for (int row = 1; row < m_WorldSize - 1; ++row) {
			for (int col = 1; col < m_WorldSize - 1; ++col) {
				if (m_World[row][col] == null) {
					m_World[row][col] = new EatersCell();
					
				}
				if (!m_World[row][col].isWall()) {
					m_World[row][col].setFood(Food.getFood(Simulation.random.nextInt(Food.foodTypeCount())));
				}
			}
		}		
	}
	
	public Food getFood(int x, int y) {
		return getCell(x,y).getFood();
	}
	
	public int getScoreCount() {
		return scoreCount;
	}
	
	void resetEaters() {
		if (m_Eaters == null) {
			return;
		}
		for (int i = 0; i < m_Eaters.length; ++i) {
			java.awt.Point location = findStartingLocation();
			m_Eaters[i].setLocation(location);
			m_Eaters[i].setMoved();
			// Put eater on map, ignore food
			getCell(location).setEater(m_Eaters[i]);
			m_Eaters[i].setPoints(0);
			m_Eaters[i].initSoar();
		}
		updateEaterInput();
	}

	void createEater(Agent agent, String productions, String color) {
		createEater(agent, productions, color, null);
	}

	void createEater(Agent agent, String productions, String color, java.awt.Point location) {
		if (location != null) {
			if (this.isInBounds(location)) {
				if (getCell(location).isWall()) {
					logger.warning("Initial location " + location + " is wall, going random.");
					location = null;
				}
			} else {
				logger.warning("Initial location " + location + " is out of bounds, going random.");
				location = null;
			}
		}

		if (location == null) {
			location = findStartingLocation();
		}
		
		if (color == null) {
			for (int i = 0; i < simulation.visuals.WindowManager.kColors.length; ++i) {
				boolean skip = false;
				for (int j = 0; j < this.m_Eaters.length; ++j) {
					if (m_Eaters[j].getColor().equalsIgnoreCase(simulation.visuals.WindowManager.kColors[i])) {
						skip = true;
					}
				}
				if (!skip) {
					color = simulation.visuals.WindowManager.kColors[i];
					break;
				}
			}
		}
		assert color != null;
		
		Eater eater = new Eater(agent, productions, color, location);
		// Put eater on map, remove food
		getCell(location).setEater(eater);
		getCell(location).setFood(null);

		if (m_Eaters == null) {
			m_Eaters = new Eater[1];
			m_Eaters[0] = eater;
		} else {
			Eater[] original = m_Eaters;
			m_Eaters = new Eater[original.length + 1];
			for (int i = 0; i < original.length; ++i) {
				m_Eaters[i] = original[i];
			}
			m_Eaters[original.length] = eater;
		}

		updateEaterInput();
	}
	
	public boolean noAgents() {
		return (m_Eaters == null);
	}
	
	public void shutdown() {
		while (m_Eaters != null) {
			m_Simulation.destroyEntity(m_Eaters[0]);
		}
	}
	
	public void destroyEntity(WorldEntity entity) {
		for (int i = 0; i < m_Eaters.length; ++i) {
			if (m_Eaters[i].getName() == entity.getName()) {
				destroyEater(m_Eaters[i]);
				return;
			}
		}
		logger.warning("Couldn't find entity name match for " + entity.getName() + ", ignoring.");
	}
	
	void destroyEater(Eater eater) {
		if (m_Eaters == null) {
			return;
		}
		for (int i = 0; i < m_Eaters.length; ++i) {
			if (eater == m_Eaters[i]) {
				if (m_Eaters.length == 1) {
					m_Eaters = null;
				} else {
					Eater[] original = m_Eaters;
					m_Eaters = new Eater[original.length - 1];
					for (int j = 0; j < m_Eaters.length; ++j) {
						if (j < i) {
							m_Eaters[j] = original[j];
						} else {
							m_Eaters[j] = original[j+1];
						}
					}
				}
				getCell(eater.getLocation()).setEater(null);
				if (m_Eaters == null) {
					break;
				}
			}
		}
	}

	private java.awt.Point findStartingLocation() {
		// set random starting location
		java.awt.Point location = new java.awt.Point(Simulation.random.nextInt(m_WorldSize), Simulation.random.nextInt(m_WorldSize));
		while (getCell(location).isWall() || (getCell(location).getEater() != null)) {
			location.x = Simulation.random.nextInt(m_WorldSize);
			location.y = Simulation.random.nextInt(m_WorldSize);				
		}
		
		return location;
	}
	
	private void moveEaters() {
		for (int i = 0; i < m_Eaters.length; ++i) {
			Eater.MoveInfo move = m_Eaters[i].getMove();
			if (move == null) {
				continue;
			}

			java.awt.Point oldLocation = m_Eaters[i].getLocation();
			java.awt.Point newLocation;
			int distance = move.jump ? 2 : 1;
			if (move.direction.equalsIgnoreCase(Direction.kNorthString)) {
				newLocation = new java.awt.Point(oldLocation.x, oldLocation.y - distance);
			} else if (move.direction.equalsIgnoreCase(Direction.kEastString)) {
				newLocation = new java.awt.Point(oldLocation.x + distance, oldLocation.y);
				
			} else if (move.direction.equalsIgnoreCase(Direction.kSouthString)) {
				newLocation = new java.awt.Point(oldLocation.x, oldLocation.y + distance);
				
			} else if (move.direction.equalsIgnoreCase(Direction.kWestString)) {
				newLocation = new java.awt.Point(oldLocation.x - distance, oldLocation.y);
				
			} else {
				logger.warning("Invalid move direction: " + move.direction);
				return;
			}
			
			if (isInBounds(newLocation) && !getCell(newLocation).isWall()) {
				assert getCell(oldLocation).getEater() != null;
				getCell(oldLocation).setEater(null);
				m_Eaters[i].setLocation(newLocation);
				if (move.jump) {
					m_Eaters[i].adjustPoints(kJumpPenalty, "jump penalty");
				}
				m_Eaters[i].setMoved();
			} else {
				m_Eaters[i].adjustPoints(kWallPenalty, "wall collision");
			}
		}
	}
	
	public EatersCell getCell(java.awt.Point location) {
		assert location.x >= 0;
		assert location.y >= 0;
		assert location.x < m_WorldSize;
		assert location.y < m_WorldSize;
		return m_World[location.y][location.x];
	}
	
	public EatersCell getCell(int x, int y) {
		assert x >= 0;
		assert y >= 0;
		assert x < m_WorldSize;
		assert y < m_WorldSize;
		return m_World[y][x];
	}
	
	public int getSize() {
		return m_WorldSize;
	}

	private void updateMapAndEatFood() {
		for (int i = 0; i < m_Eaters.length; ++i) {
			
			getCell(m_Eaters[i].getLocation()).setEater(m_Eaters[i]);
			Food f = getCell(m_Eaters[i].getLocation()).getFood();
			if (f != null) {
				if (m_Eaters[i].isHungry()) {
					m_Eaters[i].adjustPoints(f.value(), f.name());
					getCell(m_Eaters[i].getLocation()).setFood(null);
				}
			}
		}
	}
	
	private void updateEaterInput() {
		for (int i = 0; i < m_Eaters.length; ++i) {
			m_Eaters[i].updateInput(this);
		}
	}
	
	public void update() {
		// reset modified flags
		for (int y = 0; y < m_World.length; ++y) {
			for (int x = 0; x < m_World[y].length; ++x) {
				m_World[y][x].clearDraw();
				if (m_World[y][x].checkCollision()) {
					m_World[y][x].setCollision(false);
				}
			}
		}		
		
		if (m_Simulation.reachedMaxUpdates()) {
			if (!m_PrintedStats) {
				m_Simulation.stopSimulation();
				m_PrintedStats = true;
				logger.info("Reached maximum updates, stopping.");
				for (int i = 0; i < m_Eaters.length; ++i) {
					logger.info(m_Eaters[i].getName() + ": " + m_Eaters[i].getPoints());
				}
			}
			return;
		}
		
		if (EatersCell.getFoodCount() <= 0) {
			if (!m_PrintedStats) {
				m_Simulation.stopSimulation();
				m_PrintedStats = true;
				logger.info("All of the food is gone.");
				for (int i = 0; i < m_Eaters.length; ++i) {
					logger.info(m_Eaters[i].getName() + ": " + m_Eaters[i].getPoints());
				}
			}
			return;
		}

		if (m_Eaters == null) {
			logger.warning("Update called with no eaters.");
			return;
		}
		
		moveEaters();
		updateMapAndEatFood();
		handleCollisions();	
		updateEaterInput();
		
		if (m_Decay) {
			Food.decay();
			recalculateScoreCount();
		}
	}
		
	private void handleCollisions() {
		// generate collision groups
		ArrayList currentCollision = null;
		
		for (int i = 0; i < m_Eaters.length; ++i) {
			for (int j = i+1; j < m_Eaters.length; ++j) {
				// only check eaters who aren't already colliding
				if (m_Eaters[i].isColliding()) {
					continue;
				}
				
				if (m_Eaters[i].getLocation().equals(m_Eaters[j].getLocation())) {
					
					// Create data structures
					if (m_Collisions == null) {
						m_Collisions = new ArrayList();
					}
					if (currentCollision == null) {
						currentCollision = new ArrayList();
						
						// Add first agent to current collision
						currentCollision.add(m_Eaters[i]);
						
						// Flipping collision flag unnecessary as first agent will not be traversed again

						// Flip collision flag for cell
						getCell(m_Eaters[i].getLocation()).setCollision(true);

						if (logger.isLoggable(Level.FINE)) logger.fine("Starting collision group at " + m_Eaters[i].getLocation());
					}
					
					// Add second agent to current collision
					currentCollision.add(m_Eaters[j]);

					// Flip collision flag for second agent
					m_Eaters[j].setColliding(true);
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
			Eater[] collidees = (Eater[])currentCollision.toArray(new Eater[0]);
			
			if (logger.isLoggable(Level.FINE)) logger.fine("Processing collision group " + group + " with " + collidees.length + " collidees.");
			
			// Redistribute wealth
			int cash = 0;			
			for (int i = 0; i < collidees.length; ++i) {
				cash += collidees[i].getPoints();
			}			
			cash /= collidees.length;
			if (logger.isLoggable(Level.FINE)) logger.fine("Cash to each: " + cash);
			for (int i = 0; i < collidees.length; ++i) {
				collidees[i].setPoints(cash);
			}
			
			// Remove from former location (only one of these for all eaters)
			getCell(collidees[0].getLocation()).setEater(null);

			// Find new locations, update map and consume as necessary
			for (int i = 0; i < collidees.length; ++i) {
				collidees[i].setLocation(findStartingLocation());
				getCell(collidees[i].getLocation()).setEater(collidees[i]);
				Food f = getCell(collidees[i].getLocation()).getFood();
				if (f != null) {
					collidees[i].adjustPoints(f.value(), f.name());
					getCell(collidees[i].getLocation()).setFood(null);
				}
			}
		}
		
		// clear collision groups
		m_Collisions = null;
		
		// clear colliding flags
		for (int i = 0; i < m_Eaters.length; ++i) {
			m_Eaters[i].setColliding(false);
		}		
	}
	
	public Eater[] getEaters() {
		return m_Eaters;
	}
	
	public WorldEntity[] getEntities() {
		return getEaters();
	}
}

