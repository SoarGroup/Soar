package edu.umich.soar.sproom.comm;

import java.util.Arrays;

public class MessagesTest
{
	private static final String A = "a";
	private static final String B = "b";
	private static final String C = "c";
	
	private final Messages messages = new Messages();
	
	private MessagesTest() {
		Comm a = new Comm(A, messages);
		Comm b = new Comm(B, messages);
		Comm c = new Comm(C, messages);
		
		assert a.getMessages().isEmpty();
		assert b.getMessages().isEmpty();
		assert c.getMessages().isEmpty();
		
		a.sendMessage(null, Arrays.asList(new String[] {"foo", "bar" }));
		
		assert !a.getMessages().isEmpty();
		assert !b.getMessages().isEmpty();
		assert !c.getMessages().isEmpty();
		
		a.sendMessage(B, Arrays.asList(new String[] {"a", "b" }));
		a.sendMessage(C, Arrays.asList(new String[] {"a", "c" }));
		a.removeMessage(0);
		
		b.sendMessage(A, Arrays.asList(new String[] {"b", "a" }));
		b.sendMessage(C, Arrays.asList(new String[] {"b", "c" }));
		b.removeMessage(0);
		
		c.sendMessage(A, Arrays.asList(new String[] {"c", "a" }));
		c.sendMessage(B, Arrays.asList(new String[] {"c", "b" }));
		c.removeMessage(0);
		
		for (Message m : a.getMessages()) {
			System.out.println(A + ": " + m);
		}
		for (Message m : b.getMessages()) {
			System.out.println(B + ": " + m);
		}
		for (Message m : c.getMessages()) {
			System.out.println(C + ": " + m);
		}		
		
		a.clearMessages();
		b.clearMessages();
		c.clearMessages();
		
		assert a.getMessages().isEmpty();
		assert b.getMessages().isEmpty();
		assert c.getMessages().isEmpty();
		
		try {
			a.sendMessage(null, null);
		} catch (NullPointerException e) {}
		
		assert a.removeMessage(200) == null;
		
		Comm d = new Comm(A, messages);

	}
	
	public static void main(String[] args) {
		new MessagesTest();
	}
}

	