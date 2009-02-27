package org.msoar.gridmap2d.world;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Set;

import org.apache.log4j.Logger;
import org.msoar.gridmap2d.CognitiveArchitecture;
import org.msoar.gridmap2d.Direction;
import org.msoar.gridmap2d.Names;
import org.msoar.gridmap2d.Gridmap2D;
import org.msoar.gridmap2d.config.PlayerConfig;
import org.msoar.gridmap2d.map.CellObject;
import org.msoar.gridmap2d.map.EatersMap;
import org.msoar.gridmap2d.map.GridMap;
import org.msoar.gridmap2d.players.CommandInfo;
import org.msoar.gridmap2d.players.Eater;
import org.msoar.gridmap2d.players.EaterCommander;
import org.msoar.gridmap2d.players.Player;
import org.msoar.gridmap2d.players.scripted.ScriptedEater;


public class EatersWorld implements World {
	private static Logger logger = Logger.getLogger(EatersWorld.class);

	private EatersMap eatersMap;
	private PlayersManager<Eater> players = new PlayersManager<Eater>();
	private List<String> stopMessages = new ArrayList<String>();
	private CognitiveArchitecture cogArch;
	private boolean forceHuman = false;
	
	public EatersWorld(CognitiveArchitecture cogArch) throws Exception {
		this.cogArch = cogArch;
	}
	
	public void setMap(String mapPath) throws Exception {
		EatersMap oldMap = eatersMap;
		try {
			eatersMap = new EatersMap(mapPath, Gridmap2D.config.terminalsConfig().unopened_boxes, Gridmap2D.config.eatersConfig().low_probability, Gridmap2D.config.eatersConfig().high_probability);
		} catch (Exception e) {
			if (oldMap == null) {
				throw e;
			}
			eatersMap = oldMap;
			logger.error("Map load failed, restored old map.");
			return;
		}
		
		// This can throw a more fatal error.
		resetState();
	}
	
	public void reset() throws Exception {
		eatersMap.reset();
		resetState();
	}
	
	private void resetState() throws Exception {
		stopMessages.clear();
		resetPlayers();
	}
	
	private void resetPlayers() throws Exception {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (Eater eater : players.getAll()) {
			// find a suitable starting location
			int [] startingLocation = WorldUtil.getStartingLocation(eater, eatersMap, players.getInitialLocation(eater));
			players.setLocation(eater, startingLocation);

			// remove food from it
			eatersMap.getCell(startingLocation).removeAllByProperty(Names.kPropertyEdible);
			
			// put the player in it
			eatersMap.getCell(startingLocation).setPlayer(eater);

			eater.reset();
		}
		
		updatePlayers();
	}

	private void checkPointsRemaining() {
		if (Gridmap2D.config.terminalsConfig().points_remaining) {
			if (eatersMap.getScoreCount() <= 0) {
				stopMessages.add("There are no points remaining.");
			}
		}
	}
	
	private void checkFoodRemaining() {
		if (Gridmap2D.config.terminalsConfig().food_remaining) {
			if (eatersMap.getFoodCount() <= 0) {
				stopMessages.add("All the food is gone.");
			}
		}
	}
	
	private void checkUnopenedBoxes() {
		if (Gridmap2D.config.terminalsConfig().unopened_boxes) {
			if (eatersMap.getUnopenedBoxCount() <= 0) {
				stopMessages.add("All of the boxes are open.");
			}
		}
	}

	public void update(int worldCount) throws Exception {
		WorldUtil.checkNumPlayers(players.numberOfPlayers());

		// Collect input
		for (Eater eater : players.getAll()) {
			eater.resetPointsChanged();

			CommandInfo command = forceHuman ? Gridmap2D.control.getHumanCommand(eater) : eater.getCommand();
			if (command == null) {
				Gridmap2D.control.stopSimulation();
				return;
			}
			players.setCommand(eater, command);
			WorldUtil.checkStopSim(stopMessages, command, eater);
		}
		
		moveEaters();
		if (Gridmap2D.control.isShuttingDown()) {
			return;
		}
		
		updateMapAndEatFood();
		
		handleEatersCollisions(findCollisions(players));	
		updatePlayers();
		eatersMap.updateObjects();

		checkPointsRemaining();
		checkFoodRemaining();
		checkUnopenedBoxes();
		WorldUtil.checkMaxUpdates(stopMessages, worldCount);
		WorldUtil.checkWinningScore(stopMessages, players.getSortedScores());
		
		if (stopMessages.size() > 0) {
			boolean stopping = Gridmap2D.control.checkRunsTerminal();
			WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), stopping, stopMessages);

			if (stopping) {
				Gridmap2D.control.stopSimulation();
			} else {
				// reset and continue;
				reset();
				Gridmap2D.control.startSimulation(false, false);
			}
		}
	}
	
	private void updatePlayers() throws Exception {
		for (Eater eater : players.getAll()) {
			eater.update(players.getLocation(eater), eatersMap);
		}
	}

	private void moveEaters() {
		for (Eater eater : players.getAll()) {
			CommandInfo command = players.getCommand(eater);			

			if (!command.move) {
				continue;
			}

			// Calculate new location
			int [] oldLocation = players.getLocation(eater);
			int [] newLocation = org.msoar.gridmap2d.Arrays.copyOf(oldLocation, oldLocation.length);
			Direction.translate(newLocation, command.moveDirection);
			if (command.jump) {
				Direction.translate(newLocation, command.moveDirection);
			}
			
			// Verify legal move and commit move
			if (eatersMap.isInBounds(newLocation) && !eatersMap.getCell(newLocation).hasAnyWithProperty(Names.kPropertyBlock)) {
				// remove from cell
				eatersMap.getCell(oldLocation).setPlayer(null);
				
				if (command.jump) {
					eater.adjustPoints(Gridmap2D.config.eatersConfig().jump_penalty, "jump penalty");
				}
				players.setLocation(eater, newLocation);
				
			} else {
				eater.adjustPoints(Gridmap2D.config.eatersConfig().wall_penalty, "wall collision");
			}
		}
	}

	private void updateMapAndEatFood() {
		for (Eater eater : players.getAll()) {
			CommandInfo lastCommand = players.getCommand(eater);
			int [] location = players.getLocation(eater);
			
			if (lastCommand.move || lastCommand.jump) {
				eatersMap.getCell(location).setPlayer(eater);

				List<CellObject> moveApply = eatersMap.getCell(location).getAllWithProperty(Names.kPropertyMoveApply);
				if (moveApply != null) {
					for (CellObject object : moveApply) {
						if (apply(object, eater)) {
							eatersMap.getCell(location).removeObject(object.getName());
						}
					}
				}
			}
			
			if (!lastCommand.dontEat) {
				eat(eater, location);
			}
			
			if (lastCommand.open) {
				open(eater, location);
			}
		}
	}
	
	private boolean apply(CellObject object, Eater eater) {
		object.applyProperties();
		
		if (object.hasProperty("apply.points")) {
			int points = object.getIntProperty("apply.points", 0);
			eater.adjustPoints(points, object.getName());
		}
		if (object.getBooleanProperty("apply.reward", false)) {
			// am I the positive box
			if (object.getBooleanProperty("apply.reward.correct", false)) {
				// reward positively
				eater.adjustPoints(object.getIntProperty("apply.reward.positive", 0), "positive reward");
			} else {
				// I'm  not the positive box, set resetApply false
				object.removeProperty("apply.reset");
				
				// reward negatively
				eater.adjustPoints(-1 * object.getIntProperty("apply.reward.positive", 0), "negative reward (wrong box)");
			}
		}
		
		return object.getBooleanProperty("apply.remove", false);
	}
	
	private void open(Eater eater, int [] location) {
		List<CellObject> boxes = eatersMap.getCell(location).getAllWithProperty(Names.kPropertyBox);
		if (boxes == null) {
			logger.warn(eater.getName() + " tried to open but there is no box.");
		}

		// TODO: multiple boxes
		assert boxes.size() <= 1;
		
		CellObject box = boxes.get(0);
		if (box.hasProperty(Names.kPropertyStatus)) {
			if (box.getProperty(Names.kPropertyStatus).equalsIgnoreCase(Names.kOpen)) {
				logger.warn(eater.getName() + " tried to open an open box.");
			}
		}
		if (apply(box, eater)) {
			eatersMap.getCell(location).removeObject(box.getName());
		}
		checkResetApply(box);
	}

	private void checkResetApply(CellObject box) {
		if (box.getBooleanProperty("apply.reset", false)) {
			stopMessages.add(box.getName() + " called for reset.");
		}
	}
	
	private void eat(Eater eater, int [] location) {
		List<CellObject> list = eatersMap.getCell(location).getAllWithProperty(Names.kPropertyEdible);
		if (list != null) {
			for (CellObject food : list) {
				if (apply(food, eater)) {
					// if this returns true, it is consumed
					eatersMap.getCell(location).removeObject(food.getName());
				}
			}
		}			
	}
	
	private List<List<Eater>> findCollisions(PlayersManager<Eater> eaters) {
		List<List<Eater>> collisions = new ArrayList<List<Eater>>(players.numberOfPlayers() / 2);

		// Make sure collisions are possible
		if (players.numberOfPlayers() < 2) {
			return collisions;
		}
		
		// Optimization to not check the same name twice
		Set<Eater> colliding = new HashSet<Eater>(players.numberOfPlayers());
		List<Eater> collision = new ArrayList<Eater>(players.numberOfPlayers());

		ListIterator<Eater> leftIter = players.listIterator();
		while (leftIter.hasNext()) {
			Eater left = leftIter.next();
			
			// Check to see if we're already colliding
			if (colliding.contains(left)) {
				continue;
			}
			
			ListIterator<Eater> rightIter = players.listIterator(leftIter.nextIndex());
			// Clear collision list now
			collision.clear();
			while (rightIter.hasNext()) {
				// For each player to my right (I've already checked to my left)
				Eater right = rightIter.next();

				// Check to see if we're already colliding
				if (colliding.contains(right)) {
					continue;
				}
				
				// If the locations match, we have a collision
				if (players.getLocation(left).equals(players.getLocation(right))) {
					
					// Add to this set to avoid checking same player again
					colliding.add(left);
					colliding.add(right);
					
					// Add the left the first time a collision is detected
					if (collision.size() == 0) {
						collision.add(left);
						
						logger.debug("collision at " + players.getLocation(left));
					}
					// Add each right as it is detected
					collision.add(right);
				}
			}
			
			// Add the collision to the total collisions if there is one
			if (collision.size() > 0) {
				collisions.add(new ArrayList<Eater>(collision));
			}
		}

		return collisions;
	}
		
	private void handleEatersCollisions(List<List<Eater>> collisions) throws Exception {
		
		// if there are no total collisions, we're done
		if (collisions.size() < 1) {
			return;
		}

		List<Eater> collision = new ArrayList<Eater>(players.numberOfPlayers());
		
		Iterator<List<Eater>> collisionIter = collisions.iterator();
		while (collisionIter.hasNext()) {
			collision = collisionIter.next();

			assert collision.size() > 0;
			logger.debug("Processing collision group with " + collision.size() + " collidees.");

			// Redistribute wealth
			int cash = 0;			
			ListIterator<Eater> collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				cash += collideeIter.next().getPoints();
			}
			if (cash > 0) {
				int trash = cash % collision.size();
				cash /= collision.size();
				logger.debug("Cash to each: " + cash + " (" + trash + " lost in division)");
				collideeIter = collision.listIterator();
				while (collideeIter.hasNext()) {
					collideeIter.next().setPoints(cash, "collision");
				}
			} else {
				logger.debug("Sum of cash is negative.");
			}
			
			int [] collisionLocation = players.getLocation(collision.get(0));

			// Add the boom on the map
			setExplosion(collisionLocation);

			// Remove from former location (only one of these for all players)
			eatersMap.getCell(collisionLocation).setPlayer(null);
			
			// Move to new cell, consume food
			collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				Eater eater = collideeIter.next();
				int [] location = WorldUtil.getStartingLocation(eater, eatersMap, null);
				players.setLocation(eater, location);

				// put the player in it
				eatersMap.getCell(location).setPlayer(eater);
				
				eater.setFragged(true);
				if (!players.getCommand(eater).dontEat) {
					eat(eater, location);
				}
			}
		}
	}

	private void setExplosion(int[] xy) {
		CellObject explosion = eatersMap.createObjectByName(Names.kExplosion);
		explosion.setIntProperty("update.linger", 2);
		eatersMap.getCell(xy).addObject(explosion);
	}
	

	public boolean isTerminal() {
		return stopMessages.size() > 0;
	}

	public void removePlayer(String name) throws Exception {
		Eater eater = players.get(name);
		eatersMap.getCell(players.getLocation(eater)).setPlayer(null);
		players.remove(eater);
		eater.shutdownCommander();
		updatePlayers();
	}
	
	public void interrupted(String agentName) throws Exception {
		players.interrupted(agentName);
		stopMessages.add("interrupted");
	}

	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	public void addPlayer(String playerId, PlayerConfig playerConfig, boolean debug) throws Exception {
		
		Eater eater = new Eater(playerId);

		players.add(eater, eatersMap, playerConfig.pos);
		
		if (playerConfig.productions != null) {
			EaterCommander eaterCommander = cogArch.createEaterCommander(eater, playerConfig.productions, Gridmap2D.config.eatersConfig().vision, playerConfig.shutdown_commands, eatersMap.getMetadataFile(), debug);
			eater.setCommander(eaterCommander);
		} else if (playerConfig.script != null) {
			eater.setCommander(new ScriptedEater(CommandInfo.loadScript(playerConfig.script)));
		}

		int [] location = WorldUtil.getStartingLocation(eater, eatersMap, players.getInitialLocation(eater));
		players.setLocation(eater, location);
		
		// remove food from it
		eatersMap.getCell(location).removeAllByProperty(Names.kPropertyEdible);

		// put the player in it
		eatersMap.getCell(location).setPlayer(eater);
		
		logger.info(eater.getName() + ": Spawning at (" + location[0] + "," + location[1] + ")");
		
		updatePlayers();
	}

	public GridMap getMap() {
		return eatersMap;
	}

	public Player[] getPlayers() {
		return players.getAllAsPlayers();
	}

	public void setForceHumanInput(boolean setting) {
		forceHuman = setting;
	}
}
