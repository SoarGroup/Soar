package soar2d.world;

import java.lang.Math;
import java.util.Arrays;
import java.util.Iterator;

import org.apache.log4j.Logger;

import soar2d.Soar2D;
import soar2d.config.SimConfig;
import soar2d.map.EatersMap;
import soar2d.map.GridMap;
import soar2d.map.TaxiMap;
import soar2d.players.MoveInfo;
import soar2d.players.Player;

public class World {
	private static Logger logger = Logger.getLogger(World.class);

	private IWorld worldModule;
	
	private PlayersManager players = new PlayersManager();
	
	private int worldCount = 0;
	private boolean printedStats = false;
	
	private GridMap map;
	public GridMap getMap() {
		return map;
	}
	
	private void error(String message) {
		logger.error(message);
		Soar2D.control.errorPopUp(message);
	}

	public boolean load() {
		if (worldModule == null) {
			switch(Soar2D.config.game()) {
			case TANKSOAR:
				worldModule = new TankSoarWorld();
				break;
			case EATERS:
				worldModule = new EatersWorld();
				break;
			case ROOM:
				worldModule = new BookWorld();
				break;
			case KITCHEN:
				worldModule = new KitchenWorld();
				break;
			case TAXI:
				worldModule = new TaxiWorld();
				break;

			}
		}
		
		Soar2D.control.setRunsTerminal(Soar2D.config.terminalsConfig().max_runs);
		return loadInternal(false);
	}

	private boolean loadInternal(boolean resetDuringRun) {
		GridMap newMap = worldModule.newMap();
		
		try {
			newMap.load();
		} catch (GridMap.LoadError e) {
			error(e.getMessage());
			return false;
		}
		
		if (worldModule.postLoad(newMap) == false) {
			return false;
		}
		
		
		map = newMap;
		
		reset(map);
		Soar2D.control.resetTime();
		resetPlayers(resetDuringRun);
		
		logger.info(Soar2D.config.generalConfig().map + " loaded, world reset.");
		if (map.getOpenCode() != 0) {
			logger.info("The correct open code is: " + map.getOpenCode());
		}
		return true;
	}
	
	void resetPlayers(boolean resetDuringRun) {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			// for each player
			Player player = iter.next();
			
			resetPlayer(player, resetDuringRun);
		}
		
		worldModule.updatePlayers(false, map, players);
	}
	
	private boolean resetPlayer(Player player, boolean resetDuringRun) {
		// find a suitable starting location
		int [] startingLocation = putInStartingLocation(player, true);
		if (startingLocation == null) {
			return false;
		}
		
		worldModule.resetPlayer(map, player, players, resetDuringRun);
		return true;
	}
	
	public void shutdown() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Soar2D.simulation.destroyPlayer(iter.next());
			iter = players.iterator();
		}
		assert this.players.numberOfPlayers() == 0;
		map.shutdown();
	}
	
	public void removePlayer(String name) {
		logger.info("Removing player " + name);
		Player player = players.get(name);
		if (player == null) {
			logger.warn("destroyPlayer: Couldn't find player name match for " + name + ", ignoring.");
			return;
		}
		map.setPlayer(players.getLocation(player), null);
		players.remove(player);
		worldModule.updatePlayers(true, map, players);
		
	}
	
	int [] putInStartingLocation(Player player, boolean useInitialLocation) {
		int[] location = null;
		if (useInitialLocation && players.hasInitialLocation(player)) {
			location = players.getInitialLocation(player);
			if (!map.isAvailable(location)) {
				logger.warn(player.getName() + ": Initial location (" + location[0] + "," + location[1] + ") is blocked, going random.");
				location = null;
			}
		}
		
		if (location == null) {
			location = map.getAvailableLocationAmortized();
			if (location == null) {
				error("There are no suitable starting locations for " + player.getName() + ".");
				return null;
			}
		}
		
		// put the player in it
		map.setPlayer(location, player);
		players.setLocation(player, location);
		worldModule.putInStartingLocation(player, map, players, location);
		return location;
	}

	public boolean addPlayer(Player player, int [] suggestedInitialLocation, boolean human) {
		assert players.exists(player) == false;
		logger.info("Adding player " + player);

		players.add(player, suggestedInitialLocation, human);

		// find a suitable starting location (either by using suggestion or random).
		int [] startingLocation = putInStartingLocation(player, true);
		if (startingLocation == null) {
			return false;
		}
		
		int [] location = players.getLocation(player);
		assert location != null;
		printSpawnMessage(player, location);
		
		worldModule.updatePlayers(true, map, players);
		return true;
	}
	
	void printSpawnMessage(Player player, int [] location) {
		if (Soar2D.config.game() == SimConfig.Game.TANKSOAR) {
			logger.info(player.getName() + ": Spawning at (" + 
					location[0] + "," + location[1] + "), facing " + player.getFacing().id());
		} else {
			logger.info(player.getName() + ": Spawning at (" + 
					location[0] + "," + location[1] + ")");
		}
	}
	
	void stopAndDumpStats(String message) {
		int[] scores = getSortedScores();
		
		if (!printedStats) {
			printedStats = true;
			System.out.println(message);
			Soar2D.control.infoPopUp(message);
			Soar2D.control.stopSimulation();
			boolean draw = false;
			if (scores.length > 1) {
				if (scores[scores.length - 1] ==  scores[scores.length - 2]) {
					logger.debug("Draw detected.");
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
				String statline = player.getName() + ": " + player.getPoints() + " (" + status + ")";
				logger.info(statline);
				System.out.println(statline);
			}
		}
	}
	
	private int[] getSortedScores() {
		int[] scores = new int[players.numberOfPlayers()];
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
		
		boolean restartAfterUpdate = false;
		
		Soar2D.config.generalConfig().hidemap = false;
		
		// Collect human input
		Iterator<Player> humanPlayerIter = players.humanIterator();
		if (Soar2D.config.generalConfig().force_human) {
			humanPlayerIter = players.iterator();
		} else {
			humanPlayerIter = players.humanIterator();
		}
		while (humanPlayerIter.hasNext()) {
			Player human = humanPlayerIter.next();
			if (!human.getHumanMove()) {
				return;
			}
		}
		
		++worldCount;

		if (Soar2D.config.terminalsConfig().max_updates > 0) {
			if (worldCount >= Soar2D.config.terminalsConfig().max_updates) {
				if (Soar2D.control.checkRunsTerminal()) {
					stopAndDumpStats("Reached maximum updates, stopping.");
					return;
				} else {
					restartAfterUpdate = true;
				}
			}
		}
		
		if (Soar2D.config.terminalsConfig().winning_score > 0) {
			int[] scores = getSortedScores();
			if (scores[scores.length - 1] >= Soar2D.config.terminalsConfig().winning_score) {
				if (Soar2D.control.checkRunsTerminal()) {
					stopAndDumpStats("At least one player has achieved at least " + Soar2D.config.terminalsConfig().winning_score + " points.");
					return;
				} else {
					restartAfterUpdate = true;
				}
			}
		}
		
		if (Soar2D.config.game() == SimConfig.Game.EATERS) {
			EatersMap eMap = (EatersMap)map;
			if (Soar2D.config.terminalsConfig().points_remaining) {
				if (eMap.getScoreCount() <= 0) {
					stopAndDumpStats("There are no points remaining.");
					return;
				}
			}
		
			if (Soar2D.config.terminalsConfig().food_remaining) {
				if (eMap.getFoodCount() <= 0) {
					
					if (Soar2D.control.checkRunsTerminal()) {
						stopAndDumpStats("All of the food is gone.");
						return;
						
					} else {
						restartAfterUpdate = true;
					}
				}
			}

			if (Soar2D.config.terminalsConfig().unopened_boxes) {
				if (eMap.getUnopenedBoxCount() <= 0) {
					stopAndDumpStats("All of the boxes are open.");
					return;
				}
			}
		}

		if (players.numberOfPlayers() == 0) {
			logger.warn("Update called with no players.");
			Soar2D.control.stopSimulation();
			return;
		}
		
		// get moves
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			
			MoveInfo move = player.getMove();
			if (Soar2D.control.isShuttingDown()) {
				return;
			}
			
			assert move != null;
			String moveString = move.toString();
			if (moveString.length() > 0) logger.info(player.getName() + ": " + moveString);

			players.setMove(player, move);
			
			if (move.stopSim) {
				if (Soar2D.config.terminalsConfig().agent_command) {
					stopAndDumpStats(player.getName() + " issued simulation stop command.");
				} else {
					logger.warn(player.getName() + " issued ignored stop command.");
				}
			}
		}
		
		if (worldModule.update(map, players)) {
			restartAfterUpdate = true;
		}

		if (Soar2D.config.game() == SimConfig.Game.TAXI) {
			TaxiMap xMap = (TaxiMap)map;
			if (Soar2D.config.terminalsConfig().passenger_delivered) {
				if (stopNextCyclePassengerDelivered) {
					this.stopNextCyclePassengerDelivered = false;
					stopAndDumpStats("The passenger has been delivered.");
					return;
				} else if (xMap.isPassengerDelivered()) {
					this.stopNextCyclePassengerDelivered = true;
				}
			}

			if (Soar2D.config.terminalsConfig().passenger_pick_up) {
				if (stopNextCyclePassengerPickUp) {
					this.stopNextCyclePassengerPickUp = false;
					stopAndDumpStats("The passenger has been picked up.");
					return;
				} else if (xMap.isPassengerCarried()) {
					this.stopNextCyclePassengerPickUp = true;
				}
			}
			
			if (Soar2D.config.terminalsConfig().fuel_remaining) {
				if (stopNextCycleFuelRemaining) {
					this.stopNextCycleFuelRemaining = false;
					stopAndDumpStats("The taxi has run out of fuel.");
					return;
				} else if (xMap.isFuelNegative()) {
					this.stopNextCycleFuelRemaining = true;
				}
			}
		}

		if (restartAfterUpdate) {
			int[] scores = getSortedScores();
			boolean draw = false;
			if (scores.length > 1) {
				if (scores[scores.length - 1] ==  scores[scores.length - 2]) {
					logger.debug("Draw detected.");
					draw = true;
				}
			}
			
			iter = players.iterator();
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
			
			loadInternal(true);
			if (Soar2D.wm.using()) {
				Soar2D.wm.reset();
			}
		}
	}
	
	private boolean stopNextCyclePassengerPickUp = false;
	private boolean stopNextCycleFuelRemaining = false;
	private boolean stopNextCyclePassengerDelivered = false;
	
	void fragPlayer(Player player) {
		// remove from past cell
		int [] oldLocation = players.getLocation(player);
		map.setExplosion(oldLocation);
		map.setPlayer(oldLocation, null);

		// put the player somewhere
		int [] location = map.getAvailableLocationAmortized();
		if (location == null) {
			logger.error("Couldn't find spot to spawn player!");
			assert false;
			return;
		}
		
		map.setPlayer(location, player);
		
		// save the location
		players.setLocation(player, location);
		worldModule.fragPlayer(player, map, players, location);
		
		// reset the player state
		player.fragged();

		printSpawnMessage(player, location);
	}
	
	public void reset(GridMap map) {
		worldCount = 0;
		printedStats = false;
		worldModule.reset(map);
		this.stopNextCycleFuelRemaining = false;
		this.stopNextCyclePassengerPickUp = false;
		this.stopNextCyclePassengerDelivered = false;
		//System.out.println(map);
	}
	
	public int getWorldCount() {
		return worldCount;
	}
	
	public boolean isTerminal() {
		return printedStats;
	}

	public boolean recentlyMovedOrRotated(Player targetPlayer) {
		MoveInfo move = players.getMove(targetPlayer);
		if (move == null) {
			return false;
		}
		return move.move || move.rotate;
	}

	public void interruped(String name) {
		if (Soar2D.wm.using()) {
			return;
		}
		if (players.numberOfPlayers() <= 1) {
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
		this.stopAndDumpStats("interrupted");
	}
	
	public PlayersManager getPlayers() {
		return players;
	}
}
