package edu.umich.soar.gridmap2d.world;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.CognitiveArchitecture;
import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.map.GridMap;
import edu.umich.soar.gridmap2d.map.TaxiMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.Taxi;
import edu.umich.soar.gridmap2d.players.TaxiCommander;

public class TaxiWorld implements World {
	private static Logger logger = Logger.getLogger(TaxiWorld.class);

	private TaxiMap map;
	private PlayersManager<Taxi> players = new PlayersManager<Taxi>();
	private List<String> stopMessages = new ArrayList<String>();
	private CognitiveArchitecture cogArch;
	private int fuelStartMin;
	private int fuelStartMax;
	private int refuel;
	private boolean disableFuel;
	private boolean forceHuman = false;

	public TaxiWorld(CognitiveArchitecture cogArch, int fuelStartMin, int fuelStartMax, int refuel, boolean disableFuel) {
		this.cogArch = cogArch;
		this.fuelStartMin = fuelStartMin;
		this.fuelStartMax = fuelStartMax;
		this.refuel = refuel;
		this.disableFuel = disableFuel;
	}
	
	@Override
	public void setAndResetMap(String mapPath) {
		TaxiMap newMap = TaxiMap.generateInstance(mapPath);
		if (newMap == null) {
			return;
		}
		map = newMap;
		resetState();
	}
	
	private void resetState() {
		stopMessages.clear();
		resetPlayers();
	}
	
	/**
	 * @throws IllegalStateException If there are no available locations for the player to spawn
	 */
	private void resetPlayers() {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (Taxi taxi : players.getAll()) {
			// find a suitable starting location
			int [] location = WorldUtil.getStartingLocation(map, players.getInitialLocation(taxi));
			if (location == null) {
				throw new IllegalStateException("no empty locations available for spawn");
			}
			players.setLocation(taxi, location);

			// put the player in it
			map.getCell(location).setPlayer(taxi);

			taxi.reset();
		}
		
		updatePlayers();
	}

	private void updatePlayers() {
		for (Taxi taxi : players.getAll()) {
			taxi.update(players.getLocation(taxi), map);
		}
	}

	private void moveTaxis() {
		for (Taxi taxi : players.getAll()) {
			CommandInfo command = players.getCommand(taxi);			

			int [] location = players.getLocation(taxi);
			if (command.move) {
				// Calculate new location
				int [] newLocation = Arrays.copyOf(location, location.length);
				Direction.translate(newLocation, command.moveDirection);
				
				// Verify legal move and commit move
				if (map.isInBounds(newLocation) && map.exitable(location, command.moveDirection)) {
					taxi.consumeFuel();
					if (taxi.getFuel() < 0) {
						taxi.adjustPoints(-20, "fuel fell below zero");
					} else {
						// remove from cell
						map.getCell(location).removeAllPlayers();
						players.setLocation(taxi, newLocation);
						
						map.getCell(newLocation).setPlayer(taxi);
						taxi.adjustPoints(-1, "legal move");
					}
				} else {
					taxi.adjustPoints(-1, "illegal move");
				}
				
			} else if (command.pickup) {
				if (map.pickUp(location)) {
					taxi.adjustPoints(-1, "legal pickup");
				} else {
					taxi.adjustPoints(-10, "illegal pickup");
				}
				
			} else if (command.putdown) {
				if (map.putDown(location))  
				{
					if (map.isCorrectPassengerDestination(location)) {
						map.deliverPassenger();
						taxi.adjustPoints(20, "successful delivery");
					} else {
						taxi.adjustPoints(-10, "incorrect destination");
					}
				} else {
					taxi.adjustPoints(-10, "illegal putdown");
				}
				
			} else if (command.fillup) {
				if (map.isFuel(location)) {
					taxi.fillUp();
					taxi.adjustPoints(-1, "legal fillup");
				} else {
					taxi.adjustPoints(-10, "illegal fillup");
				}
			}
		}
	}
	
	public void update(int worldCount) {
		WorldUtil.checkNumPlayers(players.numberOfPlayers());

		// Collect input
		for (Taxi taxi : players.getAll()) {
			taxi.resetPointsChanged();
			CommandInfo command = forceHuman ? Gridmap2D.control.getHumanCommand(taxi) : taxi.getCommand();
			if (command == null) {
				Gridmap2D.control.stopSimulation();
				return;
			}
			players.setCommand(taxi, command);
			WorldUtil.checkStopSim(stopMessages, command, taxi);
		}

		moveTaxis();
		if (Gridmap2D.control.isShuttingDown()) {
			return;
		}
		updatePlayers();
		
		checkFuelRemaining();
		checkPassengerDelivered();
		checkPassengerPickedUp();
		WorldUtil.checkMaxUpdates(stopMessages, worldCount);
		WorldUtil.checkWinningScore(stopMessages, players.getSortedScores());
		
		if (stopMessages.size() > 0) {
			Gridmap2D.control.stopSimulation();
			boolean stopping = Gridmap2D.control.getRunsTerminal() <= 0;
			WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), stopping, stopMessages);
		}
	}
	
	private void checkFuelRemaining() {
		if (Gridmap2D.config.terminalsConfig().fuel_remaining) {
			for (Taxi taxi : players.getAll()) {
				if (taxi.getFuel() < 0) {
					stopMessages.add("Fuel is negative.");
				}
			}
		}
	}
	
	private void checkPassengerDelivered() {
		if (Gridmap2D.config.terminalsConfig().passenger_delivered) {
			if (map.isPassengerDelivered()) {
				stopMessages.add("Passenger delivered.");
			}
		}
	}
	
	private void checkPassengerPickedUp() {
		if (Gridmap2D.config.terminalsConfig().passenger_pick_up) {
			if (map.isPassengerCarried()) {
				stopMessages.add("There are no points remaining.");
			}
		}
	}
	
	public void reset() {
		map.reset();
		resetState();
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

		Taxi player = new Taxi(id, fuelStartMin, fuelStartMax, refuel, disableFuel);
		players.add(player, cfg.pos);
		
		if (cfg.productions != null) {
			TaxiCommander cmdr = cogArch.createTaxiCommander(player, cfg.productions, cfg.shutdown_commands, debug);
			if (cmdr == null) {
				players.remove(player);
				return false;
			}
			player.setCommander(cmdr);
		}

		players.setLocation(player, location);
		
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

	public void interrupted(String agentName) {
		players.interrupted(agentName);
		stopMessages.add("interrupted");
	}

	public boolean isTerminal() {
		return stopMessages.size() > 0;
	}

	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	public void removePlayer(String name) {
		Taxi taxi = players.get(name);
		map.getCell(players.getLocation(taxi)).removeAllPlayers();
		players.remove(taxi);
		taxi.shutdownCommander();
		updatePlayers();
	}

	public void setForceHumanInput(boolean setting) {
		forceHuman = setting;
	}

}
