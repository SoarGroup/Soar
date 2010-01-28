package edu.umich.soar.gridmap2d.players;


import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.map.EatersMap;

public class Eater extends Player {	
	private EaterCommander commander;

	public Eater(String playerId) {
		super(playerId);
	}
	
	public void setCommander(EaterCommander commander) {
		this.commander = commander;
	}
	
	public CommandInfo getCommand() {
		CommandInfo command;
		if (commander != null) {
			command = commander.nextCommand();
		} else {
			command = Gridmap2D.control.getHumanCommand(this);
		}
		
		// the facing depends on the move
		if (command.move) { 
			super.setFacing(command.moveDirection);
		}

		return command;
	}
	
	public void update(int[] newLocation, EatersMap eatersMap) {
		super.update(newLocation);
		if (commander != null) {
			commander.update(eatersMap);
		}
	}

	@Override
	public void reset() {
		super.reset();
		if (commander != null) {
			commander.reset();
		}
	}

	public void shutdownCommander() {
		if (commander != null) {
			commander.shutdown();
		}
	}
}
