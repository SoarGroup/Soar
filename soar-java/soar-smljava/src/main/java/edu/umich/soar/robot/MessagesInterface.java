package edu.umich.soar.robot;

public interface MessagesInterface {
	void newMessage(String dest, String message);
	void clearMessages();
	boolean removeMessage(int id);
}
