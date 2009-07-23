package edu.umich.soar.gridmap2d.players;


import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.map.RoomMap;

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

	public void receiveMessage(Player player, String message) {
		List<String> tokens = Arrays.asList(message.split(" "));
		
		Iterator<String> iter = tokens.iterator();
		while (iter.hasNext()) {
			String token = iter.next();
			if (token.length() == 0) {
				iter.remove();
			}
		}
		commander.receiveMessage(player, tokens);
	}

	public void shutdownCommander() throws Exception {
		if (commander != null) {
			commander.shutdown();
		}
	}
}
