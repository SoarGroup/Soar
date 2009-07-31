package edu.umich.soar.gridmap2d.players;


import java.util.List;

import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.robot.ReceiveMessagesInterface;

public class RoomPlayer extends Player {
	private RoomCommander commander;
	private RoomPlayerState state;
	
	public RoomPlayer(String playerID) throws Exception {
		super(playerID);
		
		state = new RoomPlayerState();
		state.reset();
	}
	
	public RoomPlayerState getState() {
		return state;
	}

	public void setCommander(RoomCommander commander) {
		this.commander = commander;
	}
	
	public CommandInfo getCommand() throws Exception {
		CommandInfo command;
		if (commander != null) {
			command = commander.nextCommand();
		} else {
			command = Gridmap2D.control.getHumanCommand(this);
		}
		
		return command;
	}
	
	public void update(int[] newLocation, RoomMap roomMap) throws Exception {
		super.update(newLocation);
		moved = true;
		if (commander != null) {
			commander.update(roomMap);
		}
	}
	
	@Override
	public void reset() throws Exception {
		super.reset();
		
		if (state != null) {
			state.reset();
		}
		
		if (commander != null) {
			commander.reset();
		}
	}

	public void shutdownCommander() throws Exception {
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
