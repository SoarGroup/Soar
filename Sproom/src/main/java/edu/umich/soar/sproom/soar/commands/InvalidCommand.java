package edu.umich.soar.sproom.soar.commands;

import edu.umich.soar.sproom.Adaptable;
import sml.Identifier;

class InvalidCommand extends OutputLinkCommand {
	public InvalidCommand(Identifier wme, String errorMessage) {
		super(wme);
		addStatus(CommandStatus.ERROR, errorMessage);
	}
	
	@Override
	public void update(Adaptable app) {
	}

	@Override
	public OutputLinkCommand accept() {
		return null;
	}
}
