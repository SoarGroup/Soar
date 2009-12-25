package edu.umich.soar.sproom.soar.commands;

import edu.umich.soar.sproom.Adaptable;
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
	public void update(Adaptable app) {
	}

	@Override
	public OutputLinkCommand accept() {
		throw new IllegalStateException();
	}
}
