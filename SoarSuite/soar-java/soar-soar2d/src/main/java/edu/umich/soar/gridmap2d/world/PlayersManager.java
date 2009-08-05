package edu.umich.soar.gridmap2d.world;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;


public class PlayersManager<P extends Player> {
	private static Logger logger = Logger.getLogger(PlayersManager.class);

	private List<P> players = new ArrayList<P>(7);
	private Map<String, P> playersMap = new HashMap<String, P>(7);
	private Map<P, int []> initialLocations = new HashMap<P, int []>(7);
	private Map<P, int []> locations = new HashMap<P, int []>(7);
	private Map<P, CommandInfo> lastCommands = new HashMap<P, CommandInfo>(7);
	
	public int numberOfPlayers() {
		return players.size();
	}

	List<P> getAll() {
		return players;
	}
	
	Player[] getAllAsPlayers() {
		return players.toArray(new Player[0]);
	}
	
	ListIterator<P> listIterator() {
		return players.listIterator();
	}
	
	ListIterator<P> listIterator(int index) {
		return players.listIterator(index);
	}
	
	public int [] getLocation(P player) {
		return locations.get(player);
	}
	
	void setLocation(P player, int [] location) {
		locations.put(player, location);
	}
	
	public CommandInfo getCommand(P player) {
		return lastCommands.get(player);
	}
	
	void setCommand(P player, CommandInfo move) {
		lastCommands.put(player, move);

		String moveString = move.toString();
		if (moveString.length() > 0) {
			logger.info(player.getName() + ": " + moveString);
		}
	}
	
	P get(String name) {
		return playersMap.get(name);
	}
	
	P get(int index) {
		return players.get(index);
	}
	
	int indexOf(P player) {
		return players.indexOf(player);
	}
	
	void remove(P player) {
		logger.info("Removing player " + player);
		players.remove(player);
		playersMap.remove(player.getName());
		initialLocations.remove(player);
		locations.remove(player);
		lastCommands.remove(player);
	}
	
	/**
	 * @param player The player to add, the player's name must be unique.
	 * @param initialLocation The player's starting location.
	 * 
	 * @throws IllegalStateException If the player name is already in use.
	 */
	void add(P player, int [] initialLocation) {
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
	
	boolean hasInitialLocation(P player) {
		return initialLocations.containsKey(player);
	}

	int [] getInitialLocation(P player) {
		return initialLocations.get(player);
	}
	
	boolean exists(String name) {
		return playersMap.containsKey(name);
	}
	
	int size() {
		return players.size();
	}
	
	int[] getSortedScores() {
		int[] scores = new int[players.size()];
		
		for (int i = 0; i < players.size(); ++i) {
			scores[i] = players.get(i).getPoints();
		}
		Arrays.sort(scores);
		return scores;
	}
	
	void interrupted(String interruptedName) {
		P interruptedPlayer = get(interruptedName);
		if (numberOfPlayers() <= 1) {
			return;
		}

		// set the player to the lowest score - 1
		Integer lowestScore = null;
		for (P player : players) {
			if (!player.getName().equals(interruptedName)) {
				if (lowestScore == null) {
					lowestScore = new Integer(player.getPoints());
				} else {
					lowestScore = Math.min(lowestScore, player.getPoints());
				}
			}
		}
		
		lowestScore -= 1;
		interruptedPlayer.setPoints(lowestScore, "interrupted");
	}
}
