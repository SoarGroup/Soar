package soar2d.player;

import java.util.*;
import java.util.logging.*;

import sml.*;
import soar2d.*;

public class SoarTank extends Eater {
	private Agent agent;
	private ArrayList<String> shutdownCommands;

	public SoarTank(Agent agent, PlayerConfig playerConfig) {
		super(playerConfig);
		this.agent = agent;
		this.shutdownCommands = playerConfig.getShutdownCommands();
		
		previousLocation = new java.awt.Point(-1, -1);
	}
	
	public void update(World world, java.awt.Point location) {
		moved = (location.x != this.previousLocation.x) || (location.y != this.previousLocation.y);
	}
	
	public MoveInfo getMove() {
		if (agent.GetNumberCommands() == 0) {
			if (logger.isLoggable(Level.FINE)) logger.fine(getName() + " issued no command.");
			return new MoveInfo();
		}
		
		return new MoveInfo();
	}
	
	public void reset() {
		agent.InitSoar();
	}
	
	public void shutdown() {
		assert agent != null;
		if (shutdownCommands == null) { 
			return;
		}
		
		Iterator<String> iter = shutdownCommands.iterator();
		while(iter.hasNext()) {
			String command = iter.next();
			String result = getName() + ": result: " + agent.ExecuteCommandLine(command, true);
			Soar2D.logger.info(getName() + ": shutdown command: " + command);
			if (agent.HadError()) {
				Soar2D.control.severeError(result);
			} else {
				Soar2D.logger.info(getName() + ": result: " + result);
			}
		}
	}
}
