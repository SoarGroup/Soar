package eaters;

import java.util.*;

import simulation.*;
import sml.*;
import utilities.*;

public class EatersWorld extends World implements WorldManager {
	private static final String kTagEatersWorld = "eaters-world";

	private static final String kTagFood = "food";
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
	
	public class Food {
		public static final String kRound = "round";
		public static final String kSquare = "square";
			
		public static final int kRoundInt = 0;
		public static final int kSquareInt = 1;
			
		String m_Name;
		int m_Value;
		int m_Shape;
		String m_ColorString;
		
		protected Logger m_Logger = Logger.logger;

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
	
	public class EatersCell extends Cell {

		private static final int kWallInt = 0;
		private static final int kEmptyInt = 1;
		private static final int kEaterInt = 2;
		private static final int kReservedIDs = 3;		
		
		private Eater m_Eater;
		
		public EatersCell(int foodIndex) {
			m_Type = foodIndex + kReservedIDs;
			m_ScoreCount += m_Food[foodIndex].getValue();
			if (m_Food[foodIndex].getValue() > 0) {
				++m_FoodCount;
			}
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
			m_Modified = true;
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
		
		public Food setEater(Eater eater) {
			m_Modified = true;
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
				m_Modified = true;
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
	
	private EatersCell[][] m_World;
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
			// Open file
			JavaElementXML root = JavaElementXML.ReadFromFile(mapFile);
			
			if (!root.getTagName().equalsIgnoreCase(kTagEatersWorld)) {
				throw new Exception("Not an eaters map!");
			}
			// TODO: Check version
			
			// Read food types from file
			JavaElementXML food = root.findChildByNameThrows(kTagFood);
				
			m_Food = new Food[food.getNumberChildren()];
			for (int i = 0; i < m_Food.length; ++i) {
				JavaElementXML foodType = food.getChild(i);
				
				try {
					m_Food[i] = new Food(
							foodType.getAttributeThrows(kParamName), 
							foodType.getAttributeIntThrows(kParamValue), 
							foodType.getAttributeThrows(kParamShape), 
							foodType.getAttributeThrows(kParamColor));
				} catch (Exception e) {
					throw new Exception("Badly formatted food (index " + i + "): " + e.getMessage()); 
				}
			}

			// Create map
			JavaElementXML cells = root.findChildByNameThrows(kTagCells);
			
			// Get dimentions
			m_WorldSize = cells.getAttributeIntThrows(kParamWorldSize);
			
			boolean randomWalls = cells.getAttributeBooleanDefault(kParamRandomWalls, false);
			boolean randomFood = cells.getAttributeBooleanDefault(kParamRandomFood, false);
						
			// Create map array
			m_World = new EatersCell[m_WorldSize][m_WorldSize];
			
			// Reset food
			m_FoodCount = 0;
			m_ScoreCount = 0;
			
			// generate walls
			// empty is not a wall, it is a food!
			// only generate empty cells in food stage!
			if (randomWalls) {
				generateRandomWalls();
			} else {
				generateWallsFromXML(cells);
			}

			// generate food
			if (randomFood) {
				generateRandomFood();
			} else {
				generateFoodFromXML(cells);
			}

		} catch (Exception e) {
			m_Logger.log("Error loading map: " + e.getMessage());
			return false;
		}
		
		resetEaters();
		
		m_Logger.log(mapFile + " loaded.");
		return true;
	}
	
	private void generateWallsFromXML(JavaElementXML cells) throws Exception {
		for(int row = 0; row < m_WorldSize; ++row) {
			//String rowString = new String();
			for (int col = 0; col < m_WorldSize; ++col) {
				try {
					m_World[row][col] = new EatersCell(cells.getChild(row).getChild(col).getAttributeThrows(kParamType));
					if (!m_World[row][col].isWall()) {
						m_World[row][col].removeFood();
						m_World[row][col] = null;
					}
					//rowString += m_World[row][col];
				} catch (Exception e) {
					throw new Exception("Error (generateWallsFromXML) on row: " + row + ", column: " + col);
				}
			}
			//m_Logger.log(rowString);
		}
	}
	
	private void generateRandomWalls() throws Exception {
		// Generate perimiter wall
		for (int row = 0; row < m_WorldSize; ++row) {
			m_World[row][0] = new EatersCell(kWallID);
			m_World[row][m_WorldSize - 1] = new EatersCell(kWallID);
		}
		for (int col = 1; col < m_WorldSize - 1; ++col) {
			m_World[0][col] = new EatersCell(kWallID);
			m_World[m_WorldSize - 1][col] = new EatersCell(kWallID);
		}
		
		Random random = new Random();
		
		double probability = kLowProbability;
		for (int row = 2; row < m_WorldSize - 2; ++row) {
			for (int col = 2; col < m_WorldSize - 2; ++col) {
				if (noWallsOnCorners(row, col)) {
					if (wallOnAnySide(row, col)) {
						probability = kHigherProbability;					
					}
					if (random.nextDouble() < probability) {
						m_World[row][col] = new EatersCell(kWallID);
					}
					probability = kLowProbability;
				}
			}
		}
	}
	
	private boolean noWallsOnCorners(int row, int col) {
		EatersCell cell = m_World[row + 1][col + 1];
		if (cell != null) {
			return false;
		}
		
		cell = m_World[row - 1][col - 1];
		if (cell != null) {
			return false;
		}
		
		cell = m_World[row + 1][col - 1];
		if (cell != null) {
			return false;
		}
		
		cell = m_World[row - 1][col + 1];
		if (cell != null) {
			return false;
		}
		return true;
	}
	
	private boolean wallOnAnySide(int row, int col) {
		EatersCell cell = m_World[row + 1][col];
		if (cell != null) {
			return true;
		}
		
		cell = m_World[row][col + 1];
		if (cell != null) {
			return true;
		}
		
		cell = m_World[row - 1][col];
		if (cell != null) {
			return true;
		}
		
		cell = m_World[row][col - 1];
		if (cell != null) {
			return true;
		}
		return false;
	}
	
	private void generateFoodFromXML(JavaElementXML cells) throws Exception {
		for(int row = 0; row < m_WorldSize; ++row) {
			//String rowString = new String();
			for (int col = 0; col < m_WorldSize; ++col) {
				if (m_World[row][col] == null) {						
					try {
						m_World[row][col] = new EatersCell(cells.getChild(row).getChild(col).getAttributeThrows(kParamType));
						//rowString += m_World[row][col];
					} catch (Exception e) {
						throw new Exception("Error (generateFoodFromXML) on row: " + row + ", column: " + col);
					}
				}
			}
			//m_Logger.log(rowString);
		}
	}
	
	private void generateRandomFood() {
		Random random = new Random();
		for (int row = 1; row < m_WorldSize - 1; ++row) {
			for (int col = 1; col < m_WorldSize - 1; ++col) {
				if (m_World[row][col] == null) {
					m_World[row][col] = new EatersCell(random.nextInt(m_Food.length));
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
			MapPoint location = findStartingLocation();
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

	void createEater(Agent agent, String productions, String color, MapPoint location) {
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
		m_Logger.log("Couldn't find entity name match for " + entity.getName() + ", ignoring.");
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

	private MapPoint findStartingLocation() {
		// set random starting location
		Random random = new Random();
		MapPoint location = new MapPoint(random.nextInt(m_WorldSize), random.nextInt(m_WorldSize));
		while (getCell(location).isWall() || getCell(location).isEater()) {
			location.x = random.nextInt(m_WorldSize);
			location.y = random.nextInt(m_WorldSize);				
		}
		
		return location;
	}
	
	private void moveEaters() {
		for (int i = 0; i < m_Eaters.length; ++i) {
			Eater.MoveInfo move = m_Eaters[i].getMove();
			if (move == null) {
				continue;
			}

			MapPoint oldLocation = m_Eaters[i].getLocation();
			MapPoint newLocation;
			int distance = move.jump ? 2 : 1;
			if (move.direction.equalsIgnoreCase(Eater.kNorth)) {
				newLocation = new MapPoint(oldLocation.x, oldLocation.y - distance);
			} else if (move.direction.equalsIgnoreCase(Eater.kEast)) {
				newLocation = new MapPoint(oldLocation.x + distance, oldLocation.y);
				
			} else if (move.direction.equalsIgnoreCase(Eater.kSouth)) {
				newLocation = new MapPoint(oldLocation.x, oldLocation.y + distance);
				
			} else if (move.direction.equalsIgnoreCase(Eater.kWest)) {
				newLocation = new MapPoint(oldLocation.x - distance, oldLocation.y);
				
			} else {
				m_Logger.log("Invalid move direction: " + move.direction);
				return;
			}
			
			if (isInBounds(newLocation) && !getCell(newLocation).isWall()) {
				if (!getCell(oldLocation).removeEater()) {
					m_Logger.log("Warning: moving eater " + m_Eaters[i].getName() + " not at old location " + oldLocation);
				}
				m_Eaters[i].setLocation(newLocation);
				if (move.jump) {
					m_Eaters[i].adjustPoints(kJumpPenalty);
				}
			} else {
				m_Eaters[i].adjustPoints(kWallPenalty);
			}
		}
	}
	
	public EatersCell getCell(MapPoint location) {
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
					m_Eaters[i].adjustPoints(f.getValue());
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
		// reset modified flags, skipping edges
		for (int y = 1; y < m_World.length - 1; ++y) {
			for (int x = 1; x < m_World[y].length - 1; ++x) {
				m_World[y][x].clearModified();
				if (m_World[y][x].checkCollision()) {
					m_World[y][x].setCollision(false);
				}
			}
		}		
		
		if (m_Simulation.reachedMaxUpdates()) {
			m_Logger.log("Reached maximum updates, stopping.");
			m_Simulation.stopSimulation();
			m_PrintedStats = true;
			m_Logger.log("All of the food is gone.");
			for (int i = 0; i < m_Eaters.length; ++i) {
				m_Logger.log(m_Eaters[i].getName() + ": " + m_Eaters[i].getPoints());
			}
			return;
		}
		
		if (getFoodCount() <= 0) {
			if (!m_PrintedStats) {
				m_Simulation.stopSimulation();
				m_PrintedStats = true;
				m_Logger.log("All of the food is gone.");
				for (int i = 0; i < m_Eaters.length; ++i) {
					m_Logger.log(m_Eaters[i].getName() + ": " + m_Eaters[i].getPoints());
				}
			}
			return;
		}

		if (m_Eaters == null) {
			m_Logger.log("Update called with no eaters.");
			return;
		}
		
		moveEaters();
		updateMapAndEatFood();
		handleCollisions();	
		updateEaterInput();
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

						m_Logger.log("Starting collision group at " + m_Eaters[i].getLocation());
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
			
			m_Logger.log("Processing collision group " + group + " with " + collidees.length + " collidees.");
			
			// Redistribute wealth
			int cash = 0;			
			for (int i = 0; i < collidees.length; ++i) {
				cash += collidees[i].getPoints();
			}			
			cash /= collidees.length;
			m_Logger.log("Cash to each: " + cash);
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
					collidees[i].adjustPoints(f.getValue());
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

