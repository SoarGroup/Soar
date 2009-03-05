package org.msoar.sps.control;

import sml.Agent;
import sml.Identifier;
import sml.StringElement;

enum CommandStatus {
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
