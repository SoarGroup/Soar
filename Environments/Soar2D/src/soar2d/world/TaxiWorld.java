package soar2d.world;

import java.awt.Point;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.configuration.Configuration;
import soar2d.map.CellObject;
import soar2d.map.GridMap;
import soar2d.map.KitchenMap;
import soar2d.map.TankSoarMap;
import soar2d.map.TaxiMap;
import soar2d.player.MoveInfo;
import soar2d.player.Player;
import soar2d.player.kitchen.Cook;
import soar2d.player.taxi.Taxi;

public class TaxiWorld implements IWorld {

	public void fragPlayer(Player player, GridMap map, PlayersManager players,
			Point location) {

	}

	public boolean postLoad(GridMap _map) {
		TaxiMap map = (TaxiMap)_map;
		map.setPassengerDefaults();
		reset(_map);
		return true;
	}

	public void putInStartingLocation(Player player, GridMap map,
			PlayersManager players, Point location) {
	}

	public void reset(GridMap _map) {
		TaxiMap map = (TaxiMap)_map;
		map.placePassengerAndSetDestination();
	}
	
	public boolean update(GridMap _map, PlayersManager players) {
		TaxiMap map = (TaxiMap)_map;
		
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Taxi player = (Taxi)iter.next();
			MoveInfo move = players.getMove(player);
			
			// for visual world 
			player.resetPointsChanged();
			
			// check for break-out
			if (Soar2D.control.isShuttingDown()) {
				return false;
			}

			if (move.move) {
				// Calculate new location
				Point oldLocation = players.getLocation(player);
				Point newLocation = new Point(oldLocation);
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
		return false;
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

	public GridMap newMap(Configuration config) {
		return new TaxiMap(config);
	}
}
