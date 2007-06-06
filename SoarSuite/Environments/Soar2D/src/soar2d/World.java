package soar2d;

import java.awt.Point;
import java.awt.geom.Point2D;
import java.util.*;
import java.util.logging.*;
import java.lang.Math;

import soar2d.Configuration.SimType;
import soar2d.player.*;
import soar2d.world.*;

public class World {

	private static Logger logger = Logger.getLogger("soar2d");

	private int worldCount = 0;
	private int runsFoodRemaining = 0;
	private int runsTerminal = 0;
	private boolean printedStats = false;
	
	private GridMap map;
	public GridMap getMap() {
		return map;
	}
	
	private ArrayList<Player> players = new ArrayList<Player>(7);
	private ArrayList<Player> humanPlayers = new ArrayList<Player>(7);
	private HashMap<String, Player> playersMap = new HashMap<String, Player>(7);
	private HashMap<String, Point> initialLocations = new HashMap<String, Point>(7);
	private HashMap<String, Point> locations = new HashMap<String, Point>(7);
	private HashMap<String, Point2D.Double> floatLocations = new HashMap<String, Point2D.Double>(7);
	private HashMap<String, MoveInfo> lastMoves = new HashMap<String, MoveInfo>(7);
	private HashMap<String, HashSet<Player> > killedTanks = new HashMap<String, HashSet<Player> >(7);
	
	private int missileID = 0;
	private int missileReset = 0;
	
	public boolean load() {
		this.runsFoodRemaining = Soar2D.config.getTerminalFoodRemainingContinue();
		this.runsTerminal = Soar2D.config.getTerminalMaxRuns();
		return loadInternal(false);
	}

	private boolean loadInternal(boolean resetDuringRun) {
		GridMap newMap = new GridMap(Soar2D.config);
		
		try {
			newMap.load();
		} catch (GridMap.LoadError e) {
			Soar2D.control.severeError(e.getMessage());
			return false;
		}
		
		// Map post-processing
		switch (Soar2D.config.getType()) {
		case kTankSoar:
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
			break;
			
		case kEaters:
			break;
			
		case kBook:
			if (newMap.generateRoomStructure() == false) {
				return false;
			}
			break;
		
		}
		
		map = newMap;
		
		if (Soar2D.config.getType() == SimType.kTankSoar) {
			// Spawn missile packs
			while (map.numberMissilePacks() < Soar2D.config.getMaxMissilePacks()) {
				spawnMissilePack(true);
			}
		}
		
		reset();
		resetPlayers(resetDuringRun);
		
		logger.info("Map loaded, world reset.");
		if (map.getOpenCode() != 0) {
			logger.info("The correct open code is: " + map.getOpenCode());
		}
		return true;
	}
	
	private boolean addCharger(boolean health, GridMap newMap) {
		ArrayList<Point> locations = this.getAvailableLocations(newMap);
		if (locations.size() <= 0) {
			Soar2D.control.severeError("No place to put charger!");
			return false;
		}
		
		Point location = locations.get(Simulation.random.nextInt(locations.size()));
		if (health) {
			logger.info("spawning health charger at (" + location.x + "," + location.y + ")");
			if (!newMap.addRandomObjectWithProperties(location, Names.kPropertyHealth, Names.kPropertyCharger)) {
				Soar2D.control.severeError("Couldn't add charger to map!");
				return false;
			}
		} else {			
			logger.info("spawning energy charger at (" + location.x + "," + location.y + ")");
			if (!newMap.addRandomObjectWithProperties(location, Names.kPropertyEnergy, Names.kPropertyCharger)) {
				Soar2D.control.severeError("Couldn't add charger to map!");
				return false;
			}
		}
		
		return true;
	}
	
	void resetPlayers(boolean resetDuringRun) {
		if (players.size() == 0) {
			return;
		}
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			// for each player
			Player player = iter.next();
			
			resetPlayer(player, resetDuringRun);
		}
		
		updatePlayers(false);
	}
	
	private boolean resetPlayer(Player player, boolean resetDuringRun) {
		// find a suitable starting location
		Point startingLocation = putInStartingLocation(player);
		if (startingLocation == null) {
			return false;
		}
		
		switch (Soar2D.config.getType()) {
		case kEaters:
			// remove food from it
			map.removeAllWithProperty(startingLocation, Names.kPropertyEdible);
			
			// This is here because the TOSCA stuff wants to keep around the reward
			// in the beginning of the next phase
			
			if (resetDuringRun) {
				player.mapReset();
			}
			
			// falls through
			
		case kTankSoar:
			// reset (init-soar)
			player.reset();
			break;
			
		case kBook:
			if (resetDuringRun) {
				player.fragged();
			} else {
				// reset (init-soar)
				player.reset();
			}
			break;
		}
		return true;
	}
	
	private void updatePlayers(boolean playersChanged) {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			
			if (Soar2D.config.getType() == SimType.kTankSoar) {
				if (playersChanged) {
					player.playersChanged();
				}
				if (players.size() < 2) {
					player.setSmell(0, null);
					player.setSound(0);
				} else {
					int distance = 99;
					String color = null;
					
					Iterator<Player> smellIter = players.iterator();
					while (smellIter.hasNext()) {
						
						Player other = smellIter.next();
						if (other.equals(player)) {
							continue;
						}
						
						Point playerLoc = locations.get(player.getName());
						Point otherLoc = locations.get(other.getName());
						int newDistance = Math.abs(playerLoc.x - otherLoc.x) + Math.abs(playerLoc.y - otherLoc.y);
						
						if (newDistance < distance) {
							distance = newDistance;
							color = other.getColor();
						} else if (newDistance == distance) {
							if (Simulation.random.nextBoolean()) {
								distance = newDistance;
								color = other.getColor();
							}
						}
					}
					player.setSmell(distance, color);
					// TODO: can eliminate sound check if smell is greater than max sound distance 
					player.setSound(map.getSoundNear(locations.get(player.getName())));
				}
				
			}
			player.update(this, locations.get(player.getName()));
		}
		
		if (Soar2D.config.getType() == SimType.kTankSoar) {
			Iterator<Player> playerIter = players.iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				player.commit(locations.get(player.getName()));
			}
			playerIter = players.iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				player.resetPointsChanged();
			}
		}
	}
	
	public void shutdown() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Soar2D.simulation.destroyPlayer(iter.next());
			iter = players.iterator();
		}
		assert this.players.size() == 0;
		assert this.humanPlayers.size() == 0;
		assert this.playersMap.size() == 0;
		assert this.initialLocations.size() == 0;
		assert this.locations.size() == 0;
		assert this.floatLocations.size() == 0;
		assert this.lastMoves.size() == 0;
		map.shutdown();
	}
	
	public void removePlayer(String name) {
		logger.info("Removing player " + name);
		Player player = playersMap.get(name);
		if (player == null) {
			logger.warning("destroyPlayer: Couldn't find player name match for " + name + ", ignoring.");
			return;
		}
		players.remove(player);
		humanPlayers.remove(player);
		playersMap.remove(name);
		initialLocations.remove(name);
		Point location = locations.remove(name);
		floatLocations.remove(name);
		lastMoves.remove(name);
		map.setPlayer(location, null);
		
		updatePlayers(true);
	}
	
	private ArrayList<Point> getAvailableLocations(GridMap theMap) {
		ArrayList<Point> availableLocations = new ArrayList<Point>();
		for (int x = 0; x < theMap.getSize(); ++x) {
			for (int y = 0; y < theMap.getSize(); ++ y) {
				Point potentialLocation = new Point(x, y);
				if (theMap.isAvailable(potentialLocation)) {
					availableLocations.add(potentialLocation);
				}
			}
		}
		return availableLocations;
	}
	
	private Point putInStartingLocation(Player player) {
		// Get available cells
		
		ArrayList<Point> availableLocations = getAvailableLocations(map);
		// make sure there is an available cell
		switch (Soar2D.config.getType()) {
		case kTankSoar:
			// There must be enough room for all tank and missile packs
			if (availableLocations.size() < Soar2D.config.getMaxMissilePacks() + 1) {
				Soar2D.control.severeError("There are no suitable starting locations for " + player.getName() + ".");
				return null;
			}
			break;
		case kEaters:
		case kBook:
			if (availableLocations.size() < 1) {
				Soar2D.control.severeError("There are no suitable starting locations for " + player.getName() + ".");
				return null;
			}
			break;
		}
		
		Point location = null;

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
		
		// put the player in it
		map.setPlayer(location, player);
		locations.put(player.getName(), location);
		if (Soar2D.config.getType() == SimType.kBook) {
			floatLocations.put(player.getName(), defaultFloatLocation(location));
		}
		return location;
	}

	public boolean addPlayer(Player player, Point initialLocation, boolean human) {
		assert !locations.containsKey(player.getName());
		logger.info("Adding player " + player.getName());

		players.add(player);
		playersMap.put(player.getName(), player);
		
		if (initialLocation != null) {
			initialLocations.put(player.getName(), initialLocation);
		}
		
		if (!resetPlayer(player, false)) {
			initialLocations.remove(player.getName());
			players.remove(player);
			playersMap.remove(player.getName());
			return false;
		}
		Point location = locations.get(player.getName());
		
		logger.info(player.getName() + ": Spawning at (" + 
				location.x + "," + location.y + ")");
		
		if (human) {
			humanPlayers.add(player);
		}

		updatePlayers(true);
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
			Point oldLocation = locations.get(player.getName());
			Point newLocation = new Point(oldLocation);
			Direction.translate(newLocation, move.moveDirection);
			if (move.jump) {
				Direction.translate(newLocation, move.moveDirection);
			}
			
			// Verify legal move and commit move
			if (map.isInBounds(newLocation) && map.enterable(newLocation)) {
				// remove from cell
				map.setPlayer(oldLocation, null);
				
				if (move.jump) {
					player.adjustPoints(Soar2D.config.getJumpPenalty(), "jump penalty");
				}
				locations.put(player.getName(), newLocation);
				
			} else {
				player.adjustPoints(Soar2D.config.getWallPenalty(), "wall collision");
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
	
	private void updateMapAndEatFood() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			MoveInfo lastMove = lastMoves.get(player.getName());
			Point location = locations.get(player.getName());
			
			if (lastMove.move || lastMove.jump) {
				map.setPlayer(location, player);

				ArrayList<CellObject> moveApply = map.getAllWithProperty(location, Names.kPropertyMoveApply);
				if (moveApply.size() > 0) {
					Iterator<CellObject> maIter = moveApply.iterator();
					while (maIter.hasNext()) {
						CellObject object = maIter.next();
						if (object.apply(this, player)) {
							map.removeObject(location, object.getName());
						}
					}
				}
			}
			
			if (!lastMove.dontEat) {
				eat(player, location);
			}
			
			if (lastMove.open) {
				open(player, location, lastMove.openCode);
			}
		}
	}
	
	private void eat(Player player, Point location) {
		ArrayList<CellObject> list = map.getAllWithProperty(location, Names.kPropertyEdible);
		Iterator<CellObject> foodIter = list.iterator();
		while (foodIter.hasNext()) {
			CellObject food = foodIter.next();
			if (food.apply(this, player)) {
				// if this returns true, it is consumed
				map.removeObject(location, food.getName());
			}
		}
	}
	
	public void missileHit(Player player, Point location, CellObject missile) {
		// Yes, I'm hit
		missile.apply(this, player);
		
		// apply points
		player.adjustPoints(Soar2D.config.getMissileHitPenalty(), missile.getName());
		Player other = playersMap.get(missile.getProperty(Names.kPropertyOwner));
		// can be null if the player was deleted after he fired but before the missile hit
		if (other != null) {
			other.adjustPoints(Soar2D.config.getMissileHitAward(), missile.getName());
		}
		
		// charger insta-kill
		if (map.getAllWithProperty(location, Names.kPropertyCharger).size() > 0) {
			player.adjustHealth(player.getHealth() * -1, "hit on charger");
		}
		
		// check for kill
		if (player.getHealth() <= 0) {
			HashSet<Player> assailants = killedTanks.get(player.getName());
			if (assailants == null) {
				assailants = new HashSet<Player>();
			}
			if (other != null) {
				assailants.add(other);
			}
			killedTanks.put(player.getName(), assailants);
		}
	}
	
	private void open(Player player, Point location, int openCode) {
		ArrayList<CellObject> boxes = map.getAllWithProperty(location, Names.kPropertyBox);
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
		if (openCode != 0) {
			box.addProperty(Names.kPropertyOpenCode, Integer.toString(openCode));
		}
		if (box.apply(this, player)) {
			map.removeObject(location, box.getName());
		}
		
		if (box.getResetApply()) {
			boolean stopNow = false;
			
			if (this.runsTerminal > 0) {
				this.runsTerminal -= 1;
				if (this.runsTerminal == 0) {
					stopNow = true;
				}
			}

			if (stopNow) {
				this.stopAndDumpStats("Max resets achieved.", getSortedScores());
			} else {
				doRestartAfterUpdate();
			}
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
	
	boolean restartAfterUpdate = false;

	public void update() {
		
		Soar2D.config.setHide(false);
		
		// Collect human input
		Iterator<Player> humanPlayerIter = humanPlayers.iterator();
		while (humanPlayerIter.hasNext()) {
			Player human = humanPlayerIter.next();
			if (!human.getHumanMove()) {
				return;
			}
		}
		
		++worldCount;

		if (Soar2D.config.getTerminalMaxUpdates() > 0) {
			if (worldCount >= Soar2D.config.getTerminalMaxUpdates()) {
				boolean stopNow = true;
				
				if (this.runsTerminal > 0) {
					this.runsTerminal -= 1;
					if (this.runsTerminal > 0) {
						stopNow = false;
					}
				}

				if (stopNow) {
					stopAndDumpStats("Reached maximum updates, stopping.", getSortedScores());
					return;
				} else {
					doRestartAfterUpdate();
				}
			}
		}
		
		if (Soar2D.config.getTerminalWinningScore() > 0) {
			int[] scores = getSortedScores();
			if (scores[scores.length - 1] >= Soar2D.config.getTerminalWinningScore()) {
				if (Soar2D.config.getTerminalWinningScoreContinue()) {
					boolean stopNow = true;
					
					if (this.runsTerminal > 0) {
						this.runsTerminal -= 1;
						if (this.runsTerminal > 0) {
							stopNow = false;
						}
					}

					if (stopNow) {
						stopAndDumpStats("At least one player has achieved at least " + Soar2D.config.getTerminalWinningScore() + " points.", scores);
						return;
					} else {
						doRestartAfterUpdate();
					}
				} else {
					stopAndDumpStats("At least one player has achieved at least " + Soar2D.config.getTerminalWinningScore() + " points.", scores);
					return;
				}
			}
		}
		
		if (Soar2D.config.getTerminalPointsRemaining()) {
			if (map.getScoreCount() <= 0) {
				stopAndDumpStats("There are no points remaining.", getSortedScores());
				return;
			}
		}

		if (Soar2D.config.getTerminalFoodRemaining()) {
			if (map.getFoodCount() <= 0) {
				boolean stopNow = true;
				
				if (Soar2D.config.getTerminalFoodRemainingContinue() != 0) {
					// reduce continues by one if it is positive
					if (runsFoodRemaining > 0) {
						runsFoodRemaining -= 1;
					}

					// we only stop if continues is zero
					if (runsFoodRemaining != 0) {
						stopNow = false;
					}
				}
				
				if (stopNow) {
					stopAndDumpStats("All of the food is gone.", getSortedScores());
					return;
					
				} else {
					doRestartAfterUpdate();
				}
			}
		}

		if (Soar2D.config.getTerminalUnopenedBoxes()) {
			if (map.getUnopenedBoxCount() <= 0) {
				stopAndDumpStats("All of the boxes are open.", getSortedScores());
				return;
			}
		}

		if (players.size() == 0) {
			logger.warning("Update called with no players.");
			Soar2D.control.stopSimulation();
			return;
		}
		
		switch (Soar2D.config.getType()) {
		case kEaters:
			eatersUpdate();
			break;
		case kTankSoar:
			tankSoarUpdate();
			break;
		case kBook:
			bookUpdate();
			break;
		}

		if (restartAfterUpdate) {
			loadInternal(true);
			if (Soar2D.wm.using()) {
				Soar2D.wm.reset();
			}
		}
		
		//System.out.println(map);
	}
	
	private void doRestartAfterUpdate() {
		restartAfterUpdate = true;
		
		int[] scores = getSortedScores();
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
	
	private boolean checkBlocked(Point location) {
		if (map.getAllWithProperty(location, Names.kPropertyBlock).size() > 0) {
			return true;
		}
		if (map.getAllWithProperty(location, "mblock").size() > 0) {
			// FIXME: check height
			return true;
		}
		return false;
	}
	
	
	private void bookMovePlayer(Player player, double time) {
		final int cellSize = Soar2D.config.getBookCellSize();
		
		Point oldLocation = locations.get(player.getName());
		Point newLocation = new Point(oldLocation);

		Point2D.Double oldFloatLocation = floatLocations.get(player.getName());
		Point2D.Double newFloatLocation = new Point2D.Double(oldFloatLocation.x, oldFloatLocation.y);

		newFloatLocation.x += player.getSpeed() * Math.cos(player.getHeadingRadians()) * time;
		newFloatLocation.y += player.getSpeed() * Math.sin(player.getHeadingRadians()) * time;
		
		newLocation.x = (int)newFloatLocation.x / cellSize;
		newLocation.y = (int)newFloatLocation.y / cellSize;
		
		while (checkBlocked(newLocation)) {
			// 1) determine what edge we're intersecting
			if ((newLocation.x != oldLocation.x) && (newLocation.y != oldLocation.y)) {
				// corner case
				java.awt.Point oldx = new java.awt.Point(oldLocation.x, newLocation.y);
				
				// if oldx is blocked
				if (checkBlocked(oldx)) {
					player.setCollisionY(true);
					// calculate y first
					if (newLocation.y > oldLocation.y) {
						// south
						newFloatLocation.y = oldLocation.y * cellSize;
						newFloatLocation.y += cellSize - 0.1;
						newLocation.y = oldLocation.y;
					} 
					else if (newLocation.y < oldLocation.y) {
						// north
						newFloatLocation.y = oldLocation.y * cellSize;
						newLocation.y = oldLocation.y;
					} else {
						assert false;
					}
				} 
				else {
					player.setCollisionX(true);
					// calculate x first
					if (newLocation.x > oldLocation.x) {
						// east
						newFloatLocation.x = oldLocation.x * cellSize;
						newFloatLocation.x += cellSize - 0.1;
						newLocation.x = oldLocation.x;
					} 
					else if (newLocation.x < oldLocation.x) {
						// west
						newFloatLocation.x = oldLocation.x * cellSize;
						newLocation.x = oldLocation.x;
					} else {
						assert false;
					} 
				}
				continue;
			}
			
			if (newLocation.x > oldLocation.x) {
				player.setCollisionX(true);
				// east
				newFloatLocation.x = oldLocation.x * cellSize;
				newFloatLocation.x += cellSize - 0.1;
				newLocation.x = oldLocation.x;
			} 
			else if (newLocation.x < oldLocation.x) {
				player.setCollisionX(true);
				// west
				newFloatLocation.x = oldLocation.x * cellSize;
				newLocation.x = oldLocation.x;
			} 
			else if (newLocation.y > oldLocation.y) {
				player.setCollisionY(true);
				// south
				newFloatLocation.y = oldLocation.y * cellSize;
				newFloatLocation.y += cellSize - 0.1;
				newLocation.y = oldLocation.y;
			} 
			else if (newLocation.y < oldLocation.y) {
				player.setCollisionY(true);
				// north
				newFloatLocation.y = oldLocation.y * cellSize;
				newLocation.y = oldLocation.y;
			}
		}
		
		player.setVelocity(new Point2D.Double(newFloatLocation.x - oldFloatLocation.x, newFloatLocation.y - oldFloatLocation.y));
		map.setPlayer(oldLocation, null);
		locations.put(player.getName(), newLocation);
		floatLocations.put(player.getName(), newFloatLocation);
		map.setPlayer(newLocation, player);
	}
	
	private double totalTime = 0;
	public double getTime() {
		return totalTime;
	}
	
	public double fmod(double a, double mod) {
		double result = a;
		assert mod > 0;
		while (Math.abs(result) > mod) {
			assert mod > 0;
			if (result > 0) {
				result -= mod;
			} else {
				result += mod;
			}
		}
		return result;
	}
	
	private void setRotationAndAbsoluteHeading(Player player, double targetHeading, double time) {
		double relativeHeading = targetHeading - player.getHeadingRadians();
		if (relativeHeading == 0) {
			player.rotateComplete();
			player.setRotationSpeed(0);
			player.resetDestinationHeading();
			return;
		}

		if (relativeHeading < 0) {
			relativeHeading += 2* Math.PI;
		}
		
		if (relativeHeading > Math.PI) {
			player.setRotationSpeed(Soar2D.config.getRotateSpeed() * -1 * time);
			player.setDestinationHeading(targetHeading);
		}
		else {
			player.setRotationSpeed(Soar2D.config.getRotateSpeed() * time);
			player.setDestinationHeading(targetHeading);
		}
		
	}
	
	private void bookUpdate() {
		final double time = (float)Soar2D.config.getCycleTimeSlice() / 1000.0f;
		totalTime += time;
		logger.fine("Total time: " + totalTime);

		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();

			// get each move
			MoveInfo move = getAndStoreMove(player);
			// null move indicates shutdown
			if (move == null) {
				return;
			}
			
			logger.finer("Processing move: " + player.getName());
			
			// update rotation speed
			if (move.rotate) {
				if (move.rotateDirection.equals(Names.kRotateLeft)) {
					logger.finer("Rotate: left");
					player.setRotationSpeed(Soar2D.config.getRotateSpeed() * -1 * time);
				} 
				else if (move.rotateDirection.equals(Names.kRotateRight)) {
					logger.finer("Rotate: right");
					player.setRotationSpeed(Soar2D.config.getRotateSpeed() * time);
				} 
				else if (move.rotateDirection.equals(Names.kRotateStop)) {
					logger.finer("Rotate: stop");
					player.setRotationSpeed(0);
				}
				player.resetDestinationHeading();
			} 
			else if (move.rotateAbsolute) {
				while (move.rotateAbsoluteHeading < 0) {
					logger.finest("Correcting command negative heading");
					move.rotateAbsoluteHeading += 2 * Math.PI;
				}
				move.rotateAbsoluteHeading = fmod(move.rotateAbsoluteHeading, 2 * Math.PI);

				logger.finer("Rotate absolute: " + move.rotateAbsoluteHeading);
				
				setRotationAndAbsoluteHeading(player, move.rotateAbsoluteHeading, time);
			}
			else if (move.rotateRelative) {
				double absoluteHeading = player.getHeadingRadians() + move.rotateRelativeAmount;
				
				while (absoluteHeading < 0) {
					logger.finest("Correcting command negative heading");
					absoluteHeading += 2 * Math.PI;
				}
				absoluteHeading = fmod(absoluteHeading, 2 * Math.PI);
				logger.finer("Rotate relative: " + move.rotateRelativeAmount + ", absolute: " + absoluteHeading);
				setRotationAndAbsoluteHeading(player, absoluteHeading, time);
			}

			// update heading if we rotate
			if (player.getRotationSpeed() != 0) {
				
				double heading = player.getHeadingRadians() + player.getRotationSpeed();
				
				if (heading < 0) {
					logger.finest("Correcting computed negative heading");
					heading += 2 * Math.PI;
				} 
				heading = fmod(heading, 2 * Math.PI);

				logger.finer("Rotating, computed heading: " + heading);
				
				if (player.hasDestinationHeading()) {
					double relativeHeading = player.getDestinationHeading() - heading;
					if (relativeHeading == 0) {
						logger.fine("Destination heading reached: " + heading);
						player.rotateComplete();
						player.resetDestinationHeading();
					} else {

						if (relativeHeading < 0) {
							relativeHeading += 2* Math.PI;
						}

						if (player.getRotationSpeed() < 0) {
							if (relativeHeading < Math.PI) {
								heading = player.getDestinationHeading();
								logger.fine("Destination heading reached: " + heading);
								player.rotateComplete();
								player.resetDestinationHeading();
								player.setRotationSpeed(0);
							} else {
								logger.finer("Destination heading pending");
							}
						}
						else if (player.getRotationSpeed() > 0) {
							if (relativeHeading > Math.PI) {
								logger.fine("Destination heading reached: " + heading);
								heading = player.getDestinationHeading();
								player.rotateComplete();
								player.resetDestinationHeading();
								player.setRotationSpeed(0);
							} else {
								logger.finer("Destination heading pending");
							}
						}
					}
				}
				player.setHeadingRadians(heading);
			}
			
			// update speed
			if (move.forward && move.backward) {
				logger.finer("Move: stop");
				player.setSpeed(0);
			} 
			else if (move.forward) {
				logger.finer("Move: forward");
				player.setSpeed(Soar2D.config.getSpeed());
			}
			else if (move.backward) {
				logger.finer("Move: backward");
				player.setSpeed(Soar2D.config.getSpeed() * -1);
			}
			
			// if we have velocity, process move
			if (player.getSpeed() != 0) {
				bookMovePlayer(player, time);
			}
			
			if (move.get) {
				logger.finer("Move: get");
				CellObject block = map.getObject(move.getLocation, "mblock");
				if (block == null || player.isCarrying()) {
					if (block == null) {
						logger.warning("get command failed, no object");
					} else {
						logger.warning("get command failed, full");
					}
					move.get = false;
					player.updateGetStatus(false);
				} else {
					// FIXME: store get info for processing later
					player.carry(map.getAllWithProperty(move.getLocation, "mblock").get(0));
					map.removeObject(move.getLocation, "mblock");
					player.updateGetStatus(true);
				}
			}
			
			if (move.drop) {
				Point2D.Double dropFloatLocation = new Point2D.Double(floatLocations.get(player.getName()).x, floatLocations.get(player.getName()).y);
				dropFloatLocation.x += Soar2D.config.getBookCellSize() * Math.cos(player.getHeadingRadians());
				dropFloatLocation.y += Soar2D.config.getBookCellSize() * Math.sin(player.getHeadingRadians());
				java.awt.Point dropLocation = new java.awt.Point((int)dropFloatLocation.x / Soar2D.config.getBookCellSize(), (int)dropFloatLocation.y / Soar2D.config.getBookCellSize());
				logger.finer("Move: drop " + dropLocation.x + "," + dropLocation.y);
				
				if (dropLocation.equals(locations.get(player.getName())) || checkBlocked(dropLocation)) {
					if (dropLocation.equals(locations.get(player.getName()))) {
						logger.warning("drop command failed, can't drop on same location");
					} else {
						logger.warning("drop command failed, blocked");
					}
					move.drop = false;
					player.updateDropStatus(false);
				} else {
					// FIXME: store drop info for processing later
					map.addObjectToCell(dropLocation, player.drop());
					player.updateDropStatus(true);
				}
			}
			
		}
		
		handleBookCollisions(findCollisions());
		
		updatePlayers(false);
	}
	
	private Point2D.Double defaultFloatLocation(Point location) {
		Point2D.Double floatLocation = new Point2D.Double();
		final int cellSize = Soar2D.config.getBookCellSize();
		
		// default to center of square
		floatLocation.x = (location.x * cellSize) + (cellSize / 2); 
		floatLocation.y = (location.y * cellSize) + (cellSize / 2); 

		return floatLocation;
	}
	
	private void handleBookCollisions(ArrayList<ArrayList<Player>> collisions) {
		
		// TODO: this is very similar to eaters code, consider combining to reduce redundancy
		
		// if there are no total collisions, we're done
		if (collisions.size() < 1) {
			return;
		}

		ArrayList<Player> collision = new ArrayList<Player>(players.size());
		
		Iterator<ArrayList<Player>> collisionIter = collisions.iterator();
		while (collisionIter.hasNext()) {
			collision = collisionIter.next();

			assert collision.size() > 0;
			if (logger.isLoggable(Level.FINER)) logger.finer("Processing collision group with " + collision.size() + " collidees.");

			// Remove from former location (only one of these for all players)
			map.setPlayer(locations.get(collision.get(0).getName()), null);
			
			// Move to new cell
			ListIterator<Player> collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				Player player = collideeIter.next();
				Point location = putInStartingLocation(player);
				assert location != null;
				player.fragged();
			}
		}
	}
	
	private void eatersUpdate() {
		moveEaters();
		if (Soar2D.control.isShuttingDown()) {
			return;
		}
		updateMapAndEatFood();
		handleEatersCollisions(findCollisions());	
		updatePlayers(false);
		map.updateObjects(this);
	}
	
	private void tankSoarUpdate() {
		
		// Read Tank output links
		getTankMoves();
		
		// We'll cache the tank new locations
		HashMap<String, Point> newLocations = new HashMap<String, Point>();
		
		// And we'll cache tanks that moved
		ArrayList<Player> movedTanks = new ArrayList<Player>(players.size());
		
		// And we'll cache tanks that fired
		ArrayList<Player> firedTanks = new ArrayList<Player>(players.size());
		
		// We need to keep track of killed tanks, reset the list
		killedTanks.clear();
		
		// Cache players who fire (never fails)
		// Rotate players (never fails)
		// Update shields & consume shield energy
		// Update radar & consume radar energy
		// Do cross checks (and only cross checks) first
		// Cross-check:
		// If moving in to a cell with a tank, check that tank for 
		// a move in the opposite direction
		Iterator<Player> playerIter = players.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();
			
			MoveInfo playerMove = lastMoves.get(player.getName());
			
			// Check for fire
			if (playerMove.fire) {
				if (player.getMissiles() > 0) {
					player.adjustMissiles(-1, "fire");
					firedTanks.add(player);
				} else {
					if (Soar2D.logger.isLoggable(Level.FINER)) logger.finer(player.getName() + ": fired with no ammo");
				}
			}
			
			// Check for rotate
			if (playerMove.rotate) {
				int facing = player.getFacingInt();
				if (playerMove.rotateDirection.equals(Names.kRotateLeft)) {
					facing = Direction.leftOf[facing];
				} else if (playerMove.rotateDirection.equals(Names.kRotateRight)) {
					facing = Direction.rightOf[facing];
				} else {
					logger.warning(player.getName() + ": unknown rotation command: " + playerMove.rotateDirection);
				}
				player.setFacingInt(facing);
			}
			
			// Check shields
			if (playerMove.shields) {
				player.setShields(playerMove.shieldsSetting);
			}
			
			// Radar
			if (playerMove.radar) {
				player.setRadarSwitch(playerMove.radarSwitch);
			}
			if (playerMove.radarPower) {
				player.setRadarPower(playerMove.radarPowerSetting);
			}

			// if we exist in the new locations, we can skip ourselves
			if (newLocations.containsKey(player.getName())) {
				continue;
			}
			
			// we haven't been checked yet
			
			// Calculate new location if I moved, or just use the old location
			Point oldLocation = locations.get(player.getName());
			
			if (!playerMove.move) {
				// No move, cross collision impossible
				newLocations.put(player.getName(), oldLocation);
				continue;
			}
			
			// we moved, calcuate new location
			Point newLocation = new Point(oldLocation);
			Direction.translate(newLocation, playerMove.moveDirection);
			
			//Cell dest = map.getCell(newLocation);
			
			// Check for wall collision
			if (!map.enterable(newLocation)) {
				// Moving in to wall, there will be no player in that cell

				// Cancel the move
				playerMove.move = false;
				newLocations.put(player.getName(), locations.get(player.getName()));
				
				// take damage
				String name = map.getAllWithProperty(newLocation, Names.kPropertyBlock).get(0).getName();
				player.adjustHealth(Soar2D.config.getCollisionPenalty(), name);
				
				if (player.getHealth() <= 0) {
					HashSet<Player> assailants = killedTanks.get(player.getName());
					if (assailants == null) {
						assailants = new HashSet<Player>();
					}
					assailants.add(player);
					killedTanks.put(player.getName(), assailants);
				}
				continue;
			}
			
			// The cell is enterable, check for player
			
			Player other = map.getPlayer(newLocation);
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
			
			player.adjustHealth(Soar2D.config.getCollisionPenalty(), "cross collision " + other.getName());
			// Getting rammed on a charger is deadly
			if (map.getAllWithProperty(getLocation(player), Names.kPropertyCharger).size() > 0) {
				player.adjustHealth(player.getHealth() * -1, "hit on charger");
			}
			
			other.adjustHealth(Soar2D.config.getCollisionPenalty(), "cross collision " + player.getName());
			// Getting rammed on a charger is deadly
			if (map.getAllWithProperty(getLocation(other), Names.kPropertyCharger).size() > 0) {
				other.adjustHealth(other.getHealth() * -1, "hit on charger");
			}
			
			
			if (player.getHealth() <= 0) {
				HashSet<Player> assailants = killedTanks.get(player.getName());
				if (assailants == null) {
					assailants = new HashSet<Player>();
				}
				assailants.add(other);
				killedTanks.put(player.getName(), assailants);
			}
			if (other.getHealth() <= 0) {
				HashSet<Player> assailants = killedTanks.get(other.getName());
				if (assailants == null) {
					assailants = new HashSet<Player>();
				}
				assailants.add(player);
				killedTanks.put(other.getName(), assailants);
			}
			
			// cancel moves
			playerMove.move = false;
			otherMove.move = false;
			
			// store new locations
			newLocations.put(player.getName(), locations.get(player.getName()));
			newLocations.put(other.getName(), locations.get(other.getName()));
		}
		
		// We've eliminated all cross collisions and walls
		
		// We'll need to save where people move, indexed by location
		HashMap<Point, ArrayList<Player> > collisionMap = new HashMap<Point, ArrayList<Player> >();
		
		// Iterate through players, checking for all other types of collisions
		// Also, moves are committed at this point and they won't respawn on
		// a charger, so do charging here too
		// and shields and radar
		playerIter = players.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();
			MoveInfo playerMove = lastMoves.get(player.getName());
			
			doMoveCollisions(player, playerMove, newLocations, collisionMap, movedTanks);

			// chargers
			chargeUp(player, newLocations.get(player.getName()));

			// Shields
			if (player.shieldsUp()) {
				if (player.getEnergy() > 0) {
					player.adjustEnergy(Soar2D.config.getShieldEnergyUsage(), "shields");
				} else {
					if (Soar2D.logger.isLoggable(Level.FINER)) logger.finer(player.getName() + ": shields ran out of energy");
					player.setShields(false);
				}
			}
			
			// radar
			if (player.getRadarSwitch()) {
				handleRadarEnergy(player);
			}
		}			
		
		// figure out collision damage
		Iterator<ArrayList<Player> > locIter = collisionMap.values().iterator();
		while (locIter.hasNext()) {
			
			ArrayList<Player> collision = locIter.next();
			
			// if there is more than one player, have them all take damage
			if (collision.size() > 1) {
				
				int damage = collision.size() - 1;
				damage *= Soar2D.config.getCollisionPenalty();
				
				if (Soar2D.logger.isLoggable(Level.FINE)) logger.fine("Collision, " + (damage * -1) + " damage:");
				
				playerIter = collision.iterator();
				while (playerIter.hasNext()) {
					Player player = playerIter.next();
					player.adjustHealth(damage, "collision");
					
					// Getting rammed on a charger is deadly
					if (map.getAllWithProperty(getLocation(player), Names.kPropertyCharger).size() > 0) {
						player.adjustHealth(player.getHealth() * -1, "hit on charger");
					}
					
					// check for kill
					if (player.getHealth() <= 0) {
						HashSet<Player> assailants = killedTanks.get(player.getName());
						if (assailants == null) {
							assailants = new HashSet<Player>();
						}
						// give everyone else involved credit for the kill
						Iterator<Player> killIter = collision.iterator();
						while (killIter.hasNext()) {
							Player other = killIter.next();
							if (other.getName().equals(player.getName())) {
								continue;
							}
							assailants.add(other);
						}
						killedTanks.put(player.getName(), assailants);
					}
				}
			}
		}
		
		// Commit tank moves in two steps, remove from old, place in new
		playerIter = movedTanks.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();
			
			// remove from past cell
			map.setPlayer(locations.remove(player.getName()), null);
		}
		
		// commit the new move, grabbing the missile pack if applicable
		playerIter = movedTanks.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();
			// put in new cell
			Point location = newLocations.get(player.getName());
			locations.put(player.getName(), location);
			map.setPlayer(location, player);
			
			// get missile pack
			ArrayList<CellObject> missilePacks = map.getAllWithProperty(location, Names.kPropertyMissiles);
			if (missilePacks.size() > 0) {
				assert missilePacks.size() == 1;
				CellObject pack = missilePacks.get(0);
				pack.apply(this, player);
				map.removeAllWithProperty(location, Names.kPropertyMissiles);
			}
			
			
			// is there a missile in the cell?
			ArrayList<CellObject> missiles = map.getAllWithProperty(location, Names.kPropertyMissile);
			if (missiles.size() == 0) {
				// No, can't collide
				continue;
			}

			// are any flying toward me?
			Iterator<CellObject> iter = missiles.iterator();
			MoveInfo move = lastMoves.get(player.getName());
			while (iter.hasNext()) {
				CellObject missile = iter.next();
				if (move.moveDirection == Direction.backwardOf[missile.getIntProperty(Names.kPropertyDirection)]) {
					missileHit(player, location, missile);
					map.removeObject(location, missile.getName());

					// explosion
					map.setExplosion(location);
				}
			}
		}
		
		// move missiles to new cells, checking for new victims
		map.updateObjects(this);
		
		// If there is more than one player out there, keep track of how
		// many updates go by before resetting everything to prevent oscillations
		if (players.size() > 1) {
			missileReset += 1;
		}
		
		if (firedTanks.size() > 0) {
			// at least one player has fired a missile, reset the 
			// missle reset counter to zero
			missileReset = 0;
		}
		
		// Spawn new Missiles in front of Tanks
		playerIter = firedTanks.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();
			Point missileLoc = new Point(locations.get(player.getName()));
			
			int direction = player.getFacingInt();
			Direction.translate(missileLoc, direction);
			
			if (!map.isInBounds(missileLoc)) {
				continue;
			}
			if (!map.enterable(missileLoc)) {
				// explosion
				map.setExplosion(missileLoc);
				continue;
			}
			
			CellObject missile = map.createRandomObjectWithProperty(Names.kPropertyMissile);
			missile.setName(player.getName() + "-" + missileID++);
			missile.addProperty(Names.kPropertyDirection, Integer.toString(direction));
			missile.addProperty(Names.kPropertyFlyPhase, "0");
			missile.addProperty(Names.kPropertyOwner, player.getName());
			missile.addProperty(Names.kPropertyColor, player.getColor());
			
			// If there is a tank there, it is hit
			Player other = map.getPlayer(missileLoc);
			if (other != null) {
				missileHit(other, missileLoc, missile);
				
				// explosion
				map.setExplosion(missileLoc);
				
			} else {
				map.addObjectToCell(missileLoc, missile);
			}
		}
		
		// Handle incoming sensors now that all missiles are flying
		map.handleIncoming();
		
		// Spawn missile packs
		if (map.numberMissilePacks() < Soar2D.config.getMaxMissilePacks()) {
			spawnMissilePack(false);
		}
		
		// Respawn killed Tanks in safe squares
		Iterator<String> playerNameIter = killedTanks.keySet().iterator();
		while (playerNameIter.hasNext()) {
			
			// apply points
			String playerName = playerNameIter.next();
			Player player = playersMap.get(playerName);
			
			player.adjustPoints(Soar2D.config.getKillPenalty(), "fragged");
			assert killedTanks.containsKey(playerName);
			playerIter = killedTanks.get(playerName).iterator();
			while (playerIter.hasNext()) {
				Player assailant = playerIter.next();
				if (assailant.getName().equals(player.getName())) {
					continue;
				}
				assailant.adjustPoints(Soar2D.config.getKillAward(), "fragged " + player.getName());
			}
			
			fragPlayer(player);
		}
		
		// if the missile reset counter is 100 and there were no killed tanks
		// this turn, reset all tanks
		if ((missileReset >= Soar2D.config.getMissileResetThreshold()) && (killedTanks.size() == 0)) {
			logger.info("missile reset threshold exceeded, resetting all tanks");
			missileReset = 0;
			playerIter = players.iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				
				fragPlayer(player);
			}
		}
		
		// Update tanks
		updatePlayers(false);
	}
	
	private void fragPlayer(Player player) {
		// remove from past cell
		Point oldLocation = locations.remove(player.getName());
		if (Soar2D.config.getType() == SimType.kBook) {
			floatLocations.remove(player.getName());
		}
		map.setExplosion(oldLocation);
		map.setPlayer(oldLocation, null);

		// Get available spots
		ArrayList<Point> spots = getAvailableLocations(map);
		assert spots.size() > 0;
		
		// pick one and put the player in it
		Point location = spots.get(Simulation.random.nextInt(spots.size()));
		map.setPlayer(location, player);
		
		// save the location
		locations.put(player.getName(), location);
		if (Soar2D.config.getType() == SimType.kBook) {
			floatLocations.put(player.getName(), defaultFloatLocation(location));
		}
		
		// reset the player state
		player.fragged();

		logger.info(player.getName() + ": Spawning at (" + 
				location.x + "," + location.y + ")");
	}
	
	private void handleRadarEnergy(Player player) {
		int available = player.getEnergy();
		if (available < player.getRadarPower()) {
			if (available > 0) {
				if (Soar2D.logger.isLoggable(Level.FINER)) logger.finer(player.getName() + ": reducing radar power due to energy shortage");
				player.setRadarPower(available);
			} else {
				if (Soar2D.logger.isLoggable(Level.FINER)) logger.finer(player.getName() + ": radar switched off due to energy shortage");
				player.setRadarSwitch(false);
				return;
			}
		}
		player.adjustEnergy(player.getRadarPower() * -1, "radar");
	}
	
	private void spawnMissilePack(boolean force) {
		if (force || (Simulation.random.nextInt(100) < Soar2D.config.getMissilePackRespawnChance())) {
			// Get available spots
			ArrayList<Point> spots = getAvailableLocations(map);
			assert spots.size() > 0;
			
			// Add a missile pack to a spot
			Point spot = spots.get(Simulation.random.nextInt(spots.size()));
			logger.info("spawning missile pack at (" + spot.x + "," + spot.y + ")");
			boolean ret = map.addRandomObjectWithProperty(spot, Names.kPropertyMissiles);
			assert ret;
		}
	}
	
	private void chargeUp(Player player, Point location) {
		// Charge up
		ArrayList<CellObject> chargers = map.getAllWithProperty(location, Names.kPropertyCharger);
		Iterator<CellObject> iter = chargers.iterator();
		while (iter.hasNext()) {
			CellObject charger = iter.next();
			if (charger.hasProperty(Names.kPropertyHealth)) {
				player.setOnHealthCharger(true);
				if (player.getHealth() < Soar2D.config.getDefaultHealth()) {
					player.adjustHealth(charger.getIntProperty(Names.kPropertyHealth), "charger");
				}
			}
			if (charger.hasProperty(Names.kPropertyEnergy)) {
				player.setOnEnergyCharger(true);
				if (player.getEnergy() < Soar2D.config.getDefaultEnergy()) {
					player.adjustEnergy(charger.getIntProperty(Names.kPropertyEnergy), "charger");
				}
			}
		}
	}
	
	private void doMoveCollisions(Player player, MoveInfo playerMove, 
			HashMap<String, Point> newLocations, 
			HashMap<Point, ArrayList<Player> > collisionMap, 
			ArrayList<Player> movedTanks) {
		
		// Get destination location
		Point newLocation = newLocations.get(player.getName());
		
		// Wall collisions checked for earlier
		
		// is there a collision in the cell
		ArrayList<Player> collision = collisionMap.get(newLocation);
		if (collision != null) {
			
			// there is a collision

			// if there is only one player here, cancel its move
			if (collision.size() == 1) {
				Player other = collision.get(0);
				MoveInfo otherMove = lastMoves.get(other.getName());
				if (otherMove.move) {
					cancelMove(other, otherMove, newLocations, movedTanks);
					doMoveCollisions(other, otherMove, newLocations, collisionMap, movedTanks);
				}
			} 

			// If there is more than one guy here, they've already been cancelled
			
			
			// Add ourselves to this cell's collision list
			collision.add(player);
			collisionMap.put(newLocation, collision);
			
			// cancel my move
			if (playerMove.move) {
				cancelMove(player, playerMove, newLocations, movedTanks);
				doMoveCollisions(player, playerMove, newLocations, collisionMap, movedTanks);
			}
			return;

		}

		// There is nothing in this cell, create a new list and add ourselves
		collision = new ArrayList<Player>(4);
		collision.add(player);
		collisionMap.put(newLocation, collision);
	}
	
	private void cancelMove(Player player, MoveInfo move, 
			HashMap<String, Point> newLocations, 
			ArrayList<Player> movedTanks) {
		move.move = false;
		movedTanks.remove(player);
		newLocations.put(player.getName(), locations.get(player.getName()));
	}
	
	private MoveInfo getAndStoreMove(Player player) {
		MoveInfo move = player.getMove();
		if (Soar2D.control.isShuttingDown()) {
			return null;
		}
		assert move != null;
		String moveString = move.toString();
		if (moveString.length() > 0) {
			logger.info(player.getName() + ": " + moveString);
		}
		lastMoves.put(player.getName(), move);
		
		if (move.stopSim) {
			if (Soar2D.config.getTerminalAgentCommand()) {
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

	private ArrayList<ArrayList<Player>> findCollisions() {
		ArrayList<ArrayList<Player>> collisions = new ArrayList<ArrayList<Player>>(players.size() / 2);

		// Make sure collisions are possible
		if (players.size() < 2) {
			return collisions;
		}
		
		// Optimization to not check the same name twice
		HashSet<Player> colliding = new HashSet<Player>(players.size());
		ArrayList<Player> collision = new ArrayList<Player>(players.size());

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
						
						if (logger.isLoggable(Level.FINER)) logger.finer("collision at " + locations.get(left.getName()));
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

		return collisions;
	}
		
	private void handleEatersCollisions(ArrayList<ArrayList<Player>> collisions) {
		
		// if there are no total collisions, we're done
		if (collisions.size() < 1) {
			return;
		}

		ArrayList<Player> collision = new ArrayList<Player>(players.size());
		
		Iterator<ArrayList<Player>> collisionIter = collisions.iterator();
		while (collisionIter.hasNext()) {
			collision = collisionIter.next();

			assert collision.size() > 0;
			if (logger.isLoggable(Level.FINER)) logger.finer("Processing collision group with " + collision.size() + " collidees.");

			// Redistribute wealth
			int cash = 0;			
			ListIterator<Player> collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				cash += collideeIter.next().getPoints();
			}
			if (cash > 0) {
				int trash = cash % collision.size();
				cash /= collision.size();
				if (logger.isLoggable(Level.FINER)) logger.finer("Cash to each: " + cash + " (" + trash + " lost in division)");
				collideeIter = collision.listIterator();
				while (collideeIter.hasNext()) {
					collideeIter.next().setPoints(cash, "collision");
				}
			} else {
				if (logger.isLoggable(Level.FINER)) logger.finer("Sum of cash is negative.");
			}
			
			Point collisionLocation = locations.get(collision.get(0).getName());

			// Add the boom on the map
			map.setExplosion(collisionLocation);

			// Remove from former location (only one of these for all players)
			map.setPlayer(collisionLocation, null);
			
			// Move to new cell, consume food
			collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				Player player = collideeIter.next();
				Point location = putInStartingLocation(player);
				assert location != null;
				player.fragged();
				if (!lastMoves.get(player.getName()).dontEat) {
					eat(player, location);
				}
			}
		}
	}
	
	public void reset() {
		worldCount = 0;
		printedStats = false;
		missileID = 0;
		missileReset = 0;
		restartAfterUpdate = false;
		totalTime = 0;
		//System.out.println(map);
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
	
	public Point2D.Double getFloatLocation(Player player) {
		return floatLocations.get(player.getName());
	}

	public ArrayList<Player> getPlayers() {
		return players;
	}
	
	boolean isTerminal() {
		return printedStats;
	}

	public boolean recentlyMovedOrRotated(Player targetPlayer) {
		MoveInfo move = lastMoves.get(targetPlayer.getName());
		if (move == null) {
			return false;
		}
		return move.move || move.rotate;
	}

	public Player getPlayer(String playerName) {
		return playersMap.get(playerName);
	}

	public void interruped(String name) {
		if (Soar2D.wm.using()) {
			return;
		}
		if (players.size() <= 1) {
			return;
		}
		Iterator<Player> iter = players.iterator();
		Player thePlayer = null;
		Integer lowestScore = null;
		while (iter.hasNext()) {
			Player player = iter.next();
			if (player.getName().equals(name)) {
				thePlayer = player;
			} else {
				if (lowestScore == null) {
					lowestScore = new Integer(player.getPoints());
				} else {
					lowestScore = Math.min(lowestScore, player.getPoints());
				}
			}
			
		}
		if ((thePlayer == null) || (lowestScore == null)) {
			// shouldn't happen if name is valid
			return;
		}
		
		lowestScore -= 1;
		thePlayer.setPoints(lowestScore, "interrupted");
		this.stopAndDumpStats("interrupted", getSortedScores());
	}
	
	public double angleOff(Player player, Point2D.Double target) {
		Point2D.Double playerVector = new Point2D.Double();
		playerVector.x = getFloatLocation(player).x;
		playerVector.y = getFloatLocation(player).y;
		
		Point2D.Double targetVector = new Point2D.Double();
		targetVector.x = target.x;
		targetVector.y = target.y;
		
		// translate target so i'm the origin
		targetVector.x -= playerVector.x;
		targetVector.y -= playerVector.y;
		
		// make target unit vector
		double targetVectorLength = Math.sqrt(Math.pow(targetVector.x, 2) + Math.pow(targetVector.y, 2));
		assert targetVectorLength > 0;
		targetVector.x /= targetVectorLength;
		targetVector.y /= targetVectorLength;
		
		// make player facing vector
		playerVector.x = Math.cos(player.getHeadingRadians());
		playerVector.y = Math.sin(player.getHeadingRadians());
		
		double dotProduct = (targetVector.x * playerVector.x) + (targetVector.y * playerVector.y);
		double crossProduct = (targetVector.x * playerVector.y) - (targetVector.y * playerVector.x);
		
		// calculate inverse cosine of that for angle
		if (crossProduct < 0) {
			return Math.acos(dotProduct);
		}
		return Math.acos(dotProduct) * -1;
	}
}
