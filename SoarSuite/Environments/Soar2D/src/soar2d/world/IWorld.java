package soar2d.world;

import soar2d.map.GridMap;
import soar2d.players.Player;

public interface IWorld {
	public boolean postLoad(GridMap newMap);
	public String update(GridMap map, PlayersManager players);
	public void updatePlayers(boolean playersChanged, GridMap map, PlayersManager players);
	public void fragPlayer(Player player, GridMap map, PlayersManager players, int [] location);
	public void putInStartingLocation(Player player, GridMap map, PlayersManager players, int [] location);
	public void reset(GridMap map);
	public int getMinimumAvailableLocations();
	public void resetPlayer(GridMap map, Player player, PlayersManager players, boolean resetDuringRun);
	public GridMap newMap();
}
