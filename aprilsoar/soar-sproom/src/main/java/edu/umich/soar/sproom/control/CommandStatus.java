package edu.umich.soar.sproom.control;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import sml.Identifier;
import sml.StringElement;

public enum CommandStatus {
	accepted,
	executing,
	complete,
	error,
	interrupted;
	
	private static final Log logger = LogFactory.getLog(CommandStatus.class);
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
