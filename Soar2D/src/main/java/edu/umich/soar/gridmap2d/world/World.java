package edu.umich.soar.gridmap2d.world;


import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.map.GridMap;
import edu.umich.soar.gridmap2d.players.Player;

public interface World {
	// simulation
	public void update(int worldCount);
	public void reset();
	public void setAndResetMap(String mapPath);
	public GridMap getMap();
	
	// player management
	public int numberOfPlayers();
	public boolean hasPlayer(String name);
	public boolean addPlayer(String playerId, PlayerConfig playerConfig, boolean debug);
	public void removePlayer(String name);
	public Player[] getPlayers();
	
	// control
	public void setForceHumanInput(boolean setting);
	public boolean isTerminal();
	public void interrupted(String agentName);
}
