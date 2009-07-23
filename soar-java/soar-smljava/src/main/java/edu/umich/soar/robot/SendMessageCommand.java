/**
 * 
 */
package edu.umich.soar.robot;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

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
	static final String NAME = "send-message";

	static Command newInstance(SendMessagesInterface messages) {
		return new SendMessageCommand(messages);
	}
	
	public SendMessageCommand(SendMessagesInterface messages) {
		this.messages = messages;
	}

	private final SendMessagesInterface messages;
	private String agentName;
	
	@Override
	public boolean execute(Agent agent, Identifier command) {
		agentName = agent.GetAgentName();
		
		String destination = command.GetParameterValue("destination");
		if (destination == null) {
			logger.warn(NAME + ": No destination on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}
		
		List<String> tokens = new ArrayList<String>();
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
				tokens.add(word.GetValueAsString());
				
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
			logger.warn(NAME + ": malformed message on send-message command.");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		if (tokens.isEmpty()) {
			logger.warn(NAME + ": no message to send");
			CommandStatus.error.addStatus(agent, command);
			return false;
		} 
		
		Iterator<String> iter = tokens.iterator();
		while (iter.hasNext()) {
			String token = iter.next();
			if (token.length() == 0) {
				iter.remove();
			}
		}
		
		messages.sendMessage(agentName, destination, tokens);
		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}
