package broken.soar2d.world;

import java.util.Arrays;
import java.util.Iterator;

import soar2d.Direction;
import soar2d.Soar2D;
import soar2d.map.GridMap;
import soar2d.map.TaxiMap;
import soar2d.players.CommandInfo;
import soar2d.players.Player;
import soar2d.players.Taxi;

public class TaxiWorld implements World {

	public TaxiWorld(String map) throws Exception {
		
	}
	
	public void fragPlayer(Player player, GridMap map, PlayersManager players,
			int [] location) {

	}

	public boolean postLoad(GridMap _map) {
		TaxiMap map = (TaxiMap)_map;
		map.setPassengerDefaults();
		reset(_map);
		return true;
	}

	public void putInStartingLocation(Player player, GridMap map,
			PlayersManager players, int [] location) {
	}

	private boolean stopNextCyclePassengerPickUp = false;
	private boolean stopNextCycleFuelRemaining = false;
	private boolean stopNextCyclePassengerDelivered = false;
	
	public void reset(GridMap _map) {
		TaxiMap map = (TaxiMap)_map;
		map.placePassengerAndSetDestination();
		
		// TODO: from world
		this.stopNextCycleFuelRemaining = false;
		this.stopNextCyclePassengerPickUp = false;
		this.stopNextCyclePassengerDelivered = false;
	}
	
	public String update(GridMap _map, PlayersManager players) {
		TaxiMap map = (TaxiMap)_map;
		
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Taxi player = (Taxi)iter.next();
			CommandInfo move = players.getCommand(player);
			
			// for visual world 
			player.resetPointsChanged();
			
			// check for break-out
			if (Soar2D.control.isShuttingDown()) {
				return null;
			}

			if (move.move) {
				// Calculate new location
				int [] oldLocation = players.getLocation(player);
				int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);
				Direction.translate(newLocation, move.moveDirection);
				
				// Verify legal move and commit move
				if (map.isInBounds(newLocation) && map.exitable(oldLocation, move.moveDirection)) {
					map.consumeFuel();
					if (map.isFuelNegative()) {
						player.adjustPoints(-20, "fuel fell below zero");
					} else {
						// remove from cell
						map.setPlayer(oldLocation, null);
						players.setLocation(player, newLocation);
						
						// TODO: collisions not handled
						
						map.setPlayer(newLocation, player);
						player.adjustPoints(-1, "legal move");
					}
				} else {
					player.adjustPoints(-1, "illegal move");
				}
				
			} else if (move.pickup) {
				if (map.pickUp(players.getLocation(player))) {
					player.adjustPoints(-1, "legal pickup");
				} else {
					player.adjustPoints(-10, "illegal pickup");
				}
				
			} else if (move.putdown) {
				if (map.putDown(players.getLocation(player)) && map.isPassengerDestination(players.getLocation(player))) 
				{
					map.deliverPassenger();
					player.adjustPoints(20, "successful delivery");					
				} else {
					player.adjustPoints(-10, "illegal putdown");
				}
				
			} else if (move.fillup) {
				if (map.fillUp(players.getLocation(player))) {
					player.adjustPoints(-1, "legal fillup");
				} else {
					player.adjustPoints(-10, "illegal fillup");
				}
			}
		}
		
		updatePlayers(false, map, players);

		map.updateObjects(null);
		
		// do not reset after this frame
		return null;
	}

	public void updatePlayers(boolean playersChanged, GridMap map, PlayersManager players) {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			player.update(players.getLocation(player));
		}
	}
	
	public int getMinimumAvailableLocations() {
		return 2;
	}

	public void resetPlayer(GridMap map, Player player, PlayersManager players, boolean resetDuringRun) {
		player.reset();
	}

	public GridMap newMap() {
		return new TaxiMap();
	}
}
