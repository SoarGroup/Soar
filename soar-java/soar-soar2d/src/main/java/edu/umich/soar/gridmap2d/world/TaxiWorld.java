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

	private TaxiMap taxiMap;
	private PlayersManager<Taxi> players = new PlayersManager<Taxi>();
	private List<String> stopMessages = new ArrayList<String>();
	private CognitiveArchitecture cogArch;
	private int fuelStartMin;
	private int fuelStartMax;
	private int refuel;
	private boolean disableFuel;
	private boolean forceHuman = false;

	public TaxiWorld(CognitiveArchitecture cogArch, int fuelStartMin, int fuelStartMax, int refuel, boolean disableFuel) throws Exception {
		this.cogArch = cogArch;
		this.fuelStartMin = fuelStartMin;
		this.fuelStartMax = fuelStartMax;
		this.refuel = refuel;
		this.disableFuel = disableFuel;
	}
	
	public void setMap(String mapPath) throws Exception {
		TaxiMap oldMap = taxiMap;
		try {
			taxiMap = new TaxiMap(mapPath);
		} catch (Exception e) {
			if (oldMap == null) {
				throw e;
			}
			taxiMap = oldMap;
			logger.error("Map load failed, restored old map.");
			return;
		}
		
		// This can throw a more fatal error.
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
		
		for (Taxi taxi : players.getAll()) {
			// find a suitable starting location
			int [] startingLocation = WorldUtil.getStartingLocation(taxi, taxiMap, players.getInitialLocation(taxi));
			players.setLocation(taxi, startingLocation);

			// put the player in it
			taxiMap.getCell(startingLocation).setPlayer(taxi);

			taxi.reset();
		}
		
		updatePlayers();
	}

	private void updatePlayers() throws Exception {
		for (Taxi taxi : players.getAll()) {
			taxi.update(players.getLocation(taxi), taxiMap);
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
				if (taxiMap.isInBounds(newLocation) && taxiMap.exitable(location, command.moveDirection)) {
					taxi.consumeFuel();
					if (taxi.getFuel() < 0) {
						taxi.adjustPoints(-20, "fuel fell below zero");
					} else {
						// remove from cell
						taxiMap.getCell(location).setPlayer(null);
						players.setLocation(taxi, newLocation);
						
						// TODO: collisions not handled
						
						taxiMap.getCell(newLocation).setPlayer(taxi);
						taxi.adjustPoints(-1, "legal move");
					}
				} else {
					taxi.adjustPoints(-1, "illegal move");
				}
				
			} else if (command.pickup) {
				if (taxiMap.pickUp(location)) {
					taxi.adjustPoints(-1, "legal pickup");
				} else {
					taxi.adjustPoints(-10, "illegal pickup");
				}
				
			} else if (command.putdown) {
				if (taxiMap.putDown(location))  
				{
					if (taxiMap.isCorrectPassengerDestination(location)) {
						taxiMap.deliverPassenger();
						taxi.adjustPoints(20, "successful delivery");
					} else {
						taxi.adjustPoints(-10, "incorrect destination");
					}
				} else {
					taxi.adjustPoints(-10, "illegal putdown");
				}
				
			} else if (command.fillup) {
				if (taxiMap.isFuel(location)) {
					taxi.fillUp();
					taxi.adjustPoints(-1, "legal fillup");
				} else {
					taxi.adjustPoints(-10, "illegal fillup");
				}
			}
		}
	}
	
	public void update(int worldCount) throws Exception {
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
			if (taxiMap.isPassengerDelivered()) {
				stopMessages.add("Passenger delivered.");
			}
		}
	}
	
	private void checkPassengerPickedUp() {
		if (Gridmap2D.config.terminalsConfig().passenger_pick_up) {
			if (taxiMap.isPassengerCarried()) {
				stopMessages.add("There are no points remaining.");
			}
		}
	}
	
	public void reset() throws Exception {
		taxiMap.reset();
		resetState();
	}

	public void addPlayer(String playerId, PlayerConfig playerConfig, boolean debug) throws Exception {
		
		Taxi taxi = new Taxi(playerId, fuelStartMin, fuelStartMax, refuel, disableFuel);

		players.add(taxi, taxiMap, playerConfig.pos);
		
		if (playerConfig.productions != null) {
			TaxiCommander taxiCommander = cogArch.createTaxiCommander(taxi, playerConfig.productions, playerConfig.shutdown_commands, taxiMap.getMetadataFile(), debug);
			taxi.setCommander(taxiCommander);
		}

		int [] location = WorldUtil.getStartingLocation(taxi, taxiMap, players.getInitialLocation(taxi));
		players.setLocation(taxi, location);
		
		// put the player in it
		taxiMap.getCell(location).setPlayer(taxi);
		
		logger.info(taxi.getName() + ": Spawning at (" + location[0] + "," + location[1] + ")");
		
		updatePlayers();
	}

	public GridMap getMap() {
		return taxiMap;
	}

	public Player[] getPlayers() {
		return players.getAllAsPlayers();
	}

	public void interrupted(String agentName) throws Exception {
		players.interrupted(agentName);
		stopMessages.add("interrupted");
	}

	public boolean isTerminal() {
		return stopMessages.size() > 0;
	}

	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	public void removePlayer(String name) throws Exception {
		Taxi taxi = players.get(name);
		taxiMap.getCell(players.getLocation(taxi)).setPlayer(null);
		players.remove(taxi);
		taxi.shutdownCommander();
		updatePlayers();
	}

	public void setForceHumanInput(boolean setting) {
		forceHuman = setting;
	}
}
