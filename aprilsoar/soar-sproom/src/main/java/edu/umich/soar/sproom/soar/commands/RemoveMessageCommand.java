/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.command.Comm;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Removes a message from the received message list.
 */
public class RemoveMessageCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(RemoveMessageCommand.class);
	static final String NAME = "remove-messages";

	private static final String ID = "id";
	
	private final Identifier wme;
	private boolean complete = false;
	private long id;

	RemoveMessageCommand(Identifier wme) {
		super(Integer.valueOf(wme.GetTimeTag()));
		this.wme = wme;
	}

	@Override
	public OutputLinkCommand accept() {
		String idString = wme.GetParameterValue(ID);
		try {
			id = Long.parseLong(idString);
		} catch (NullPointerException e) {
			return new InvalidCommand(wme, "No " + ID);
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Error parsing " + ID + ": " + idString);
		}
		
		logger.debug(id);
		CommandStatus.accepted.addStatus(wme);
		return this;
	}
	
	@Override
	public String getName() {
		return NAME;
	}

	@Override
	public void update(Adaptable app) {
		if (!complete) {
			Comm comm = (Comm)app.getAdapter(Comm.class);
			comm.removeMessage(id);
			CommandStatus.complete.addStatus(wme);
			complete = true;
		}
	}
}
