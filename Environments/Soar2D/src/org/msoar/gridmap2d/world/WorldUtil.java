package org.msoar.gridmap2d.world;

import java.util.List;

import org.apache.log4j.Logger;
import org.msoar.gridmap2d.Gridmap2D;
import org.msoar.gridmap2d.map.GridMap;
import org.msoar.gridmap2d.players.CommandInfo;
import org.msoar.gridmap2d.players.Player;


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
	
	static void checkNumPlayers(int numPlayers) throws Exception {
		if (numPlayers == 0) {
			Gridmap2D.control.stopSimulation();
			throw new Exception("Update called with no players.");
		}
	}
	
	public static int [] getStartingLocation(Player player, GridMap map, int[] initialLocation) throws Exception {
		int[] location = null;
		if (initialLocation != null) {
			location = org.msoar.gridmap2d.Arrays.copyOf(initialLocation, initialLocation.length);
			if (!map.isAvailable(location)) {
				logger.warn(player.getName() + ": Initial location (" + location[0] + "," + location[1] + ") is blocked, going random.");
				location = null;
			}
		}
		
		if (location == null) {
			location = map.getAvailableLocationAmortized();
			if (location == null) {
				throw new Exception("There are no suitable starting locations for " + player.getName() + ".");
			}
		}
		return location;
	}

}
