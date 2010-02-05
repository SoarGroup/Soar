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
	private boolean complete = false;
	private final List<String> tokens = new ArrayList<String>();
	private String destination;
	
	public SendMessageCommand(Identifier wme) {
		super(Integer.valueOf(wme.GetTimeTag()));
		this.wme = wme;
	}

	@Override
	public String getName() {
		return NAME;
	}
	
	@Override
	public OutputLinkCommand accept() {
		destination = wme.GetParameterValue(DESTINATION);

		try {
			Identifier next = wme.FindByAttribute(FIRST, 0).ConvertToIdentifier();
			logger.trace(FIRST + next);
			while (next != null) {
				WMElement word = next.FindByAttribute(WORD, 0);
				if (word == null) {
					return new InvalidCommand(wme, WORD + " is null");
				}
				logger.trace(WORD + ": " + word.GetValueAsString());
				tokens.add(word.GetValueAsString());
				
				WMElement nextwme = next.FindByAttribute(NEXT, 0);
				if (nextwme == null) {
					return new InvalidCommand(wme, NEXT + " is null");
				}
				logger.trace(NEXT + ": " + nextwme.GetValueAsString());
				if (nextwme.GetValueAsString().equals(NIL)) {
					break;
				}
				
				next = nextwme.ConvertToIdentifier();
				if (next == null) {
					return new InvalidCommand(wme, NEXT + " is not identifier");
				}
			}
			
		} catch (NullPointerException e) {
			return new InvalidCommand(wme, "malformed message");
		}

		if (tokens.isEmpty()) {
			return new InvalidCommand(wme, "no message");
		} 
		
		Iterator<String> iter = tokens.iterator();
		while (iter.hasNext()) {
			String token = iter.next();
			if (token.length() == 0) {
				iter.remove();
			}
		}
		
		CommandStatus.accepted.addStatus(wme);
		return this;
	}
	
	@Override
	public void update(Adaptable app) {
		if (!complete) {
			Comm comm = (Comm)app.getAdapter(Comm.class);
			Agent agent = (Agent)app.getAdapter(Agent.class);
			comm.sendMessage(agent.GetAgentName(), destination, tokens);
			CommandStatus.complete.addStatus(wme);
			complete = true;
		}
	}
}
