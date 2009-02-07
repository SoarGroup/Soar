package org.msoar.sps.control.io;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;

class RangeIL {
	private Identifier rangewme;
	private FloatElement startwme;
	private FloatElement endwme;
	private FloatElement distancewme;
	private Agent agent;

	RangeIL(Agent agent, Identifier rangeswme, int id) {
		this.agent = agent;
		rangewme = agent.CreateIdWME(rangeswme, "range");
		agent.CreateIntWME(rangewme, "id", id);
	}

	void update(double start, double end, double distance) {
		start = Math.toDegrees(start);
		end = Math.toDegrees(end);
		
		if (startwme == null) {
			startwme = agent.CreateFloatWME(rangewme, "start", start);
		} else {
			agent.Update(startwme, start);
		}
		
		if (endwme == null) {
			endwme = agent.CreateFloatWME(rangewme, "end", end);
		} else {
			agent.Update(endwme, end);
		}
		
		if (distancewme == null) {
			distancewme = agent.CreateFloatWME(rangewme, "distance", distance);
		} else {
			agent.Update(distancewme, distance);
		}
	}
}
