package edu.umich.soar.gridmap2d.players;

import java.util.List;

import edu.umich.soar.gridmap2d.map.RoomMap;

public interface RoomCommander extends Commander {
	public void update(RoomMap roomMap) throws Exception;
	public void receiveMessage(Player player, List<String> message);
}
