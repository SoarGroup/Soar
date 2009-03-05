package org.msoar.sps.control;

import sml.Agent;
import sml.Identifier;
import sml.IntElement;

class TimeIL {
	private final static long nanosecondsPerSecond = 1000000000L;

	private final Agent agent;
	private final IntElement secondswme;
	private final IntElement microsecondswme;
	
	TimeIL(Agent agent, Identifier time) {
		this.agent = agent;
		secondswme = agent.CreateIntWME(time, "seconds", 0);
		microsecondswme = agent.CreateIntWME(time, "microseconds", 0);

		update();
	}

	void update() {
		long current = System.nanoTime();
		int seconds = (int) (current / nanosecondsPerSecond);
		int microseconds = (int) (current % nanosecondsPerSecond);
		microseconds /= 1000;

		agent.Update(secondswme, seconds);
		agent.Update(microsecondswme, microseconds);
	}
}

