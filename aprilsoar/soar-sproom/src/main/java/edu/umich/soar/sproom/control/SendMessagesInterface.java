package edu.umich.soar.sproom.control;

import java.util.List;

public interface SendMessagesInterface {
	void sendMessage(String from, String to, List<String> tokens);
}
