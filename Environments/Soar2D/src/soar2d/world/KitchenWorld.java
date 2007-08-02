package soar2d.world;

import java.awt.Point;
import java.util.Iterator;

import soar2d.Direction;
import soar2d.Soar2D;
import soar2d.map.GridMap;
import soar2d.player.MoveInfo;
import soar2d.player.Player;

public class KitchenWorld implements IWorld {

	public void fragPlayer(Player player, GridMap map, PlayersManager players,
			Point location) {

	}

	public boolean postLoad(GridMap newMap) {
		return true;
	}

	public void putInStartingLocation(Player player, GridMap map,
			PlayersManager players, Point location) {

	}

	public void reset() {

	}

	public boolean update(GridMap map, PlayersManager players) {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			MoveInfo move = players.getMove(player);
			
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
				if (map.isInBounds(newLocation) && map.enterable(newLocation)) {
					// remove from cell
					map.setPlayer(oldLocation, null);
					players.setLocation(player, newLocation);
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
		return 1;
	}

	public void resetPlayer(GridMap map, Player player, PlayersManager players, boolean resetDuringRun) {
		player.reset();
	}
}
