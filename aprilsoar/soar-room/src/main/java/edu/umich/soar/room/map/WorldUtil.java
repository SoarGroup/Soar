package edu.umich.soar.room.map;

import java.util.Arrays;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.room.core.Simulation;

public class WorldUtil {
	private static Log logger = LogFactory.getLog(WorldUtil.class);

	// TODO: reimplement using some kind of event
//	static void dumpStats(Simulation sim, int[] sortedScores, Player[] players, boolean stopping, List<String> messages) {
//		StringBuilder bigMessage = new StringBuilder(); 
//		for (String message : messages) {
//			System.out.println(message);
//			logger.info(message);
//			if (stopping) {
//				bigMessage.append(message);
//				bigMessage.append("\n");
//			}
//		}
//		if (stopping) {
//			sim.info(bigMessage.toString());
//		}
//		
//		boolean draw = false;
//		if (sortedScores.length > 1) {
//			if (sortedScores[sortedScores.length - 1] ==  sortedScores[sortedScores.length - 2]) {
//				logger.debug("Draw detected.");
//				draw = true;
//			}
//		}
//		
//		for (Player player : players) {
//			String status = null;
//			if (player.getPoints() == sortedScores[sortedScores.length - 1]) {
//				status = draw ? "draw" : "winner";
//			} else {
//				status = "loser";
//			}
//			String statline = player.getName() + ": " + player.getPoints() + " (" + status + ")";
//			logger.info(statline);
//			System.out.println(statline);
//		}
//	}

	static void checkMaxUpdates(Simulation sim, List<String> stopMessages, int worldCount) {
		if (sim.getConfig().terminalsConfig().max_updates > 0) {
			if (worldCount >= sim.getConfig().terminalsConfig().max_updates) {
				stopMessages.add("Reached maximum updates, stopping.");
			}
		}
	}
	
	static void checkStopSim(Simulation sim, List<String> stopMessages, boolean stopSim, Robot player) {
		if (stopSim) {
			if (sim.getConfig().terminalsConfig().agent_command) {
				stopMessages.add(player.getName() + " issued simulation stop command.");
			} else {
				logger.warn(player.getName() + " issued illegal simulation stop command.");
			}
		}
	}

	static void checkWinningScore(Simulation sim, List<String> stopMessages, int[] scores) {
		if (sim.getConfig().terminalsConfig().winning_score > 0) {
			if (scores[scores.length - 1] >= sim.getConfig().terminalsConfig().winning_score) {
				stopMessages.add("At least one player has achieved at least " + sim.getConfig().terminalsConfig().winning_score + " points.");
			}
		}
	}
	
	static void checkNumPlayers(Simulation sim, int numPlayers) {
		if (numPlayers == 0) {
			sim.stop();
			sim.error("Update called with no players.");
		}
	}
	
	/**
	 * @param map The map to search.
	 * @param initialLocation The desired starting location.
	 * @return The starting location or null if there are none available.
	 */
	public static int [] getStartingLocation(RoomMap map, int[] initialLocation) {
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
