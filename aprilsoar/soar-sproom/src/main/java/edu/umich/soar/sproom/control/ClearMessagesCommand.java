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
 * Removes all messages from message list.
 */
final public class ClearMessagesCommand extends NoDDCAdapter implements Command {
	private static final Log logger = LogFactory.getLog(ClearMessagesCommand.class);
	static final String NAME = "clear-messages";

	static Command newInstance(ReceiveMessagesInterface messages) {
		return new ClearMessagesCommand(messages);
	}
	
	private ClearMessagesCommand(ReceiveMessagesInterface messages) {
		this.messages = messages;
	}

	private final ReceiveMessagesInterface messages;

	@Override
	public boolean execute(Identifier command) {
		messages.clearMessages();
		logger.info(NAME + ":");

		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		return true;
	}
}
