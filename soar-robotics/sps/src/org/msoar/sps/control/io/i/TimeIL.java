package org.msoar.sps.control.io.i;

import sml.Agent;
import sml.Identifier;
import sml.IntElement;

class TimeIL {
	private IntElement secondswme;
	private IntElement microsecondswme;
	private final static long nanosecondsPerSecond = 1000000000;
	private Agent agent;
	
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

