package edu.umich.soar.robot;

import sml.Identifier;
import sml.IntElement;
import sml.Kernel;
import sml.smlSystemEventId;

final public class TimeIL implements Kernel.SystemEventInterface {
	private final static long nanosecondsPerSecond = 1000000000L;

	private final Identifier time;
	private final IntElement secondswme;
	private final IntElement microsecondswme;
	private long offset = 0;
	private long stopTime = 0;
	
	public TimeIL(Identifier time) {
		this.time = time;
		secondswme = time.CreateIntWME("seconds", 0);
		microsecondswme = time.CreateIntWME("microseconds", 0);

		updateInternal(0);
	}
	
	public void destroy() {
		time.DestroyWME();
	}

	public void update() {
		updateInternal(System.nanoTime() - offset);
	}
	
	private void updateInternal(long nanoTime) {
		int seconds = (int) (nanoTime / nanosecondsPerSecond);
		int microseconds = (int) (nanoTime % nanosecondsPerSecond);
		microseconds /= 1000;

		secondswme.Update(seconds);
		microsecondswme.Update(microseconds);
	}
	
	public void systemEventHandler(int eventId, Object arg1, Kernel arg2) {
		if (eventId == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
			offset += System.nanoTime() - stopTime;
		} else if (eventId == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
			stopTime = System.nanoTime();
		}
	}
}

