package edu.umich.soar.gridmap2d.world;

import java.util.Arrays;
import java.util.List;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.map.GridMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;


public class WorldUtil {
	private static Logger logger = Logger.getLogger(WorldUtil.class);

	static void dumpStats(int[] sortedScores, Player[] players, boolean stopping, List<String> messages) {
		StringBuilder bigMessage = new StringBuilder(); 
		for (String message : messages) {
			System.out.println(message);
			logger.info(message);
			if (stopping) {
				bigMessage.append(message);
				bigMessage.append("\n");
			}
		}
		if (stopping) {
			Gridmap2D.control.infoPopUp(bigMessage.toString());
		}
		
		boolean draw = false;
		if (sortedScores.length > 1) {
			if (sortedScores[sortedScores.length - 1] ==  sortedScores[sortedScores.length - 2]) {
				logger.debug("Draw detected.");
				draw = true;
			}
		}
		
		for (Player player : players) {
			String status = null;
			if (player.getPoints() == sortedScores[sortedScores.length - 1]) {
				status = draw ? "draw" : "winner";
			} else {
				status = "loser";
			}
			String statline = player.getName() + ": " + player.getPoints() + " (" + status + ")";
			logger.info(statline);
			System.out.println(statline);
		}
	}

	static void checkMaxUpdates(List<String> stopMessages, int worldCount) {
		if (Gridmap2D.config.terminalsConfig().max_updates > 0) {
			if (worldCount >= Gridmap2D.config.terminalsConfig().max_updates) {
				stopMessages.add("Reached maximum updates, stopping.");
			}
		}
	}
	
	static void checkStopSim(List<String> stopMessages, CommandInfo command, Player player) {
		if (command.stopSim) {
			if (Gridmap2D.config.terminalsConfig().agent_command) {
				stopMessages.add(player.getName() + " issued simulation stop command.");
			} else {
				logger.warn(player.getName() + " issued illegal simulation stop command.");
			}
		}
	}

	static void checkWinningScore(List<String> stopMessages, int[] scores) {
		if (Gridmap2D.config.terminalsConfig().winning_score > 0) {
			if (scores[scores.length - 1] >= Gridmap2D.config.terminalsConfig().winning_score) {
				stopMessages.add("At least one player has achieved at least " + Gridmap2D.config.terminalsConfig().winning_score + " points.");
			}
		}
	}
	
	static void checkNumPlayers(int numPlayers) {
		if (numPlayers == 0) {
			Gridmap2D.control.stopSimulation();
			Gridmap2D.control.errorPopUp("Update called with no players.");
		}
	}
	
	/**
	 * @param map The map to search.
	 * @param initialLocation The desired starting location.
	 * @return The starting location or null if there are none available.
	 */
	public static int [] getStartingLocation(GridMap map, int[] initialLocation) {
		int[] location = null;
		if (initialLocation != null) {
			location = Arrays.copyOf(initialLocation, initialLocation.length);
			if (!map.isAvailable(location)) {
				logger.warn("Initial location (" + location[0] + "," + location[1] + ") is blocked, going random.");
				location = null;
			}
		}
		
		if (location == null) {
			location = map.getAvailableLocationAmortized();
		}
		return location;
	}

}
