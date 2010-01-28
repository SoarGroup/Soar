package edu.umich.soar.sps.control.robot;

import sml.Identifier;

abstract class DDCCommand implements Command {

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
		if (command == null) {
			throw new IllegalStateException();
		}
		
		CommandStatus.complete.addStatus(command);
		command = null;
	}

	@Override
	public boolean update() {
		return false; // still executing
	}
}
