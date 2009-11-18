package edu.umich.soar.room.map;

import java.util.List;

import edu.umich.soar.robot.ReceiveMessagesInterface;

public interface RobotCommander {
	public List<double[]> getWaypointList();
	public ReceiveMessagesInterface getReceiveMessagesInterface();
	public RobotCommand nextCommand();
	public void reset();
	public void shutdown();
}
