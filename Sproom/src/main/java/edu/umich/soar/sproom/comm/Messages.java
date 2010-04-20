package edu.umich.soar.sproom.comm;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Messages
{
	private final Map<String, CommReceiver> receivers = new HashMap<String, CommReceiver>();
	
	/**
	 * See Map.put()
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
			CommReceiver c = receivers.get(destination);
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
