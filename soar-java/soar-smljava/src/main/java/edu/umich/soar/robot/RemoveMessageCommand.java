/**
 * 
 */
package edu.umich.soar.robot;

import org.apache.log4j.Logger;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Removes a message from the received message list.
 */
final public class RemoveMessageCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(RemoveMessageCommand.class);
	static final String NAME = "remove-message";

	static Command newInstance(ReceiveMessagesInterface messages) {
		return new RemoveMessageCommand(messages);
	}
	
	public RemoveMessageCommand(ReceiveMessagesInterface messages) {
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
