package edu.umich.soar.sproom.command;

import lcmtypes.pose_t;
import sml.Identifier;

class InvalidCommand extends OutputLinkCommand {
	private final String name;
	
	InvalidCommand(Identifier wme, String errorMessage) {
		super(Integer.valueOf(wme.GetTimeTag()));
		this.name = wme.GetCommandName();
		CommandStatus.error.addStatus(wme, errorMessage);
	}
	
	@Override
	public String getName() {
		return name;
	}
	
	@Override
	public boolean update(pose_t pose) {
		return true;
	}

	@Override
	public OutputLinkCommand accept() {
		throw new IllegalStateException();
	}
}
