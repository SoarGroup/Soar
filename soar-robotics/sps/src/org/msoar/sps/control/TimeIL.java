package org.msoar.sps.control;

import sml.Agent;
import sml.Identifier;
import sml.IntElement;
import sml.Kernel;
import sml.smlSystemEventId;

final class TimeIL implements Kernel.SystemEventInterface {
	private final static long nanosecondsPerSecond = 1000000000L;

	private final Agent agent;
	private final IntElement secondswme;
	private final IntElement microsecondswme;
	private long offset = 0;
	private long stopTime = 0;
	
	TimeIL(Agent agent, Identifier time) {
		this.agent = agent;
		secondswme = agent.CreateIntWME(time, "seconds", 0);
		microsecondswme = agent.CreateIntWME(time, "microseconds", 0);

		updateInternal(0);
	}

	void update() {
		updateInternal(System.nanoTime() - offset);
	}
	
	private void updateInternal(long nanoTime) {
		int seconds = (int) (nanoTime / nanosecondsPerSecond);
		int microseconds = (int) (nanoTime % nanosecondsPerSecond);
		microseconds /= 1000;

		agent.Update(secondswme, seconds);
		agent.Update(microsecondswme, microseconds);
	}
	
	public void systemEventHandler(int eventId, Object arg1, Kernel arg2) {
		if (eventId == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
			offset += System.nanoTime() - stopTime;
		} else if (eventId == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
			stopTime = System.nanoTime();
		}
	}
}

