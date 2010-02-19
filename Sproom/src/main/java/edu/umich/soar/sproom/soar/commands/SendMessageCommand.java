/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.command.Comm;

import sml.Identifier;
import sml.WMElement;
import sml.Agent;

/**
 * @author voigtjr
 *
 * Broadcasts a message to all listeners.
 */
public class SendMessageCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(SendMessageCommand.class);
	static final String NAME = "send-message";
	
	private static final String DESTINATION = "destination";
	private static final String FIRST = "first";
	private static final String WORD = "word";
	private static final String NEXT = "next";
	private static final String NIL = "nil";

	private final Identifier wme;
	private final List<String> tokens = new ArrayList<String>();
	private String destination;
	
	public SendMessageCommand(Identifier wme) {
		super(wme);
		this.wme = wme;
	}

	@Override
	protected boolean accept() {
		destination = wme.GetParameterValue(DESTINATION);

		try {
			Identifier next = wme.FindByAttribute(FIRST, 0).ConvertToIdentifier();
			logger.trace(FIRST + next);
			while (next != null) {
				WMElement word = next.FindByAttribute(WORD, 0);
				if (word == null) {
					addStatusError(WORD + " is null");
					return false;
				}
				logger.trace(WORD + ": " + word.GetValueAsString());
				tokens.add(word.GetValueAsString());
				
				WMElement nextwme = next.FindByAttribute(NEXT, 0);
				if (nextwme == null) {
					addStatusError(NEXT + " is null");
					return false;
				}
				logger.trace(NEXT + ": " + nextwme.GetValueAsString());
				if (nextwme.GetValueAsString().equals(NIL)) {
					break;
				}
				
				next = nextwme.ConvertToIdentifier();
				if (next == null) {
					addStatusError(NEXT + " is not identifier");
					return false;
				}
			}
			
		} catch (NullPointerException e) {
			addStatusError("malformed message");
			return false;
		}

		if (tokens.isEmpty()) {
			addStatusError("no message");
			return false;
		} 
		
		Iterator<String> iter = tokens.iterator();
		while (iter.hasNext()) {
			String token = iter.next();
			if (token.length() == 0) {
				iter.remove();
			}
		}
		
		addStatus(CommandStatus.ACCEPTED);
		return true;
	}
	
	@Override
	public void update(Adaptable app) {
		Comm comm = (Comm)app.getAdapter(Comm.class);
		Agent agent = (Agent)app.getAdapter(Agent.class);
		comm.sendMessage(agent.GetAgentName(), destination, tokens);
		addStatus(CommandStatus.COMPLETE);
	}
}
