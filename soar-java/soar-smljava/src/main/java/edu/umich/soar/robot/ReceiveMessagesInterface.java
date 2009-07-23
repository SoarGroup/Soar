package edu.umich.soar.robot;

import java.util.List;

public interface ReceiveMessagesInterface {
	void newMessage(String from, String to, List<String> tokens);
	void clearMessages();
	boolean removeMessage(int id);
}
