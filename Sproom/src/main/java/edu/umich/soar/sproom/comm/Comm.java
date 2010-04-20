package edu.umich.soar.sproom.comm;

import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * @author voigtjr@gmail.com
 */
public class Comm implements CommReceiver 
{
	private final Map<Long, Message> history = new ConcurrentHashMap<Long, Message>();
	private final String self;
	private final Messages messages;
	
	public Comm(String self, Messages messages) 
	{
		this.self = self;
		this.messages = messages;
		
		if (messages.registerReceiver(self, this) != null) {
			// TODO: warn
		}
	}
	
	public boolean sendMessage(String destination, List<String> tokens) 
	{
		return messages.sendMessage(self, destination, tokens);
	}

	public void clearMessages() 
	{
		history.clear();
	}

	public Message removeMessage(long id) 
	{
		return history.remove(Long.valueOf(id));
	}
	
	public Collection<Message> getMessages() 
	{
		return Collections.unmodifiableCollection(history.values());
	}

	@Override
	public void receiveMessage(Message message)
	{
		history.put(message.getId(), message);
	}
}
