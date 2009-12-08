package edu.umich.soar.sproom.control;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;

import sml.Identifier;

public final class MessagesIL implements ReceiveMessagesInterface {
	private final Identifier receivedwme;
	private final Map<Integer, Identifier> messages = new HashMap<Integer, Identifier>();
	private final Queue<Message> queuedMessages = new LinkedList<Message>();
	private int count = 0;
	
	private static class Message {
		String from;
		List<String> tokens;
		
		Message(String from, List<String> tokens) {
			this.from = from;
			this.tokens = new ArrayList<String>();
			this.tokens.addAll(tokens);
		}
	}

	public MessagesIL(Identifier receivedwme) {
		this.receivedwme = receivedwme;
	}
	
	void destroy() {
		receivedwme.DestroyWME();
	}

	public void update() {
		for (Message message : queuedMessages) {
			Identifier messageWme = receivedwme.CreateIdWME("message");
			Integer id = count++;
			messageWme.CreateStringWME("from", message.from);
			messageWme.CreateIntWME("id", id);
			Identifier next = null;
			for (String word : message.tokens) {
				// first one is "first", rest are next
				next = (next == null) ? messageWme.CreateIdWME("first") : next.CreateIdWME("next");
				next.CreateStringWME("word", word);
			}
			next.CreateStringWME("next", "nil");
			
			messages.put(id, messageWme);
		}
		queuedMessages.clear();
	}
	
	@Override
	public void clearMessages() {
		for (Identifier message : messages.values()) {
			message.DestroyWME();
		}
		messages.clear();
	}

	@Override
	public void newMessage(String from, List<String> tokens) {
		if (tokens == null || tokens.size() == 0) {
			return;
		}
		
		queuedMessages.add(new Message(from, tokens));
	}

	@Override
	public boolean removeMessage(int id) {
		Identifier message = messages.get(Integer.valueOf(id));
		if (message == null) {
			return false;
		}
		
		message.DestroyWME();
		messages.remove(Integer.valueOf(id));
		return true;
	}
	
}
