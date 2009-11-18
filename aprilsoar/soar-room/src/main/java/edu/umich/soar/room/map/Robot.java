package edu.umich.soar.room.map;

import java.util.ArrayList;
import java.util.List;

import edu.umich.soar.robot.ReceiveMessagesInterface;
import edu.umich.soar.room.core.PlayerColor;

public class Robot {
	private RobotCommander commander;
	private RobotState state;
	
	private final String name;	
	private final PlayerColor color;	

	public PlayerColor getColor() {
		return this.color;
	}

	public String getName() {
		return this.name;
	}
	
	@Override
	public int hashCode() {
		return name.hashCode();
	}

	@Override
	public boolean equals(Object o) {
		if (this == o) {
			return true;
		}
		
		if (!(o instanceof Robot)) {
			return false;
		}
		Robot player = (Robot)o;
		
		return name.equals(player.name);
	}

	@Override
	public String toString() {
		return getName();
	}
	
	public Robot(String name, PlayerColor color) {
		if (name == null) {
			throw new NullPointerException("name is null");
		}
		this.name = name;
		
		if (color == null) {
			throw new NullPointerException("color is null");
		}
		this.color = color;
		
		state = new RobotState();
		state.reset();
	}
	
	public RobotState getState() {
		return state;
	}

	public void setCommander(RobotCommander commander) {
		this.commander = commander;
	}
	
	public RobotCommand getCommand() {
		return commander.nextCommand();
	}
	
	public void reset() {
		if (state != null) {
			state.reset();
		}
		
		if (commander != null) {
			commander.reset();
		}
	}

	public void shutdown() {
		if (commander != null) {
			commander.shutdown();
		}
	}

	private static final List<double[]> EMPTY_LIST = new ArrayList<double[]>(0);
	public List<double[]> getWaypointList() {
		if (commander != null) {
			return commander.getWaypointList();
		}
		return EMPTY_LIST;
	}

	public ReceiveMessagesInterface getReceiveMessagesInterface() {
		return commander.getReceiveMessagesInterface();
	}
}
