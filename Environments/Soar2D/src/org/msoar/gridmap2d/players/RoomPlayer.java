package org.msoar.gridmap2d.players;

import org.msoar.gridmap2d.Gridmap2D;
import org.msoar.gridmap2d.map.CellObject;
import org.msoar.gridmap2d.map.RoomMap;

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
	
	public void update(int[] newLocation, double[] newFloatLocation, RoomMap roomMap) throws Exception {
		super.update(newLocation);
		moved = moved || Double.compare(newFloatLocation[0], state.getFloatLocation()[0]) != 0 || Double.compare(newFloatLocation[1], state.getFloatLocation()[1]) != 0;
		moved = moved || getState().rotated();
		getState().resetRotated();
		if (moved) {
			state.setFloatLocation(newFloatLocation);
		}
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

	public void rotateComplete() {
		commander.rotateComplete();
	}

	public void updateDropStatus(boolean b) {
		commander.updateDropStatus(b);
	}

	public void receiveMessage(Player player, String message) {
		commander.receiveMessage(player, message);
	}

	public void updateGetStatus(boolean b) {
		commander.updateGetStatus(b);
	}

	public void shutdownCommander() throws Exception {
		if (commander != null) {
			commander.shutdown();
		}
	}

	public void carry(CellObject object) {
		state.carry(object);
		commander.carry(object);
	}
	
	public CellObject drop() {
		CellObject temp = state.drop();
		commander.drop();
		return temp;
	}

}
