package edu.umich.soar.sps.control;

import edu.umich.soar.robot.OffsetPose;
import sml.Agent;
import sml.Identifier;

abstract class DDCCommand implements Command {

	protected Agent agent;
	protected Identifier command;
	
	public boolean createsDDC() {
		return true;
	}

	public Identifier wme() {
		return command;
	}

	public void interrupt() {
		if (agent == null || command == null) {
			throw new IllegalStateException();
		}
		
		CommandStatus.complete.addStatus(agent, command);
		agent = null;
		command = null;
	}

	public boolean update(OffsetPose opose) {
		return false; // still executing
	}
}
