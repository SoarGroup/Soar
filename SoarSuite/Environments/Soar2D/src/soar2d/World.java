package soar2d;

import java.awt.*;
import java.util.*;
import java.util.logging.*;

import soar2d.player.*;
import soar2d.world.*;
import soar2d.xml.*;

public class World {

	private static Logger logger = Logger.getLogger("soar2d");

	private int scoreCount;
	private int foodCount;
	private int worldCount = 0;
	private boolean printedStats = false;
	
	public GridMap map;
	
	private ArrayList<Player> players = new ArrayList<Player>();
	private HashMap<String, java.awt.Point> initialLocations = new HashMap<String, java.awt.Point>();
	private HashMap<String, java.awt.Point> locations = new HashMap<String, java.awt.Point>();
	private HashMap<String, MoveInfo> lastMoves = new HashMap<String, MoveInfo>();
	
	private boolean terminal = false;
	
	public boolean load() {
		
		MapLoader loader = new MapLoader();
		if (!loader.load()) {
			return false;
		}
		
		GridMap newMap = loader.getMap();
		
		if (Soar2D.config.tanksoar) {
			if (!newMap.hasEnergyCharger()) {
				if (!addCharger(false, newMap)) {
					return false;
				}
			}
			if (!newMap.hasHealthCharger()) {
				if (!addCharger(true, newMap)) {
					return false;
				}
			}
		}
		
		map = newMap;
		
		reset();
		resetPlayers();
		recalculateScoreAndFoodCount();
		
		logger.info("Map loaded, world reset.");
		return true;
	}
	
	private boolean addCharger(boolean health, GridMap newMap) {
		ArrayList<java.awt.Point> location = this.getAvailableLocations(newMap);
		if (location.size() <= 0) {
			Soar2D.control.severeError("No place to put energy charger!");
			return false;
		}
		Cell cell = newMap.getCell(location.get(Simulation.random.nextInt(location.size())));
		assert cell != null;
		CellObject charger = newMap.getObjectManager().createRandomObjectWithProperties(Names.kPropertyHealth, Names.kPropertyCharger);
		if (charger == null) {
			Soar2D.control.severeError("No energy charger in templates!");
			return false;
		}
		cell.addCellObject(charger);
		return true;
	}
	
	private int pointsCount(Cell cell) {
		ArrayList list = cell.getAllWithProperty(Names.kPropertyEdible);
		Iterator iter = list.iterator();
		int count = 0;
		while (iter.hasNext()) {
			count += ((CellObject)iter.next()).getIntProperty(Names.kPropertyPoints);
		}
		return count;
	}
	
	public int getScoreCount() {
		return scoreCount;
	}
	
	public int getFoodCount() {
		return foodCount;
	}
	
	void resetPlayers() {
		if (players.size() == 0) {
			return;
		}
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			// for each player
			Player player = iter.next();
			
			resetPlayer(player);
		}
		
		updatePlayers();
	}
	
	private boolean resetPlayer(Player player) {
		// find a suitable starting location
		Cell startingCell = putInStartingCell(player);
		if (startingCell == null) {
			return false;
		}
		
		// update score count
		scoreCount -= pointsCount(startingCell);
		
		// update food count
		foodCount -= startingCell.getAllWithProperty(Names.kPropertyEdible).size();
		
		// remove food from it
		startingCell.removeAllWithProperty(Names.kPropertyEdible);
		
		// set points to zero
		player.setPoints(0, "reset");
		
		// reset (init-soar)
		player.reset();
		return true;
	}
	
	private void updatePlayers() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			player.update(this, locations.get(player.getName()));
		}
	}
	
	public void shutdown() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			player.shutdown();
		}
		players.clear();
		initialLocations.clear();
		locations.clear();
		lastMoves.clear();
		map.shutdown();
	}
	
	public void removePlayer(String name) {
		ListIterator<Player> iter = players.listIterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			if (player.getName().equals(name)) {
				initialLocations.remove(name);
				java.awt.Point location = locations.remove(name);
				lastMoves.remove(name);
				Cell cell = map.getCell(location);
				cell.setPlayer(null);
				if (!cell.hasObject(Names.kRedraw)) {
					cell.addCellObject(new CellObject(Names.kRedraw, false, true));
				}
				iter.remove();
				return;
			}
		}
		logger.warning("destroyPlayer: Couldn't find player name match for " + name + ", ignoring.");
	}
	
	private ArrayList<java.awt.Point> getAvailableLocations(GridMap theMap) {
		ArrayList<java.awt.Point> availableLocations = new ArrayList<java.awt.Point>();
		for (int x = 0; x < theMap.getSize(); ++x) {
			for (int y = 0; y < theMap.getSize(); ++ y) {
				java.awt.Point availableLocation = new java.awt.Point(x, y);
				Cell cell = theMap.getCell(availableLocation);
				if (cell.enterable() && (cell.getPlayer() == null)) {
					availableLocations.add(availableLocation);
				}
			}
		}
		return availableLocations;
	}
	
	private Cell putInStartingCell(Player player) {
		// Get available cells
		ArrayList<java.awt.Point> availableLocations = getAvailableLocations(map);
		
		// make sure there is an available cell
		if (availableLocations.size() < 1) {
			Soar2D.control.severeError("There are no suitable starting locations for " + player.getName() + ".");
			return null;
		}
		
		Cell cell;
		java.awt.Point location = null;

		if (initialLocations.containsKey(player.getName())) {
			if (availableLocations.contains(initialLocations.get(player.getName()))) {
				location = initialLocations.get(player.getName());
			} else {
				logger.warning(player.getName() + ": Initial location (" + location.x + "," + location.y + ") is blocked, going random.");
			}
		}
		
		if (location == null) {
			location = availableLocations.get(Simulation.random.nextInt(availableLocations.size()));
		}
		
		cell = map.getCell(location);
		
		// put the player in it
		locations.put(player.getName(), location);
		cell.setPlayer(player);
		
		if (!cell.hasObject(Names.kRedraw)) {
			cell.addCellObject(new CellObject(Names.kRedraw, false, true));
		}
		return cell;
	}

	public boolean addPlayer(Player player, java.awt.Point initialLocation) {
		assert !locations.containsKey(player.getName());
		
		players.add(player);
		
		if (initialLocation != null) {
			initialLocations.put(player.getName(), initialLocation);
		}
		
		if (!resetPlayer(player)) {
			initialLocations.remove(player.getName());
			players.remove(player);
			return false;
		}
		java.awt.Point location = locations.get(player.getName());
		
		logger.info(player.getName() + ": Spawning at (" + 
				location.x + "," + location.y + ")");

		updatePlayers();
		return true;
	}
	
	private void moveEaters() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			
			MoveInfo move = player.getMove();
			if (Soar2D.control.isShuttingDown()) {
				return;
			}
			lastMoves.put(player.getName(), move);
			
			if (move.stopSim) {
				if (Soar2D.config.terminalAgentCommand) {
					if (!printedStats) {
						Soar2D.control.stopSimulation();
						printedStats = true;
						Soar2D.control.infoPopUp(player.getName() + " issued simulation stop command.");
						dumpStats();
						terminal = true;
					}
				} else {
					Soar2D.logger.warning(player.getName() + " issued ignored stop command.");
				}
			}
			
			if (!move.move) {
				continue;
			}

			// Calculate new location
			java.awt.Point oldLocation = locations.get(player.getName());
			java.awt.Point newLocation = new java.awt.Point(oldLocation);
			Direction.translate(newLocation, move.moveDirection);
			if (move.jump) {
				Direction.translate(newLocation, move.moveDirection);
			}
			
			// Verify legal move and commit move
			if (isInBounds(newLocation) && map.getCell(newLocation).enterable()) {
				Cell oldCell = map.getCell(oldLocation);
				assert oldCell.getPlayer() != null;
				oldCell.setPlayer(null);
				
				// Add redraw object
				if (!oldCell.hasObject(Names.kRedraw)) {
					oldCell.addCellObject(new CellObject(Names.kRedraw, false, true));
				}

				if (move.jump) {
					player.adjustPoints(Soar2D.config.kJumpPenalty, "jump penalty");
				}
				locations.put(player.getName(), newLocation);
				
			} else {
				player.adjustPoints(Soar2D.config.kWallPenalty, "wall collision");
			}
		}
	}
	
	public int getSize() {
		return map.getSize();
	}

	private void updateMapAndEatFood() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			MoveInfo lastMove = lastMoves.get(player.getName());
			java.awt.Point location = locations.get(player.getName());
			Cell cell = map.getCell(location);
			
			if (lastMove.move || lastMove.jump) {
				cell.setPlayer(player);
				if (!cell.hasObject(Names.kRedraw)) {
					cell.addCellObject(new CellObject(Names.kRedraw, false, true));
				}

				ArrayList<CellObject> moveApply = cell.getAllWithProperty(Names.kPropertyMoveApply);
				if (moveApply.size() > 0) {
					Iterator<CellObject> maIter = moveApply.iterator();
					while (maIter.hasNext()) {
						CellObject object = maIter.next();
						if (object.apply(player)) {
							cell.removeObject(object.getName());
						}
					}
				}
			}
			
			if (!lastMove.dontEat) {
				eat(player, cell);
				if (!cell.hasObject(Names.kRedraw)) {
					cell.addCellObject(new CellObject(Names.kRedraw, false, true));
				}
			}
			
			if (lastMove.open) {
				open(player, cell);
				if (!cell.hasObject(Names.kRedraw)) {
					cell.addCellObject(new CellObject(Names.kRedraw, false, true));
				}
			}
		}
	}
	
	private void eat(Player player, Cell cell) {
		ArrayList<CellObject> list = cell.getAllWithProperty(Names.kPropertyEdible);
		Iterator<CellObject> foodIter = list.iterator();
		while (foodIter.hasNext()) {
			CellObject food = foodIter.next();
			if (food.apply(player)) {
				// if this returns true, it is consumed
				cell.removeObject(food.getName());
				scoreCount -= food.getIntProperty(Names.kPropertyPoints);
				foodCount -= 1;
			}
		}
	}
	
	private void open(Player player, Cell cell) {
		ArrayList<CellObject> boxes = cell.getAllWithProperty(Names.kPropertyBox);
		if (boxes.size() <= 0) {
			Soar2D.logger.warning(player.getName() + " tried to open but there is no box.");
			return;
		}

		// TODO: multiple boxes
		assert boxes.size() <= 1;
		
		CellObject box = boxes.get(0);
		if (box.hasProperty(Names.kPropertyStatus)) {
			if (box.getProperty(Names.kPropertyStatus).equalsIgnoreCase(Names.kOpen)) {
				Soar2D.logger.warning(player.getName() + " tried to open an open box.");
				return;
			}
		}
		if (box.apply(player)) {
			cell.removeObject(box.getName());
		}
	}
	
	private void dumpStats() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			logger.info(player.getName() + ": " + player.getPoints());
		}
	}
	
	private int[] getSortedScores() {
		int[] scores = new int[players.size()];
		Iterator<Player> iter = players.iterator();
		int i = 0;
		Player player;
		while (iter.hasNext()) {
			player = iter.next();
			scores[i] = player.getPoints();
		}
		Arrays.sort(scores);
		return scores;
	}
	
	public void update() {
		if (Soar2D.config.terminalMaxUpdates > 0) {
			if (worldCount >= Soar2D.config.terminalMaxUpdates) {
				if (!printedStats) {
					Soar2D.control.stopSimulation();
					printedStats = true;
					Soar2D.control.infoPopUp("Reached maximum updates, stopping.");
					dumpStats();
					terminal = true;
				}
				return;
			}
		}
		
		if (Soar2D.config.terminalWinningScore > 0) {
			int[] scores = getSortedScores();
			if (scores[scores.length - 1] >= Soar2D.config.terminalWinningScore) {
				if (!printedStats) {
					Soar2D.control.stopSimulation();
					printedStats = true;
					Soar2D.control.infoPopUp("At least one player has achieved at least " + Soar2D.config.terminalWinningScore + " points.");
					dumpStats();
					terminal = true;
				}
				return;
			}
		}
		
		if (Soar2D.config.terminalPointsRemaining) {
			if (scoreCount <= 0) {
				if (!printedStats) {
					Soar2D.control.stopSimulation();
					printedStats = true;
					Soar2D.control.infoPopUp("There are no points remaining.");
					dumpStats();
					terminal = true;
				}
				return;
			}
		}

		if (Soar2D.config.terminalFoodRemaining) {
			if (foodCount <= 0) {
				if (!printedStats) {
					Soar2D.control.stopSimulation();
					printedStats = true;
					Soar2D.control.infoPopUp("All of the food is gone.");
					dumpStats();
					terminal = true;
				}
				return;
			}
		}

		if (players.size() == 0) {
			logger.warning("Update called with no eaters.");
			return;
		}
		
		moveEaters();
		if (Soar2D.control.isShuttingDown()) {
			return;
		}
		updateMapAndEatFood();
		handleCollisions();	
		updatePlayers();
		if (map.getObjectManager().updatablesExist()) {
			updateObjects();
		}
		
		++worldCount;
	}
		
	private void recalculateScoreAndFoodCount() {
		scoreCount = 0;
		foodCount = 0;
		Cell cell;
		for (int i = 1; i < map.getSize() - 1; ++i) {
			for (int j = 1; j < map.getSize() - 1; ++j) {
				cell = map.getCell(i, j);
				scoreCount += pointsCount(cell);
				foodCount += cell.getAllWithProperty(Names.kPropertyEdible).size();
			}
		}
	}

	private void updateObjects() {
		scoreCount = 0;
		foodCount = 0;
		Cell cell;
		java.awt.Point location = new java.awt.Point();
		for (location.x = 0; location.x < map.getSize(); ++location.x) {
			for (location.y = 0; location.y < map.getSize(); ++ location.y) {
				cell = map.getCell(location);
				cell.update(this, location);
				scoreCount += pointsCount(cell);
				foodCount += cell.getAllWithProperty(Names.kPropertyEdible).size();
			}
		}
	}

	private void handleCollisions() {
		// Make sure collisions are possible
		if (players.size() < 2) {
			return;
		}
		
		// Optimization to not check the same name twice
		HashSet<Player> colliding = new HashSet<Player>(players.size());
		ArrayList<Player> collision = new ArrayList<Player>(players.size());
		ArrayList<ArrayList<Player>> collisions = new ArrayList<ArrayList<Player>>(players.size() / 2);

		ListIterator<Player> leftIter = players.listIterator();
		while (leftIter.hasNext()) {
			Player left = leftIter.next();
			
			// Check to see if we're already colliding
			if (colliding.contains(left)) {
				continue;
			}
			
			ListIterator<Player> rightIter = players.listIterator(leftIter.nextIndex());
			// Clear collision list now
			collision.clear();
			while (rightIter.hasNext()) {
				// For each player to my right (I've already checked to my left)
				Player right = rightIter.next();

				// Check to see if we're already colliding
				if (colliding.contains(right)) {
					continue;
				}
				
				// If the locations match, we have a collision
				if (locations.get(left.getName()).equals(locations.get(right.getName()))) {
					
					// Add to this set to avoid checking same player again
					colliding.add(left);
					colliding.add(right);
					
					// Add the left the first time a collision is detected
					if (collision.size() == 0) {
						collision.add(left);
						if (logger.isLoggable(Level.FINE)) logger.fine("collision at " + locations.get(left.getName()));
						
						// Add a collision object the first time it is detected
						Cell cell = map.getCell(locations.get(left.getName()));
						cell.addCellObject(new CellObject(Names.kExplosion, false, true));
					}
					// Add each right as it is detected
					collision.add(right);
				}
			}
			
			// Add the collision to the total collisions if there is one
			if (collision.size() > 0) {
				collisions.add(new ArrayList<Player>(collision));
			}
		}
		
		// if there are no total collisions, we're done
		if (collisions.size() < 1) {
			return;
		}
		
		Iterator<ArrayList<Player>> collisionIter = collisions.iterator();
		while (collisionIter.hasNext()) {
			collision = collisionIter.next();

			assert collision.size() > 0;
			if (logger.isLoggable(Level.FINE)) logger.fine("Processing collision group with " + collision.size() + " collidees.");

			// Redistribute wealth
			int cash = 0;			
			ListIterator<Player> collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				cash += collideeIter.next().getPoints();
			}
			if (cash > 0) {
				cash /= collision.size();
			}
			if (logger.isLoggable(Level.FINE)) logger.fine("Cash to each: " + cash);
			collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				collideeIter.next().setPoints(cash, "collision");
			}
			
			// Remove from former location (only one of these for all players)
			map.getCell(locations.get(collision.get(0).getName())).setPlayer(null);
			
			// Move to new cell, consume food
			collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				Player player = collideeIter.next();
				Cell cell = putInStartingCell(player);
				assert cell != null;
				if (!lastMoves.get(player.getName()).dontEat) {
					eat(player, cell);
				}
			}
		}
	}
	
	public boolean isInBounds(Point location) {
		return isInBounds(location.x, location.y);
	}

	public boolean isInBounds(int x, int y) {
		return (x >= 0) && (y >= 0) && (x < map.getSize()) && (y < map.getSize());
	}

	public void reset() {
		worldCount = 0;
		printedStats = false;
		terminal = false;
	}
	
	public int getWorldCount() {
		return worldCount;
	}
	
	public boolean hasPlayers() {
		return players.size() > 0;
	}

	public Point getLocation(Player player) {
		return locations.get(player.getName());
	}

	public ArrayList<Player> getPlayers() {
		return players;
	}
	
	boolean isTerminal() {
		return terminal;
	}
}
