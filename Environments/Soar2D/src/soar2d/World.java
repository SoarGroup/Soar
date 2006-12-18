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
				setRedraw(cell);
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
				boolean enterable = cell.enterable();
				boolean noPlayer = cell.getPlayer() == null;
				boolean noMissilePack = cell.getAllWithProperty(Names.kPropertyMissiles).size() <= 0;
				boolean noCharger = cell.getAllWithProperty(Names.kPropertyCharger).size() <= 0;
				if (enterable && noPlayer && noMissilePack && noCharger) {
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
			location = initialLocations.get(player.getName());
			if (!availableLocations.contains(location)) {
				logger.warning(player.getName() + ": Initial location (" + location.x + "," + location.y + ") is blocked, going random.");
				location = null;
			}
		}
		
		if (location == null) {
			location = availableLocations.get(Simulation.random.nextInt(availableLocations.size()));
		}
		
		cell = map.getCell(location);
		
		// put the player in it
		locations.put(player.getName(), location);
		cell.setPlayer(player);
		setRedraw(cell);
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
			MoveInfo move = getAndStoreMove(player);			
			if (move == null) {
				return;
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
				setRedraw(oldCell);

				if (move.jump) {
					player.adjustPoints(Soar2D.config.kJumpPenalty, "jump penalty");
				}
				locations.put(player.getName(), newLocation);
				
			} else {
				player.adjustPoints(Soar2D.config.kWallPenalty, "wall collision");
			}
		}
	}
	
	private void stopAndDumpStats(String message, int[] scores) {
		if (!printedStats) {
			printedStats = true;
			Soar2D.control.infoPopUp(message);
			Soar2D.control.stopSimulation();
			boolean draw = false;
			if (scores.length > 1) {
				if (scores[scores.length - 1] ==  scores[scores.length - 2]) {
					if (logger.isLoggable(Level.FINER)) logger.finer("Draw detected.");
					draw = true;
				}
			}
			
			Iterator<Player> iter = players.iterator();
			while (iter.hasNext()) {
				String status = null;
				Player player = iter.next();
				if (player.getPoints() == scores[scores.length - 1]) {
					status = draw ? "draw" : "winner";
				} else {
					status = "loser";
				}
				logger.info(player.getName() + ": " + player.getPoints() + " (" + status + ").");
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
				setRedraw(cell);

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
				setRedraw(cell);
			}
			
			if (lastMove.open) {
				open(player, cell);
				setRedraw(cell);
			}
		}
	}
	private void setRedraw(Cell cell) {
		if (!cell.hasObject(Names.kRedraw)) {
			cell.addCellObject(new CellObject(Names.kRedraw, false, true));
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
	
	private int[] getSortedScores() {
		int[] scores = new int[players.size()];
		Iterator<Player> iter = players.iterator();
		int i = 0;
		Player player;
		while (iter.hasNext()) {
			player = iter.next();
			scores[i] = player.getPoints();
			++i;
		}
		Arrays.sort(scores);
		return scores;
	}
	
	public void update() {
		if (Soar2D.config.terminalMaxUpdates > 0) {
			if (worldCount >= Soar2D.config.terminalMaxUpdates) {
				stopAndDumpStats("Reached maximum updates, stopping.", getSortedScores());
				return;
			}
		}
		
		if (Soar2D.config.terminalWinningScore > 0) {
			int[] scores = getSortedScores();
			if (scores[scores.length - 1] >= Soar2D.config.terminalWinningScore) {
				stopAndDumpStats("At least one player has achieved at least " + Soar2D.config.terminalWinningScore + " points.", scores);
				return;
			}
		}
		
		if (Soar2D.config.terminalPointsRemaining) {
			if (scoreCount <= 0) {
				stopAndDumpStats("There are no points remaining.", getSortedScores());
				return;
			}
		}

		if (Soar2D.config.terminalFoodRemaining) {
			if (foodCount <= 0) {
				stopAndDumpStats("All of the food is gone.", getSortedScores());
				return;
			}
		}

		if (players.size() == 0) {
			logger.warning("Update called with no players.");
			return;
		}
		
		if (Soar2D.config.eaters) {
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
			
		} else if (Soar2D.config.tanksoar) {
			
			// Read Tank output links
			getTankMoves();
			
			// We'll cache the tank new locations
			HashMap<String, java.awt.Point> newLocations = new HashMap<String, java.awt.Point>();
			
			// We'll cache tanks we need to respawn
			ArrayList<Player> killedTanks = new ArrayList<Player>();
			
			// And we'll cache tanks that moved
			ArrayList<Player> movedTanks = new ArrayList<Player>();
			
			// Do cross checks (and only cross checks) first
			// Cross-check:
			// If moving in to a cell with a tank, check that tank for 
			// a move in the opposite direction
			Iterator<Player> playerIter = players.iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				
				// if we exist in the new locations, we can skip ourselves
				if (newLocations.containsKey(player.getName())) {
					continue;
				}
				
				// we haven't been checked yet
				
				MoveInfo playerMove = lastMoves.get(player.getName());

				// Calculate new location if I moved, or just use the old location
				java.awt.Point oldLocation = locations.get(player.getName());
				
				if (!playerMove.move) {
					// No move, cross collision impossible
					newLocations.put(player.getName(), oldLocation);
					continue;
				}
				
				// we moved, calcuate new location
				java.awt.Point newLocation = new java.awt.Point(oldLocation);
				Direction.translate(newLocation, playerMove.moveDirection);
				
				Cell dest = map.getCell(newLocation);
				
				// Check for wall collision
				if (!dest.enterable()) {
					// Moving in to wall, there will be no player in that cell

					// Cancel the move
					playerMove.move = false;
					newLocations.put(player.getName(), locations.get(player.getName()));
					
					// take damage
					String name = dest.getAllWithProperty(Names.kPropertyBlock).get(0).getName();
					player.adjustHealth(Soar2D.config.kTankCollisionPenalty, name);
					
					// TODO: check for kills
					//killedTanks.add(player);
					continue;
				}
				
				// The cell is enterable, check for player
				
				Player other = dest.getPlayer();
				if (other == null) {
					// No tank, cross collision impossible
					newLocations.put(player.getName(), newLocation);
					movedTanks.add(player);
					continue;
				}
				
				// There is another player, check its move
				
				MoveInfo otherMove = lastMoves.get(other.getName());
				if (!otherMove.move) {
					// they didn't move, cross collision impossible
					newLocations.put(player.getName(), newLocation);
					movedTanks.add(player);
					continue;
				}
				
				// the other player is moving, check its direction
				
				if (playerMove.moveDirection != Direction.backwardOf[otherMove.moveDirection]) {
					// we moved but not toward each other, cross collision impossible
					newLocations.put(player.getName(), newLocation);
					movedTanks.add(player);
					continue;
				}

				// Cross collision detected
				
				// take damage
				player.adjustHealth(Soar2D.config.kTankCollisionPenalty, "cross collision " + other.getName());
				other.adjustHealth(Soar2D.config.kTankCollisionPenalty, "cross collision " + player.getName());
				
				// TODO: check for kills
				//killedTanks.add(player);
				//killedTanks.add(other);
				
				// cancel moves
				playerMove.move = false;
				otherMove.move = false;
				
				// store new locations
				newLocations.put(player.getName(), locations.get(player.getName()));
				newLocations.put(other.getName(), locations.get(other.getName()));
			}
			
			// We've eliminated all cross collisions and walls
			
			// We'll need to save where people move, indexed by location
			HashMap<java.awt.Point, ArrayList<Player> > collisionMap = new HashMap<java.awt.Point, ArrayList<Player> >();
			
			// Iterate through players, checking for all other types of collisions
			playerIter = players.iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				MoveInfo playerMove = lastMoves.get(player.getName());
				
				doMoveCollisions(player, playerMove, newLocations, collisionMap, movedTanks);
			}			
			
			// figure out collision damage
			Iterator<ArrayList<Player> > locIter = collisionMap.values().iterator();
			while (locIter.hasNext()) {
				
				ArrayList<Player> collision = locIter.next();
				
				// if there is more than one player, have them all take damage
				if (collision.size() > 1) {
					
					int damage = collision.size() - 1;
					damage *= Soar2D.config.kTankCollisionPenalty;
					
					logger.info("Collision, " + damage + ":");
					
					playerIter = collision.iterator();
					while (playerIter.hasNext()) {
						Player player = playerIter.next();
						player.adjustHealth(damage, "collision");
					}
					
					// TODO: check for kills
					//killedTanks.add()
				}
			}
			
			// Commit tank moves in two steps, remove from old, place in new
			playerIter = movedTanks.iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				
				// remove from past cell
				Cell cell = map.getCell(locations.remove(player.getName()));
				cell.setPlayer(null);
				setRedraw(cell);
			}
			
			playerIter = movedTanks.iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				// put in new cell
				locations.put(player.getName(), newLocations.get(player.getName()));
				Cell cell = map.getCell(locations.get(player.getName()));
				cell.setPlayer(player);
				setRedraw(cell);
			}
			
			// Move all Missiles
//			if (missiles.size() > 0) {
//				if (logger.isLoggable(Level.FINEST)) logger.finest("Moving all missiles");
//				moveMissiles();
//				
//				// Check for Missile-Tank special collisions
//				if (logger.isLoggable(Level.FINEST)) logger.finest("Checking for missile-tank special collisions");
//				for (int i = 0; i < m_Tanks.length; ++i) {
//					if (m_Tanks[i].recentlyMoved()) {
//						//   Tank and Missile swapping spaces
//						Integer[] ids = checkMissilePassThreat(m_Tanks[i]);
//						if (ids != null) {
//							if (logger.isLoggable(Level.FINE)) logger.fine(m_Tanks[i].getName() + ": moved through " + Integer.toString(ids.length) + " missile(s).");
//							//   Assign penalties and awards
//							m_Tanks[i].hitBy(ids);
//							removeMissilesByID(ids);
//							if (!m_Tanks[i].getShieldStatus()) {
//								getCell(m_Tanks[i].getLocation()).setExplosion();
//							}
//						}
//					}
//				}
//			} else {
//				if (logger.isLoggable(Level.FINEST)) logger.finest("Skipping missile movement, no missiles");
//			}
//
//			//   Spawn new Missiles in front of Tanks
//			if (logger.isLoggable(Level.FINEST)) logger.finest("Spawning newly fired missiles");
//			for (int i = 0; i < m_Tanks.length; ++i) {
//				if (m_Tanks[i].firedMissile()) {
//					addMissile(m_Tanks[i]);
//				}
//			}
//			
//			// Check for Missile-Tank collisions
//			if (missiles.size() > 0) {
//				if (logger.isLoggable(Level.FINEST)) logger.finest("Checking for missile-tank collisions");
//				for (int i = 0; i < m_Tanks.length; ++i) {
//					Integer[] ids = checkForMissileThreat(m_Tanks[i].getLocation());
//					if (ids != null) {
//						if (logger.isLoggable(Level.FINE)) logger.fine(m_Tanks[i].getName() + ": hit by " + Integer.toString(ids.length) + " missile(s).");
//						//   Assign penalties and awards
//						m_Tanks[i].hitBy(ids);
//						removeMissilesByID(ids);
//						if (!m_Tanks[i].getShieldStatus()) {
//							getCell(m_Tanks[i].getLocation()).setExplosion();
//						}
//					}
//				}
//			} else {
//				if (logger.isLoggable(Level.FINEST)) logger.finest("Skipping missile-tank collision check, no missiles");
//			}
//
			
			// Spawn missile packs
			if (map.numberMissilePacks() < Soar2D.config.kMaxMissilePacks) {
				if (Simulation.random.nextInt(100) < Soar2D.config.kMissilePackRespawnChance) {
					// Create a pack
					CellObject missiles = map.getObjectManager().createRandomObjectWithProperty(Names.kPropertyMissiles);
					assert missiles != null;

					// Get available spots
					ArrayList<java.awt.Point> spots = getAvailableLocations(map);
					assert locations.size() > 0;
					
					// pick one and get the cell
					Cell cell = map.getCell(spots.get(Simulation.random.nextInt(spots.size())));
					assert cell != null;
					
					// add it 
					cell.addCellObject(missiles);
					setRedraw(cell);

					// update the count
					map.incrementMissilePacks();
				}
			}
			
			//  Respawn killed Tanks in safe squares
			playerIter = killedTanks.iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				
				// Get available spots
				ArrayList<java.awt.Point> spots = getAvailableLocations(map);
				assert locations.size() > 0;
				
				// pick one and get the cell
				java.awt.Point location = spots.get(Simulation.random.nextInt(spots.size()));
				Cell cell = map.getCell(location);
				assert cell != null;
				
				// put them in the cell
				cell.setPlayer(player);
				setRedraw(cell);

				// save the location
				locations.put(player.getName(), location);
			}
			
			// Update tanks
			playerIter = players.iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				player.update(this, locations.get(player.getName()));
			}
			
			// Commit input
			playerIter = players.iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				player.commit();
			}
			
		} else {
			Soar2D.control.severeError("Update called, unknown game type.");
		}
		
		++worldCount;
	}
	
	private void doMoveCollisions(Player player, MoveInfo playerMove, 
			HashMap<String, java.awt.Point> newLocations, 
			HashMap<java.awt.Point, ArrayList<Player> > collisionMap, 
			ArrayList<Player> movedTanks) {
		
		// Get destination location
		java.awt.Point newLocation = newLocations.get(player.getName());
		
		// Wall collisions checked for earlier
		
		// is there a collision in the cell
		ArrayList<Player> collision = collisionMap.get(newLocation);
		if (collision != null) {
			
			// there is a collision, save location of collision
			java.awt.Point collisionLocation = new java.awt.Point(newLocation);

			// if there is only one player here, cancel its move
			if (collision.size() == 1) {
				Player other = collision.get(0);
				MoveInfo otherMove = lastMoves.get(other.getName());
				if (otherMove.move) {
					otherMove.move = false;
					movedTanks.remove(other);
					newLocations.put(other.getName(), locations.get(other.getName()));
					doMoveCollisions(other, otherMove, newLocations, collisionMap, movedTanks);
				}
			} 

			// If there is more than one guy here, they've already been cancelled
			
			
			// Add ourselves to this collision's list
			collision.add(player);
			collisionMap.put(collisionLocation, collision);
			
			// cancel my move
			if (playerMove.move) {
				playerMove.move = false;
				movedTanks.remove(player);
				newLocations.put(player.getName(), locations.get(player.getName()));
				newLocation = locations.get(player.getName());
				doMoveCollisions(player, playerMove, newLocations, collisionMap, movedTanks);
			}
			return;

		}

		// There is nothing in this cell, create a new list and add ourselves
		collision = new ArrayList<Player>();
		collision.add(player);
		collisionMap.put(newLocation, collision);
	}
	
	private MoveInfo getAndStoreMove(Player player) {
		MoveInfo move = player.getMove();
		if (Soar2D.control.isShuttingDown()) {
			return null;
		}
		lastMoves.put(player.getName(), move);
		
		if (move.stopSim) {
			if (Soar2D.config.terminalAgentCommand) {
				stopAndDumpStats(player.getName() + " issued simulation stop command.", getSortedScores());
			} else {
				Soar2D.logger.warning(player.getName() + " issued ignored stop command.");
			}
		}
		return move;
	}
		
	private void getTankMoves() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			getAndStoreMove(iter.next());
		}
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
		return printedStats;
	}
}
