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
	
	static final String kWallID = "wall";
	static final String kEmptyID = "empty";
	static final String kEaterID = "eater";
	
	private static final int kWallPenalty = -5;
	private static final int kJumpPenalty = -5;

	private static final double kLowProbability = .15;
	private static final double kHigherProbability = .65;
	
	private boolean m_Decay = false;
	
	public class Food {
		public static final String kRound = "round";
		public static final String kSquare = "square";
			
		public static final int kRoundInt = 0;
		public static final int kSquareInt = 1;
			
		String m_Name;
		int m_Value;
		int m_Shape;
		String m_ColorString;
		
		public Food(String name, int value, String shape, String color) {
			m_Name = name;
			m_Value = value;
			if (shape.equalsIgnoreCase(kRound)) {
				m_Shape = kRoundInt;
			} else if (shape.equalsIgnoreCase(kSquare)) {
				m_Shape = kSquareInt;
			}
			m_ColorString = color;
		}
		
		public String getColor() {
			return m_ColorString;
		}
		
		public String getName() {
			return m_Name;
		}
		
		public int getValue() {
			return m_Value;
		}
		
		public int getShape() {
			return m_Shape;
		}
		
		public String getShapeName() {
			switch (m_Shape) {
			case kRoundInt:
				return kRound;
			case kSquareInt:
				return kSquare;
			}
			return null;
		}
	}
	
	private void decayFood() {
		// This function is called if decay=true (in map file) and if so, after
		// each world update.
		// Example food decay function:
		// Remove one from the value of each type of food after each update.
		for (int i = 0; i < m_Food.length; ++i) {
			--m_Food[i].m_Value;
		}
		
		// If you change the value of a food type, you MUST recalculate the remaining
		// food and remaining score member variables!  The following code does this for
		// the entire map:
		this.m_ScoreCount = 0;
		this.m_FoodCount = 0;
		for (int y = 0; y < m_World.length; ++y) {
			for (int x = 0; x < m_World[y].length; ++x) {
				EatersCell cell = m_World[y][x];
				if (cell.isFood()) {
					Food f = m_World[y][x].getFood();
					m_ScoreCount += f.getValue();
					if (f.getValue() > 0) {
						++m_FoodCount;
					}
				}
			}
		}
	}
	
	public void setStopping(boolean status) {
		// FIXME: not implemented (this is used to detect unforced interrupts)
	}
	
	public class EatersCell extends Cell {

		private static final int kWallInt = 0;
		private static final int kEmptyInt = 1;
		private static final int kEaterInt = 2;
		private static final int kReservedIDs = 3;		
		
		private Eater m_Eater;
		
		public EatersCell(int foodIndex) {
			m_Type = kEmptyInt;
			setFood(foodIndex);
		}

		public EatersCell(String name) throws Exception {
			if (name.equalsIgnoreCase(kWallID)) {
				m_Type = kWallInt;
				return;
			} else if (name.equalsIgnoreCase(kEmptyID)) {
				m_Type = kEmptyInt;			
				return;
			} else {	
				int index = getFoodIndexByName(name);
				if (index == -1) {
					throw new Exception("Invalid type name: " + name);
				}
				m_Type = index + kReservedIDs;
				m_ScoreCount += m_Food[index].getValue();
				if (m_Food[index].getValue() > 0) {
					++m_FoodCount;					
				}
				return;
			}
		}
		
		public boolean isWall() {
			return m_Type == kWallInt;
		}
		
		public void setWall() {
			assert m_Eater == null;
			m_Type = kWallInt;
		}
		
		public boolean isEmpty() {
			return m_Type == kEmptyInt;
		}
		
		public boolean isEater() {
			return (m_Type == kEaterInt) || (m_Eater != null);
		}
		
		public boolean isFood() {
			return m_Type >= kReservedIDs;
		}
		
		public boolean removeEater() {
			if (!isEater()) {
				return false;
			}
			m_Redraw = true;
			if (m_Type == kEaterInt) {
				m_Type = kEmptyInt;
			}
			m_Eater = null;
			return true;
		}
		
		public Food setFood(Food newFood) {
			Food oldFood = null;
			if (isFood()) {
				oldFood = removeFood();
			}
			m_Type = getFoodIndexByName(newFood.getName()) + kReservedIDs;
			if (m_Type == -1) {
				m_Type = kEmptyInt;
			} else {
				m_ScoreCount += newFood.getValue();
				if (newFood.getValue() > 0) {
					++m_FoodCount;					
				}
			}
			return oldFood;
		}
		
		void setFood(int foodIndex) {
			if (isFood()) {
				removeFood();
			}
			m_Type = foodIndex + kReservedIDs;
			m_ScoreCount += m_Food[foodIndex].getValue();
			if (m_Food[foodIndex].getValue() > 0) {
				++m_FoodCount;
			}
		}
		
		public Food setEater(Eater eater) {
			m_Redraw = true;
			Food f = null;
			if (isFood()) {
				f = removeFood();
			}
			m_Type = kEaterInt;
			m_Eater = eater;
			return f;
		}
		
		public Eater getEater() {
			return m_Eater;
		}
		
		public String getName() {
			switch (m_Type) {
			case kWallInt:
				return kWallID;
			case kEmptyInt:
				return kEmptyID;
			case kEaterInt:
				return kEaterID;
			default:
				break;
			}
			
			// TODO: risking null exception here
			return getFood().getName();
		}
		
		public Food getFood() {
			if ((m_Type - kReservedIDs) < 0) return null;
			return m_Food[(m_Type - kReservedIDs)];
		}
		
		public Food removeFood() {
			if (isFood()) {
				m_Redraw = true;
				Food f = getFood();
				if (m_Eater == null) {
					m_Type = kEmptyInt;
				} else {
					m_Type = kEaterInt;
				}
				m_ScoreCount -= f.getValue();
				if (f.getValue() > 0) {
					--m_FoodCount;
				}
				return f;
			}
			return null;
		}
	}

	private int getFoodIndexByName(String name) {
		for (int i = 0; i < m_Food.length; ++i) {
			if (m_Food[i].getName().equalsIgnoreCase(name)) {
				return i;
			}
		}
		return -1;
	}
	
	private EatersCell[][] m_World = null;
	private Food[] m_Food;
	private int m_FoodCount;
	private int m_ScoreCount;
	private EatersSimulation m_Simulation;
	private Eater[] m_Eaters;
	private boolean m_PrintedStats = false;
	private ArrayList m_Collisions;
	
	public EatersWorld(EatersSimulation simulation) {
		m_Simulation = simulation;
	}
	
	public boolean load(String mapFile) {
		
		m_PrintedStats = false;
		
		try {
			ElementXML rootTag = ElementXML.ParseXMLFromFile(mapFile);
			if (rootTag == null) {
				throw new Exception("Error parsing file: " + ElementXML.GetLastParseErrorDescription());
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
						m_Food = new Food[mainTag.GetNumberChildren()];
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
							String foodShape = null;
							String foodColor = null;
							
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
									foodShape = value;
									
								} else if (attribute.equalsIgnoreCase(kParamColor)) {
									foodColor = value;
								}
							}
							
							if ((foodName != null) || (foodShape != null) || (foodColor != null)) {
								m_Food[foodTagIndex] = new Food(foodName, foodValue, foodShape, foodColor);
							} else {
								logger.warning("Ignoring food " + foodTagIndex + " because a required attribute is missing.");
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
							throw new Exception("Illegal or missing world size.");
						}
						
						// Create map array
						m_World = new EatersCell[m_WorldSize][m_WorldSize];
						
						// Reset food
						m_FoodCount = 0;
						m_ScoreCount = 0;

						if (!randomWalls || !randomFood) {
							generateMapFromXML(mainTag);
						}
						
						if (randomWalls) {
							generateRandomWalls();
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
				
				if (m_Food == null) {
					assert false;
					throw new Exception("No food tag.");
				}
				if (m_World == null) {
					assert false;
					throw new Exception("No cells tag.");
				}
				
			} else {
				logger.warning("Unknown tag: " + rootTag.GetTagName());
				throw new Exception("No root eaters-world tag.");
			}			
			rootTag.ReleaseRefOnHandle();
			rootTag = null;

		} catch (Exception e) {
			logger.warning("Error loading map: " + e.getMessage());
			return false;
		}
		
		resetEaters();
		
		logger.info(mapFile + " loaded.");
		return true;
	}
	
	private void generateMapFromXML(ElementXML cells) throws Exception {
		if (cells.GetNumberChildren() != m_WorldSize) {
			assert false;
			throw new Exception("Row count different than world size.");
		}
		
		ElementXML rowElement = new ElementXML();
		for(int row = 0; row < m_WorldSize; ++row) {
			cells.GetChild(rowElement, row);
			if (rowElement == null) {
				assert false;
				throw new Exception("Error with row " + row);
			}
			
			if (rowElement.GetNumberChildren() != m_WorldSize) {
				assert false;
				throw new Exception("Column count different than world size.");
			}
			
			ElementXML cellElement = new ElementXML();
			//String rowString = new String();
			for (int col = 0; col < m_WorldSize; ++col) {
				rowElement.GetChild(cellElement, col);
				if (cellElement == null) {
					assert false;
					throw new Exception("Error with row " + row + ", col " + col);
				}

				if (cellElement.GetNumberAttributes() > 0) {
					if (cellElement.GetAttributeName(0).equalsIgnoreCase(kParamType)) {
						m_World[row][col] = new EatersCell(cellElement.GetAttributeValue(0));
						//rowString += m_World[row][col];
					} else {
						throw new Exception("Error with type of cell on row " + row + ", col " + col);
					}
				}
				//if (logger.isLoggable(Level.FINEST)) logger.finest(rowString);
			}
		}
	}
	
	private void generateRandomWalls() throws Exception {
		// Generate perimiter wall
		for (int row = 0; row < m_WorldSize; ++row) {
			if (m_World[row][0] == null) {
				m_World[row][0] = new EatersCell(kWallID);
			} else {
				m_World[row][0].setWall();
			}
			if (m_World[row][m_WorldSize - 1] == null) {
				m_World[row][m_WorldSize - 1] = new EatersCell(kWallID);
			} else {
				m_World[row][m_WorldSize - 1].setWall();
			}
		}
		for (int col = 1; col < m_WorldSize - 1; ++col) {
			if (m_World[0][col] == null) {
				m_World[0][col] = new EatersCell(kWallID);
			} else {
				m_World[0][col].setWall();
			}
			if (m_World[m_WorldSize - 1][col] == null) {
				m_World[m_WorldSize - 1][col] = new EatersCell(kWallID);
			} else {
				m_World[m_WorldSize - 1][col].setWall();
			}
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
							m_World[row][col] = new EatersCell(kWallID);
						} else {
							m_World[row][col].setWall();
						}
					}
					probability = kLowProbability;
				}
			}
		}
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
					m_World[row][col] = new EatersCell(Simulation.random.nextInt(m_Food.length));
				} else if (!m_World[row][col].isWall()) {
					m_World[row][col].setFood(Simulation.random.nextInt(m_Food.length));
				}
			}
		}		
	}
	
	public Food[] getFood() {
		return m_Food;
	}
	
	public Food getFood(int x, int y) {
		return getCell(x,y).getFood();
	}
	
	public int getFoodCount() {
		return m_FoodCount;
	}
	
	public int getScoreCount() {
		return m_ScoreCount;
	}
	
	public String getContentNameByLocation(int x, int y) {
		if (this.isInBounds(x,y)) {
			return getCell(x,y).getName();
		}
		return kEmptyID;
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
		
		Eater eater = new Eater(agent, productions, color, location);
		// Put eater on map, ignore food
		getCell(location).setEater(eater);

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
				getCell(eater.getLocation()).removeEater();
				if (m_Eaters == null) {
					break;
				}
			}
		}
	}

	private java.awt.Point findStartingLocation() {
		// set random starting location
		java.awt.Point location = new java.awt.Point(Simulation.random.nextInt(m_WorldSize), Simulation.random.nextInt(m_WorldSize));
		while (getCell(location).isWall() || getCell(location).isEater()) {
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
				if (!getCell(oldLocation).removeEater()) {
					logger.warning("Warning: moving eater " + m_Eaters[i].getName() + " not at old location " + oldLocation);
				}
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
		return m_World[location.y][location.x];
	}
	
	public EatersCell getCell(int x, int y) {
		return m_World[y][x];
	}
	
	private void updateMapAndEatFood() {
		for (int i = 0; i < m_Eaters.length; ++i) {
			Food f = getCell(m_Eaters[i].getLocation()).setEater(m_Eaters[i]);
			if (f != null) {
				if (m_Eaters[i].isHungry()) {
					m_Eaters[i].adjustPoints(f.getValue(), "food");
				} else {
					getCell(m_Eaters[i].getLocation()).setFood(f);
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
				m_World[y][x].clearRedraw();
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
		
		if (getFoodCount() <= 0) {
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
			decayFood();
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
			getCell(collidees[0].getLocation()).removeEater();

			// Find new locations, update map and consume as necessary
			for (int i = 0; i < collidees.length; ++i) {
				collidees[i].setLocation(findStartingLocation());
				Food f = getCell(collidees[i].getLocation()).setEater(collidees[i]);
				if (f != null) {
					collidees[i].adjustPoints(f.getValue(), "food");
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

