package edu.umich.soar.room.map;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


public class PlayersManager {
	private static Log logger = LogFactory.getLog(PlayersManager.class);

	private final List<Robot> players = new ArrayList<Robot>(7);
	private final Map<String, Robot> playersMap = new HashMap<String, Robot>(7);
	private final Map<Robot, int []> initialLocations = new HashMap<Robot, int []>(7);
	private final Map<Robot, RobotCommand> lastCommands = new HashMap<Robot, RobotCommand>(7);
	
	public int numberOfPlayers() {
		return players.size();
	}

	List<Robot> getAll() {
		return new ArrayList<Robot>(players);
	}
	
	ListIterator<Robot> listIterator() {
		return players.listIterator();
	}
	
	ListIterator<Robot> listIterator(int index) {
		return players.listIterator(index);
	}
	
	public RobotCommand getCommand(Robot player) {
		return lastCommands.get(player);
	}
	
	void setCommand(Robot player, RobotCommand move) {
		lastCommands.put(player, move);

		String moveString = move.toString();
		if (moveString.length() > 0) {
			logger.info(player.getName() + ": " + moveString);
		}
	}
	
	Robot get(String name) {
		return playersMap.get(name);
	}
	
	Robot get(int index) {
		return players.get(index);
	}
	
	int indexOf(Robot player) {
		return players.indexOf(player);
	}
	
	void remove(Robot player) {
		logger.info("Removing player " + player);
		players.remove(player);
		playersMap.remove(player.getName());
		initialLocations.remove(player);
		lastCommands.remove(player);
	}
	
	/**
	 * @param player The player to add, the player's name must be unique.
	 * @param initialLocation The player's starting location.
	 * 
	 * @throws IllegalStateException If the player name is already in use.
	 */
	void add(Robot player, int [] initialLocation) {
		if(playersMap.containsKey(player.getName())) {
			throw new IllegalStateException(player.getName() + " already exists");
		}

		logger.info("Adding player " + player);
		players.add(player);
		playersMap.put(player.getName(), player);
		
		if (initialLocation != null) {
			initialLocations.put(player, Arrays.copyOf(initialLocation, initialLocation.length));
		}
	}
	
	boolean hasInitialLocation(Robot player) {
		return initialLocations.containsKey(player);
	}

	int [] getInitialLocation(Robot player) {
		return initialLocations.get(player);
	}
	
	boolean exists(String name) {
		return playersMap.containsKey(name);
	}
	
	int size() {
		return players.size();
	}
	
}
