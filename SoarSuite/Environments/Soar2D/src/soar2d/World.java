package soar2d;

import java.awt.*;
import java.util.*;
import java.util.logging.*;
import soar2d.world.*;

public class World {

	private static Logger logger = Logger.getLogger("soar2d");

	private int scoreCount;
	private int worldCount;
	private boolean printedStats = false;
	
	int size;
	private Cell[][] mapCells = null;
	private ArrayList<Entity> entities = new ArrayList<Entity>();
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
		
		resetEntities();
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
	
	void resetEntities() {
		if (entities.size() == 0) {
			return;
		}
		Iterator<Entity> iter = entities.iterator();
		while (iter.hasNext()) {
			// for each player
			Entity entity = iter.next();
			
			resetEntity(entity);
		}
		
		updateEntities();
	}
	
	private void resetEntity(Entity entity) {
		// find a suitable starting location
		Cell startingCell = putInStartingCell(entity);
		
		// update score count
		scoreCount -= foodCount(startingCell);
		
		// remove food from it
		startingCell.removeAllWithProperty(Names.kPropertyEdible);
		
		// set points to zero
		entity.setPoints(0, "reset");
		
		// reset (init-soar)
		entity.reset();
	}
	
	private void updateEntities() {
		Iterator<Entity> iter = entities.iterator();
		while (iter.hasNext()) {
			Entity entity = iter.next();
			entity.update(this, locations.get(entity.getName()));
		}
	}
	
	public void shutdown() {
		Iterator<Entity> iter = entities.iterator();
		while (iter.hasNext()) {
			Entity entity = iter.next();
			entity.shutdown();
		}
		entities.clear();
		initialLocations.clear();
		locations.clear();
		lastMoves.clear();
		mapCells = null;
	}
	
	public void destroyEntity(String name) {
		ListIterator<Entity> iter = entities.listIterator();
		while (iter.hasNext()) {
			Entity entity = iter.next();
			if (entity.getName().equals(name)) {
				initialLocations.remove(name);
				java.awt.Point location = locations.remove(name);
				lastMoves.remove(name);
				Cell cell = getCell(location);
				cell.setEntity(null);
				entity.shutdown();
				iter.remove();
				return;
			}
		}
		logger.warning("destroyEntity: Couldn't find entity name match for " + name + ", ignoring.");
	}
	
	private Cell putInStartingCell(Entity entity) {
		// FIXME: change algorithm to select from remaining starting locations
		
		java.awt.Point initialLocation = null;
		java.awt.Point location = null;
		Cell cell;
		
		if (initialLocations.containsKey(entity.getName())) {
			initialLocation = new java.awt.Point(initialLocations.get(entity.getName()));
			location = new java.awt.Point(initialLocation);
			cell = getCell(location);
		} else {
			location = new java.awt.Point(Simulation.random.nextInt(size), Simulation.random.nextInt(size));
			cell = getCell(location);
		}

		while (!cell.enterable() || (cell.getEntity() != null)) {
			if (initialLocation != null) {
				logger.warning(entity.getName() + ": Initial location (" + initialLocation.x + "," + initialLocation.y + ") is blocked, going random.");
				initialLocation = null;
			}
			location.x = Simulation.random.nextInt(size);
			location.y = Simulation.random.nextInt(size);
			cell = getCell(location);
		}
		// put the player in it
		locations.put(entity.getName(), location);
		cell.setEntity(entity);
		return cell;
	}

	public void createEntity(Entity entity, java.awt.Point initialLocation) {
		assert !locations.containsKey(entity.getName());
		
		entities.add(entity);
		
		if (initialLocation != null) {
			initialLocations.put(entity.getName(), initialLocation);
		}
		resetEntity(entity);
		java.awt.Point location = locations.get(entity.getName());
		
		logger.info(entity.getName() + ": Spawning at (" + 
				location.x + "," + location.y + ")");

		updateEntities();
	}
	
	private void moveEaters() {
		Iterator<Entity> iter = entities.iterator();
		while (iter.hasNext()) {
			Entity entity = iter.next();
			
			MoveInfo move = entity.getMove();
			lastMoves.put(entity.getName(), move);
			if (move == null || !move.move) {
				continue;
			}

			// Calculate new location
			java.awt.Point oldLocation = locations.get(entity.getName());
			java.awt.Point newLocation = Direction.translate(oldLocation, move.moveDirection);
			if (move.jump) {
				Direction.translate(newLocation, move.moveDirection);
			}
			
			// Verify legal move and commit move
			if (isInBounds(newLocation) && getCell(newLocation).enterable()) {
				assert getCell(oldLocation).getEntity() != null;
				getCell(oldLocation).setEntity(null);
				if (move.jump) {
					entity.adjustPoints(Soar2D.config.kJumpPenalty, "jump penalty");
				}
				locations.put(entity.getName(), newLocation);
				
			} else {
				entity.adjustPoints(Soar2D.config.kWallPenalty, "wall collision");
			}
		}
	}
	
	public int getSize() {
		return size;
	}

	private void updateMapAndEatFood() {
		Iterator<Entity> iter = entities.iterator();
		while (iter.hasNext()) {
			Entity entity = iter.next();
			MoveInfo lastMove = lastMoves.get(entity.getName());
			java.awt.Point location = locations.get(entity.getName());
			
			Cell cell = getCell(location);
			cell.setEntity(entity);
			
			if (lastMove.eat) {
				eat(entity, cell);
			}
		}
	}
	
	private void eat(Entity entity, Cell cell) {
		ArrayList<CellObject> list = cell.getAllWithProperty(Names.kPropertyEdible);
		Iterator<CellObject> foodIter = list.iterator();
		while (foodIter.hasNext()) {
			CellObject food = foodIter.next();
			if (food.apply(entity)) {
				// if this returns true, it is consumed
				cell.removeObject(food);
				scoreCount -= food.getIntProperty(Names.kPropertyEdible);
			}
		}
	}
	
	private void dumpStats() {
		Iterator<Entity> iter = entities.iterator();
		while (iter.hasNext()) {
			Entity entity = iter.next();
			logger.info(entity.getName() + ": " + entity.getPoints());
		}
	}
	
	public void update() {
		if (Soar2D.simulation.reachedMaxUpdates()) {
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

		if (entities.size() == 0) {
			logger.warning("Update called with no eaters.");
			return;
		}
		
		moveEaters();
		updateMapAndEatFood();
		handleCollisions();	
		updateEntities();
	}
		
	private void handleCollisions() {
		// Make sure collisions are possible
		if (entities.size() < 2) {
			return;
		}
		
		// Optimization to not check the same name twice
		HashSet<Entity> colliding = new HashSet<Entity>(entities.size());
		ArrayList<Entity> collision = new ArrayList<Entity>(entities.size());
		ArrayList<ArrayList<Entity>> collisions = new ArrayList<ArrayList<Entity>>(entities.size() / 2);

		ListIterator<Entity> leftIter = entities.listIterator();
		while (leftIter.hasNext()) {
			Entity left = leftIter.next();
			
			// Check to see if we're already colliding
			if (colliding.contains(left)) {
				continue;
			}
			
			ListIterator<Entity> rightIter = entities.listIterator(leftIter.nextIndex());
			// Clear collision list now
			collision.clear();
			while (rightIter.hasNext()) {
				// For each entity to my right (I've already checked to my left)
				Entity right = rightIter.next();

				// Check to see if we're already colliding
				if (colliding.contains(right)) {
					continue;
				}
				
				// If the locations match, we have a collision
				if (locations.get(left.getName()).equals(locations.get(right.getName()))) {
					
					// Add to this set to avoid checking same entity again
					colliding.add(left);
					colliding.add(right);
					
					// Add the left the first time a collision is detected
					if (collision.size() == 0) {
						collision.add(left);
						if (logger.isLoggable(Level.FINE)) logger.fine("collision at " + locations.get(left.getName()));
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
		
		Iterator<ArrayList<Entity>> collisionIter = collisions.iterator();
		while (collisionIter.hasNext()) {
			collision = collisionIter.next();

			if (logger.isLoggable(Level.FINE)) logger.fine("Processing collision group with " + collision.size() + " collidees.");

			// Redistribute wealth
			int cash = 0;			
			ListIterator<Entity> collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				cash += collideeIter.next().getPoints();
			}
			cash /= collision.size();
			if (logger.isLoggable(Level.FINE)) logger.fine("Cash to each: " + cash);
			collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				collideeIter.next().setPoints(cash, "collision");
			}
			
			// Remove from former location (only one of these for all entities)
			getCell(locations.get(collision.get(0).getName())).setEntity(null);
			
			// Move to new cell, consume food
			collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				Entity entity = collideeIter.next();
				Cell cell = putInStartingCell(entity);
				if (lastMoves.get(entity.getName()).eat) {
					eat(entity, cell);
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
		
		//FIXME: ???
	}
}
