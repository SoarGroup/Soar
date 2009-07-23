package edu.umich.soar.robot;

import sml.Agent;
import sml.Identifier;
import sml.StringElement;

public enum CommandStatus {
	accepted,
	executing,
	complete,
	error,
	interrupted;
	
	private static final String STATUS = "status";
	
	public StringElement addStatus(Agent agent, Identifier command) {
		return agent.CreateStringWME(command, STATUS, this.toString());
	}
}
