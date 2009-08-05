package edu.umich.soar.gridmap2d.world;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Set;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.CognitiveArchitecture;
import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.EatersMap;
import edu.umich.soar.gridmap2d.map.GridMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Eater;
import edu.umich.soar.gridmap2d.players.EaterCommander;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.scripted.ScriptedEater;


public class EatersWorld implements World {
	private static Logger logger = Logger.getLogger(EatersWorld.class);

	private EatersMap map;
	private final PlayersManager<Eater> players = new PlayersManager<Eater>();
	private final List<String> stopMessages = new ArrayList<String>();
	private final CognitiveArchitecture cogArch;
	private boolean forceHuman = false;
	
	public EatersWorld(CognitiveArchitecture cogArch) {
		this.cogArch = cogArch;
	}
	
	public void setAndResetMap(String mapPath) {
		EatersMap newMap = EatersMap.generateInstance(mapPath, Gridmap2D.config.terminalsConfig().unopened_boxes, Gridmap2D.config.eatersConfig().low_probability, Gridmap2D.config.eatersConfig().high_probability); 
		if (newMap == null) {
			return;
		}
		map = newMap;
		resetState();
	}
	
	public void reset() {
		map.reset();
		resetState();
	}
	
	private void resetState() {
		stopMessages.clear();
		resetPlayers();
	}
	
	/**
	 * @throws IllegalStateException If there are now empty locations available to respawn players
	 */
	private void resetPlayers() {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (Eater eater : players.getAll()) {
			// find a suitable starting location
			int [] location = WorldUtil.getStartingLocation(map, players.getInitialLocation(eater));
			if (location == null) {
				throw new IllegalStateException("no empty locations available for spawn");
			}
			players.setLocation(eater, location);

			// remove food from it
			map.getCell(location).removeAllByProperty(Names.kPropertyEdible);
			
			// put the player in it
			map.getCell(location).setPlayer(eater);

			eater.reset();
		}
		
		updatePlayers();
	}

	private void checkPointsRemaining() {
		if (Gridmap2D.config.terminalsConfig().points_remaining) {
			if (map.getScoreCount() <= 0) {
				stopMessages.add("There are no points remaining.");
			}
		}
	}
	
	private void checkFoodRemaining() {
		if (Gridmap2D.config.terminalsConfig().food_remaining) {
			if (map.getFoodCount() <= 0) {
				stopMessages.add("All the food is gone.");
			}
		}
	}
	
	private void checkUnopenedBoxes() {
		if (Gridmap2D.config.terminalsConfig().unopened_boxes) {
			if (map.getUnopenedBoxCount() <= 0) {
				stopMessages.add("All of the boxes are open.");
			}
		}
	}

	public void update(int worldCount) {
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
		map.updateObjects();

		checkPointsRemaining();
		checkFoodRemaining();
		checkUnopenedBoxes();
		WorldUtil.checkMaxUpdates(stopMessages, worldCount);
		WorldUtil.checkWinningScore(stopMessages, players.getSortedScores());
		
		if (stopMessages.size() > 0) {
			Gridmap2D.control.stopSimulation();
			boolean stopping = Gridmap2D.control.getRunsTerminal() <= 0;
			WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), stopping, stopMessages);
		}
	}
	
	private void updatePlayers() {
		for (Eater eater : players.getAll()) {
			eater.update(players.getLocation(eater), map);
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
			int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);
			Direction.translate(newLocation, command.moveDirection);
			if (command.jump) {
				Direction.translate(newLocation, command.moveDirection);
			}
			
			// Verify legal move and commit move
			if (map.isInBounds(newLocation) && !map.getCell(newLocation).hasAnyWithProperty(Names.kPropertyBlock)) {
				// remove from cell
				map.getCell(oldLocation).setPlayer(null);
				
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
				map.getCell(location).setPlayer(eater);

				List<CellObject> moveApply = map.getCell(location).getAllWithProperty(Names.kPropertyMoveApply);
				if (moveApply != null) {
					for (CellObject object : moveApply) {
						if (apply(object, eater)) {
							map.getCell(location).removeObject(object.getName());
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
		List<CellObject> boxes = map.getCell(location).getAllWithProperty(Names.kPropertyBox);
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
			map.getCell(location).removeObject(box.getName());
		}
		checkResetApply(box);
	}

	private void checkResetApply(CellObject box) {
		if (box.getBooleanProperty("apply.reset", false)) {
			stopMessages.add(box.getName() + " called for reset.");
		}
	}
	
	private void eat(Eater eater, int [] location) {
		List<CellObject> list = map.getCell(location).getAllWithProperty(Names.kPropertyEdible);
		if (list != null) {
			for (CellObject food : list) {
				if (apply(food, eater)) {
					// if this returns true, it is consumed
					map.getCell(location).removeObject(food.getName());
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
		
	/**
	 * @param collisions
	 * @throws IllegalStateException If there are no available locations to spawn the eaters.
	 */
	private void handleEatersCollisions(List<List<Eater>> collisions) {
		
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
			map.getCell(collisionLocation).setPlayer(null);
			
			// Move to new cell, consume food
			collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				Eater eater = collideeIter.next();
				
				int [] location = WorldUtil.getStartingLocation(map, null);
				if (location == null) {
					throw new IllegalStateException("no empty locations available for spawn");
				}
				
				players.setLocation(eater, location);

				// put the player in it
				map.getCell(location).setPlayer(eater);
				
				eater.setFragged(true);
				if (!players.getCommand(eater).dontEat) {
					eat(eater, location);
				}
			}
		}
	}

	private void setExplosion(int[] xy) {
		CellObject explosion = map.createObjectByName(Names.kExplosion);
		explosion.setIntProperty("update.linger", 2);
		map.getCell(xy).addObject(explosion);
	}
	

	public boolean isTerminal() {
		return stopMessages.size() > 0;
	}

	public void removePlayer(String name) {
		Eater eater = players.get(name);
		map.getCell(players.getLocation(eater)).setPlayer(null);
		players.remove(eater);
		eater.shutdownCommander();
		updatePlayers();
	}
	
	public void interrupted(String agentName) {
		players.interrupted(agentName);
		stopMessages.add("interrupted");
	}

	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	@Override
	public boolean hasPlayer(String name) {
		return players.get(name) != null;
	}
	
	@Override
	public boolean addPlayer(String id, PlayerConfig cfg, boolean debug) {
		int [] location = WorldUtil.getStartingLocation(map, cfg.pos);
		if (location == null) {
			Gridmap2D.control.errorPopUp("There are no suitable starting locations.");
			return false;
		}
		
		List<CommandInfo> script = null;
		if (cfg.script != null) {
			try {
				script = CommandInfo.loadScript(cfg.script);
			} catch (IOException e) {
				Gridmap2D.control.errorPopUp("IOException loading script " + cfg.script);
				return false;
			}
		}
		
		Eater player = new Eater(id);  
		players.add(player, cfg.pos);
		
		if (cfg.productions != null) {
			EaterCommander cmdr = cogArch.createEaterCommander(player, cfg.productions, Gridmap2D.config.eatersConfig().vision, cfg.shutdown_commands, map.getMetadataFile(), debug);
			if (cmdr == null) {
				players.remove(player);
				return false;
			}
			player.setCommander(cmdr);
		} else if (cfg.script != null) {
			assert script != null;
			player.setCommander(new ScriptedEater(script));
		}

		players.setLocation(player, location);
		
		// remove food from it
		map.getCell(location).removeAllByProperty(Names.kPropertyEdible);

		// put the player in it
		map.getCell(location).setPlayer(player);
		
		logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + ")");
		
		updatePlayers();
		return true;
	}

	public GridMap getMap() {
		return map;
	}

	public Player[] getPlayers() {
		return players.getAllAsPlayers();
	}

	public void setForceHumanInput(boolean setting) {
		forceHuman = setting;
	}
}
