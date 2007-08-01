package soar2d;

import java.awt.Point;
import java.util.ArrayList;
import java.util.HashMap;

import soar2d.player.MoveInfo;
import soar2d.player.Player;
import soar2d.world.GridMap;

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
