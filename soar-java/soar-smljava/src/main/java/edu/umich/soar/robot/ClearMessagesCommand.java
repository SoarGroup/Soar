/**
 * 
 */
package edu.umich.soar.robot;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Removes all messages from message list.
 */
final public class ClearMessagesCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(ClearMessagesCommand.class);
	static final String NAME = "clear-messages";

	static Command newInstance(ReceiveMessagesInterface messages) {
		return new ClearMessagesCommand(messages);
	}
	
	public ClearMessagesCommand(ReceiveMessagesInterface messages) {
		this.messages = messages;
	}

	private final ReceiveMessagesInterface messages;

	@Override
	public boolean execute(Agent agent, Identifier command) {
		messages.clearMessages();
		logger.info(NAME + ":");

		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}
