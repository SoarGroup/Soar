package edu.umich.soar.robot;

import sml.Agent;
import sml.Identifier;

abstract class DDCCommand implements Command {

	protected Agent agent;
	protected Identifier command;
	
	@Override
	public boolean createsDDC() {
		return true;
	}

	@Override
	public Identifier wme() {
		return command;
	}

	@Override
	public void interrupt() {
		if (agent == null || command == null) {
			throw new IllegalStateException();
		}
		
		CommandStatus.complete.addStatus(agent, command);
		agent = null;
		command = null;
	}

	@Override
	public boolean update() {
		return false; // still executing
	}
}
