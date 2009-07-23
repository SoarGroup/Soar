/**
 * 
 */
package edu.umich.soar.robot;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;
import sml.WMElement;

/**
 * @author voigtjr
 *
 * Broadcasts a message to all listeners.
 */
final public class SendMessageCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(SendMessageCommand.class);
	public static final String NAME = "send-message";

	public static Command newInstance(MessagesInterface messages) {
		return new SendMessageCommand(messages);
	}
	
	public SendMessageCommand(MessagesInterface messages) {
		this.messages = messages;
	}

	private final MessagesInterface messages;

	@Override
	public boolean execute(Agent agent, Identifier command) {
		String destination = command.GetParameterValue("destination");
		if (destination == null) {
			logger.warn(NAME + ": No destination on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}
		
		StringBuilder message = new StringBuilder();
		try {
			Identifier next = command.FindByAttribute("first", 0).ConvertToIdentifier();
			logger.trace("first: " + next);
			while (next != null) {
				WMElement word = next.FindByAttribute("word", 0);
				if (word == null) {
					logger.trace("word is null");
					throw new NullPointerException();
				}
				logger.trace("word: " + word.GetValueAsString());
				message.append(word.GetValueAsString());
				message.append(" ");
				
				WMElement nextwme = next.FindByAttribute("next", 0);
				if (nextwme == null) {
					logger.trace("next is null");
					throw new NullPointerException();
				}
				logger.trace("next: " + nextwme.GetValueAsString());
				if (nextwme.GetValueAsString().equals("nil")) {
					break;
				}
				
				next = nextwme.ConvertToIdentifier();
				if (next == null) {
					logger.trace("next is not identifier");
					throw new NullPointerException();
				}
			}
			
		} catch (NullPointerException e) {
			logger.warn(NAME + ": malformed message on send-message command. Message before error: " + message);
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		if (message.length() == 0) {
			logger.warn(NAME + ": no message to send");
			CommandStatus.error.addStatus(agent, command);
			return false;
		} 
		
		messages.newMessage(destination, message.toString());
		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}
