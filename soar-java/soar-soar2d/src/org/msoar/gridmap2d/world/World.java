package org.msoar.gridmap2d.world;

import org.msoar.gridmap2d.config.PlayerConfig;
import org.msoar.gridmap2d.map.GridMap;
import org.msoar.gridmap2d.players.Player;

public interface World {
	// simulation
	public void update(int worldCount) throws Exception;
	public void reset() throws Exception;
	public void setMap(String mapPath) throws Exception;
	public GridMap getMap();
	
	// player management
	public int numberOfPlayers();
	public void addPlayer(String playerId, PlayerConfig playerConfig, boolean debug) throws Exception;
	public void removePlayer(String name) throws Exception;
	public Player[] getPlayers();
	
	// control
	public void setForceHumanInput(boolean setting);
	public boolean isTerminal();
	public void interrupted(String agentName) throws Exception;
}
