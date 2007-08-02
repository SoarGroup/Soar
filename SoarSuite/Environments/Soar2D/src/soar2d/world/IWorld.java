package soar2d.world;

import java.awt.Point;

import soar2d.map.GridMap;
import soar2d.player.Player;

public interface IWorld {
	public boolean postLoad(GridMap newMap);
	public boolean update(GridMap map, PlayersManager players);
	public void updatePlayers(boolean playersChanged, GridMap map, PlayersManager players);
	public void fragPlayer(Player player, GridMap map, PlayersManager players, Point location);
	public void putInStartingLocation(Player player, GridMap map, PlayersManager players, Point location);
	public void reset();
	public int getMinimumAvailableLocations();
	public void resetPlayer(GridMap map, Player player, PlayersManager players, boolean resetDuringRun);
}
