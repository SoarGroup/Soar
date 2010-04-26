package edu.umich.soar.sproom.comm;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Messages
{
	private final Map<String, CommReceiver> receivers = new HashMap<String, CommReceiver>();
	
	/**
	 * See Map.put(). Null key means receive all messages no matter what destination.
	 * 
	 * @param name key
	 * @param comm value
	 * @return
	 */
	public CommReceiver registerReceiver(String name, CommReceiver comm) {
		return receivers.put(name, comm);
	}
	
	/**
	 * @param from
	 * @param destination null for broadcast
	 * @param message will be split up on spaces
	 * @return see other send message, also returns false if split yields no tokens
	 */
	public boolean sendMessage(String from, String destination, String message) {
		List<String> tokens = new ArrayList<String>();

		for (String token : message.trim().split("\\s")) {
			token = token.trim();
			if (token.isEmpty()) {
				continue;
			}
			tokens.add(token);
		}
		
		if (tokens.isEmpty()) {
			return false;
		}
		
		return sendMessage(from, destination, tokens);
	}
	
	/**
	 * @param from
	 * @param destination null for broadcast
	 * @param tokens
	 * @return false if unknown destination (and not broadcast)
	 */
	public boolean sendMessage(String from, String destination, List<String> tokens) {
		Message message = (destination == null) ? new Message(from, tokens) : new Message(from, destination, tokens);

		if (destination == null) {
			// broadcast
			for (CommReceiver c : receivers.values()) {
				c.receiveMessage(message);
			}
		} else {
			// null receiver gets everything
			CommReceiver c = receivers.get(null);
			if (c != null) {
				c.receiveMessage(message);
			}
			
			c = receivers.get(destination);
			if (c != null) {
				c.receiveMessage(message);
			} else {
				// TODO: warning message: unknown destination
				return false;
			}
		}
		return true;
	}
}
