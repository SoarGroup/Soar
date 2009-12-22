/**
 * 
 */
package edu.umich.soar.sproom.control;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Removes a message from the received message list.
 */
public class RemoveMessageCommand extends NoDDCAdapter implements Command {
	private static final Log logger = LogFactory.getLog(RemoveMessageCommand.class);
	static final String NAME = "remove-message";

	static Command newInstance(ReceiveMessagesInterface messages) {
		return new RemoveMessageCommand(messages);
	}
	
	private RemoveMessageCommand(ReceiveMessagesInterface messages) {
		this.messages = messages;
	}

	private final ReceiveMessagesInterface messages;

	@Override
	public boolean execute(Identifier command) {

		int id = -1;
		try {
			id = Integer.parseInt(command.GetParameterValue("id"));
		} catch (NullPointerException ignored) {
			CommandStatus.error.addStatus(command, NAME + ": No id on command");
			return false;
		} catch (NumberFormatException e) {
			CommandStatus.error.addStatus(command, NAME + ": Unable to parse id: " + command.GetParameterValue("id"));
			return false;
		}

		logger.debug(String.format(NAME + ": %d", id));
		
		if (messages.removeMessage(id) == false) {
			CommandStatus.error.addStatus(command, NAME + ": Unable to remove message " + id + ", no such message");
			return false;
		}

		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		return true;
	}
}
