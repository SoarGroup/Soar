package edu.umich.soar.gridmap2d.players;

import java.util.List;

import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.robot.ReceiveMessagesInterface;

public interface RobotCommander extends Commander {
	public void update(RoomMap roomMap);
	public List<double[]> getWaypointList();
	public ReceiveMessagesInterface getReceiveMessagesInterface();
}
