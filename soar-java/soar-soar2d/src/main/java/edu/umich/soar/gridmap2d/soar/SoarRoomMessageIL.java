package edu.umich.soar.gridmap2d.soar;

import edu.umich.soar.gridmap2d.Gridmap2D;

import sml.Agent;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;

class SoarRoomMessageIL {
	Agent agent;
	Identifier parent;
	StringElement from, message;
	IntElement cycle;

	int cycleCreated;

	SoarRoomMessageIL(Agent agent, Identifier parent) {
		this.agent = agent;
		this.parent = parent;
	}
	
	void initialize(String from, String message) {
		this.from = agent.CreateStringWME(parent, "from", from);
		this.message = agent.CreateStringWME(parent, "message", message);
		this.cycleCreated = Gridmap2D.simulation.getWorldCount();
		this.cycle = agent.CreateIntWME(parent, "cycle", this.cycleCreated); 
	}
}

