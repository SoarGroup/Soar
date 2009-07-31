package edu.umich.soar.robot;

import java.util.List;

public interface ReceiveMessagesInterface {
	void newMessage(String from, List<String> tokens);
	void clearMessages();
	boolean removeMessage(int id);
}
