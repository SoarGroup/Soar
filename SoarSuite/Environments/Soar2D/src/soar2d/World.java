package soar2d;

import java.awt.*;
import java.util.*;
import java.util.logging.*;

import soar2d.player.Player;
import soar2d.player.MoveInfo;
import soar2d.world.*;
import soar2d.xml.MapLoader;

public class World {

	private static Logger logger = Logger.getLogger("soar2d");

	private int scoreCount;
	private int worldCount;
	private boolean printedStats = false;
	
	int size;
	private Cell[][] mapCells = null;
	
	private ArrayList<Player> players = new ArrayList<Player>();
	private HashMap<String, java.awt.Point> initialLocations = new HashMap<String, java.awt.Point>();
	private HashMap<String, java.awt.Point> locations = new HashMap<String, java.awt.Point>();
	private HashMap<String, MoveInfo> lastMoves = new HashMap<String, MoveInfo>();
	
	public boolean load(String mapFile) {
		
		MapLoader loader = new MapLoader();
		if (!loader.load(mapFile)) {
			return false;
		}
		
		mapCells = loader.getCells();
		size = loader.getSize();
		
		scoreCount = 0;
		worldCount = 0;
		
		resetPlayers();
		recalculateScoreCount();
		
		logger.info(mapFile + " loaded, world reset.");
		return true;
	}
	
	public Cell getCell(java.awt.Point location) {
		assert location.x >= 0;
		assert location.y >= 0;
		assert location.x < size;
		assert location.y < size;
		return mapCells[location.y][location.x];
	}
	
	public Cell getCell(int x, int y) {
		assert x >= 0;
		assert y >= 0;
		assert x < size;
		assert y < size;
		return mapCells[y][x];
	}
	
	private int foodCount(Cell cell) {
		ArrayList list = cell.getAllWithProperty(Names.kPropertyEdible);
		Iterator iter = list.iterator();
		int count = 0;
		while (iter.hasNext()) {
			count += ((CellObject)iter.next()).getIntProperty(Names.kPropertyPoints);
		}
		return count;
	}
	
	private void recalculateScoreCount() {
		for (int i = 1; i < size - 1; ++i) {
			for (int j = 1; j < size - 1; ++j) {
				scoreCount += foodCount(mapCells[i][j]);
			}
		}
	}

	public int getScoreCount() {
		return scoreCount;
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
	
	private void resetPlayer(Player player) {
		// find a suitable starting location
		Cell startingCell = putInStartingCell(player);
		
		// update score count
		scoreCount -= foodCount(startingCell);
		
		// remove food from it
		startingCell.removeAllWithProperty(Names.kPropertyEdible);
		
		// set points to zero
		player.setPoints(0, "reset");
		
		// reset (init-soar)
		player.reset();
	}
	
	private void updatePlayers() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			player.update(this, locations.get(player.getName()));
		}
	}
	
	public void shutdown() {
		players.clear();
		initialLocations.clear();
		locations.clear();
		lastMoves.clear();
		mapCells = null;
	}
	
	public void removePlayer(String name) {
		ListIterator<Player> iter = players.listIterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			if (player.getName().equals(name)) {
				initialLocations.remove(name);
				java.awt.Point location = locations.remove(name);
				lastMoves.remove(name);
				Cell cell = getCell(location);
				cell.setPlayer(null);
				iter.remove();
				return;
			}
		}
		logger.warning("destroyPlayer: Couldn't find player name match for " + name + ", ignoring.");
	}
	
	private Cell putInStartingCell(Player player) {
		// TODO: change algorithm to select from remaining starting locations
		
		java.awt.Point initialLocation = null;
		java.awt.Point location = null;
		Cell cell;
		
		if (initialLocations.containsKey(player.getName())) {
			initialLocation = new java.awt.Point(initialLocations.get(player.getName()));
			location = new java.awt.Point(initialLocation);
			cell = getCell(location);
		} else {
			location = new java.awt.Point(Simulation.random.nextInt(size), Simulation.random.nextInt(size));
			cell = getCell(location);
		}

		while (!cell.enterable() || (cell.getPlayer() != null)) {
			if (initialLocation != null) {
				logger.warning(player.getName() + ": Initial location (" + initialLocation.x + "," + initialLocation.y + ") is blocked, going random.");
				initialLocation = null;
			}
			location.x = Simulation.random.nextInt(size);
			location.y = Simulation.random.nextInt(size);
			cell = getCell(location);
		}
		// put the player in it
		locations.put(player.getName(), location);
		cell.setPlayer(player);
		return cell;
	}

	public void addPlayer(Player player, java.awt.Point initialLocation) {
		assert !locations.containsKey(player.getName());
		
		players.add(player);
		
		if (initialLocation != null) {
			initialLocations.put(player.getName(), initialLocation);
		}
		resetPlayer(player);
		java.awt.Point location = locations.get(player.getName());
		
		logger.info(player.getName() + ": Spawning at (" + 
				location.x + "," + location.y + ")");

		updatePlayers();
	}
	
	private void moveEaters() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			
			MoveInfo move = player.getMove();
			lastMoves.put(player.getName(), move);
			if (move == null || !move.move) {
				continue;
			}

			// Calculate new location
			java.awt.Point oldLocation = locations.get(player.getName());
			java.awt.Point newLocation = Direction.translate(oldLocation, move.moveDirection);
			if (move.jump) {
				Direction.translate(newLocation, move.moveDirection);
			}
			
			// Verify legal move and commit move
			if (isInBounds(newLocation) && getCell(newLocation).enterable()) {
				assert getCell(oldLocation).getPlayer() != null;
				getCell(oldLocation).setPlayer(null);
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
		return size;
	}

	private void updateMapAndEatFood() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			MoveInfo lastMove = lastMoves.get(player.getName());
			java.awt.Point location = locations.get(player.getName());
			
			Cell cell = getCell(location);
			cell.setPlayer(player);
			
			if (lastMove.eat) {
				eat(player, cell);
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
				cell.removeObject(food);
				scoreCount -= food.getIntProperty(Names.kPropertyEdible);
			}
		}
	}
	
	private void dumpStats() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			logger.info(player.getName() + ": " + player.getPoints());
		}
	}
	
	public void update() {
		if (worldCount >= Soar2D.config.maxUpdates) {
			if (!printedStats) {
				Soar2D.control.stopSimulation();
				printedStats = true;
				logger.info("Reached maximum updates, stopping.");
				dumpStats();
			}
			return;
		}
		
		if (scoreCount <= 0) {
			if (!printedStats) {
				Soar2D.control.stopSimulation();
				printedStats = true;
				logger.info("All of the food is gone.");
				dumpStats();
			}
			return;
		}

		if (players.size() == 0) {
			logger.warning("Update called with no eaters.");
			return;
		}
		
		moveEaters();
		updateMapAndEatFood();
		handleCollisions();	
		updatePlayers();
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
						getCell(locations.get(left.getName())).addCellObject(new CellObject(Names.kExplosion, false, true));
					}
					// Add each right as it is detected
					collision.add(right);
				}
			}
			
			// Add the collision to the total collisions if there is one
			if (collision.size() > 0) {
				collisions.add(collision);
			}
		}
		
		// if there are no total collisions, we're done
		if (collisions.size() < 1) {
			return;
		}
		
		Iterator<ArrayList<Player>> collisionIter = collisions.iterator();
		while (collisionIter.hasNext()) {
			collision = collisionIter.next();

			if (logger.isLoggable(Level.FINE)) logger.fine("Processing collision group with " + collision.size() + " collidees.");

			// Redistribute wealth
			int cash = 0;			
			ListIterator<Player> collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				cash += collideeIter.next().getPoints();
			}
			cash /= collision.size();
			if (logger.isLoggable(Level.FINE)) logger.fine("Cash to each: " + cash);
			collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				collideeIter.next().setPoints(cash, "collision");
			}
			
			// Remove from former location (only one of these for all players)
			getCell(locations.get(collision.get(0).getName())).setPlayer(null);
			
			// Move to new cell, consume food
			collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				Player player = collideeIter.next();
				Cell cell = putInStartingCell(player);
				if (lastMoves.get(player.getName()).eat) {
					eat(player, cell);
				}
			}
		}
	}
	
	protected boolean isInBounds(Point location) {
		return isInBounds(location.x, location.y);
	}

	protected boolean isInBounds(int x, int y) {
		return (x >= 0) && (y >= 0) && (x < size) && (y < size);
	}

	public void reset() {
		worldCount = 0;
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
}
