package org.msoar.sps.control;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import sml.Agent;
import sml.Identifier;

final class ReceivedMessagesIL {
	private final Agent agent;
	private final Identifier receivedwme;
	private final Map<Integer, Identifier> messages = new HashMap<Integer, Identifier>();
	
	private int count = 0;

	ReceivedMessagesIL(Agent agent, Identifier receivedwme) {
		this.agent = agent;
		this.receivedwme = receivedwme;
	}

	void update(List<String> tokens) {
		if (tokens == null || tokens.size() == 0) {
			return;
		}
		
		Identifier message = agent.CreateIdWME(receivedwme, "message");
		Integer id = count++;
		agent.CreateStringWME(message, "from", "http"); // TODO support multiple message sources
		agent.CreateIntWME(message, "id", id);
		Identifier next = null;
		for (String word : tokens) {
			// first one is "first", rest are next
			next = (next == null) ? agent.CreateIdWME(message, "first") : agent.CreateIdWME(next, "next");
			agent.CreateStringWME(next, "word", word);
		}
		agent.CreateStringWME(next, "next", "nil");
		
		messages.put(id, message);
	}
	
	boolean remove(int id) {
		return messages.remove(Integer.valueOf(id)) != null;
	}
	
	void clear() {
		for (Identifier message : messages.values()) {
			agent.DestroyWME(message);
		}
		messages.clear();
	}
	
}
