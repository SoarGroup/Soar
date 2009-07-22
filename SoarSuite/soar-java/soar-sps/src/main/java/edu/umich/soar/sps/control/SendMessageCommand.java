/**
 * 
 */
package edu.umich.soar.sps.control;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;
import sml.WMElement;

/**
 * @author voigtjr
 *
 * Broadcasts a message to all listeners.
 */
final class SendMessageCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(SendMessageCommand.class);
	static final String NAME = "send-message";

	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, SplinterState splinter,
			OutputLinkManager outputLinkManager) {

		if (splinter == null) {
			throw new AssertionError();
		}
		
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
		
		if (destination.equals("say")) {
			Say.newMessage(message.toString());
			CommandStatus.accepted.addStatus(agent, command);
			CommandStatus.complete.addStatus(agent, command);
			return true;
		}

		logger.warn(NAME + ": Unsupported destination: " + destination);
		CommandStatus.error.addStatus(agent, command);
		return false;
	}
}
