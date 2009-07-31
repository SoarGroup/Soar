package edu.umich.soar.gridmap2d.players;

import java.util.List;

import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.robot.ReceiveMessagesInterface;

public interface RoomCommander extends Commander {
	public void update(RoomMap roomMap) throws Exception;
	public List<double[]> getWaypointList();
	public ReceiveMessagesInterface getReceiveMessagesInterface();
}
