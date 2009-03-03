package org.msoar.sps.control.o;

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
	
	StringElement addStatus(Agent agent, Identifier command) {
		return agent.CreateStringWME(command, STATUS, this.toString());
	}
}
