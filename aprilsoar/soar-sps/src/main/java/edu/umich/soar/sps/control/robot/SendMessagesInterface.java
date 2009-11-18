package edu.umich.soar.sps.control.robot;

import java.util.List;

public interface SendMessagesInterface {
	void sendMessage(String from, String to, List<String> tokens);
}
