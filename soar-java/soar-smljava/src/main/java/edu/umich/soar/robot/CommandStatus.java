package edu.umich.soar.robot;

import org.apache.log4j.Logger;

import sml.Identifier;
import sml.StringElement;

public enum CommandStatus {
	accepted,
	executing,
	complete,
	error,
	interrupted;
	
	private static final Logger logger = Logger.getLogger(CommandStatus.class);
	private static final String STATUS = "status";
	
	public StringElement addStatus(Identifier command) {
		return command.CreateStringWME(STATUS, this.toString());
	}
	
	public StringElement addStatus(Identifier command, String message) {
		logger.info("Command message: " + message);
		command.CreateStringWME("message", message);
		return command.CreateStringWME(STATUS, this.toString());
	}
}
