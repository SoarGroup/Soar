package edu.umich.soar.sproom.control;

import sml.Identifier;
import sml.IntElement;
import sml.Kernel;
import sml.smlSystemEventId;

public class TimeIL implements Kernel.SystemEventInterface {
	private final static long NANO_PER_SEC = 1000000000L;

	private final Identifier time;
	private final IntElement secondswme;
	private final IntElement microsecondswme;
	private long offset;
	private long stopTime;
	private boolean invalid = false;
	
	public TimeIL(Identifier time) {
		this.time = time;
		secondswme = time.CreateIntWME("seconds", 0);
		microsecondswme = time.CreateIntWME("microseconds", 0);

		updateExact(0);
	}
	
	public void destroy() {
		time.DestroyWME();
		invalid = true;
	}

	public void update() {
		if (invalid)
			throw new IllegalStateException("already destroyed");
		updateExact(System.nanoTime() - offset);
	}
	
	public void updateExact(long nanoTime) {
		if (invalid)
			throw new IllegalStateException("already destroyed");
		int seconds = (int) (nanoTime / NANO_PER_SEC);
		int microseconds = (int) (nanoTime % NANO_PER_SEC);
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

