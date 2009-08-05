package edu.umich.soar.gridmap2d.players;


import java.util.List;

import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.robot.ReceiveMessagesInterface;

public class Robot extends Player {
	private RobotCommander commander;
	private RobotState state;
	
	public Robot(String playerID) {
		super(playerID);
		
		state = new RobotState();
		state.reset();
	}
	
	public RobotState getState() {
		return state;
	}

	public void setCommander(RobotCommander commander) {
		this.commander = commander;
	}
	
	public CommandInfo getCommand() {
		CommandInfo command;
		if (commander != null) {
			command = commander.nextCommand();
		} else {
			command = Gridmap2D.control.getHumanCommand(this);
		}
		
		return command;
	}
	
	public void update(int[] newLocation, RoomMap roomMap) {
		super.update(newLocation);
		moved = true;
		if (commander != null) {
			commander.update(roomMap);
		}
	}
	
	@Override
	public void reset() {
		super.reset();
		
		if (state != null) {
			state.reset();
		}
		
		if (commander != null) {
			commander.reset();
		}
	}

	public void shutdownCommander() {
		if (commander != null) {
			commander.shutdown();
		}
	}

	public List<double[]> getWaypointList() {
		return commander.getWaypointList();
	}

	public ReceiveMessagesInterface getReceiveMessagesInterface() {
		return commander.getReceiveMessagesInterface();
	}
}
