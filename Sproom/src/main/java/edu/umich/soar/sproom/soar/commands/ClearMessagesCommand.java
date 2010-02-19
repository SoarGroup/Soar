/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.command.Comm;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Removes all messages from message list.
 */
public class ClearMessagesCommand extends OutputLinkCommand {
	static final String NAME = "clear-messages";

	public ClearMessagesCommand(Identifier wme) {
		super(wme);
	}

	@Override
	protected boolean accept() {
		addStatus(CommandStatus.ACCEPTED);
		return true;
	}
	
	@Override
	public void update(Adaptable app) {
		Comm comm = (Comm)app.getAdapter(Comm.class);
		comm.clearMessages();
		addStatus(CommandStatus.COMPLETE);
	}
}
