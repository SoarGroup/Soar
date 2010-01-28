package edu.umich.soar.sproom.command;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Comm {
	private final Map<Long, CommMessage> messages = new ConcurrentHashMap<Long, CommMessage>();
	
	public void sendMessage(String from, String destination,
			List<String> tokens) {
		CommMessage message = (destination == null) ? new CommMessage(from, tokens) : new CommMessage(from, destination, tokens);
		messages.put(message.getId(), message);
	}

	public void clearMessages() {
		messages.clear();
	}

	public void removeMessage(long id) {
		messages.remove(Long.valueOf(id));
	}
	
	public Map<Long, CommMessage> getMessages() {
		return Collections.unmodifiableMap(messages);
	}
}
