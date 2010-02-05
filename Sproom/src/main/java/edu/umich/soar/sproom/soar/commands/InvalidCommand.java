package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import sml.Identifier;

class InvalidCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(OutputLinkCommand.class);
	
	private final String name;
	
	public InvalidCommand(Identifier wme, String errorMessage) {
		super(Integer.valueOf(wme.GetTimeTag()));
		this.name = wme.GetCommandName();
		CommandStatus.error.addStatus(wme, errorMessage);
		logger.warn(this.name + ": " + errorMessage);
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
