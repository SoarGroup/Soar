/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.comm.Comm;

import sml.Identifier;

/**
 * Removes a message from the received message list.
 *
 * @author voigtjr@gmail.com
 */
public class RemoveMessageCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(RemoveMessageCommand.class);
	static final String NAME = "remove-messages";

	private final Identifier wme;
	private long id;

	public RemoveMessageCommand(Identifier wme) {
		super(wme);
		this.wme = wme;
	}

	@Override
	protected boolean accept() {
		String idString = wme.GetParameterValue(SharedNames.ID);
		try {
			id = Long.parseLong(idString);
		} catch (NullPointerException e) {
			addStatusError("No " + SharedNames.ID);
			return false;
		} catch (NumberFormatException e) {
			addStatusError("Error parsing " + SharedNames.ID + ": " + idString);
			return false;
		}
		
		logger.debug(id);
		addStatus(CommandStatus.ACCEPTED);
		return true;
	}

	@Override
	public void update(Adaptable app) {
		Comm comm = (Comm)app.getAdapter(Comm.class);
		comm.removeMessage(id);
		addStatus(CommandStatus.COMPLETE);
	}
}
