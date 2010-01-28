package edu.umich.soar.sproom.soar;

import edu.umich.soar.IntWme;
import edu.umich.soar.sproom.Adaptable;
import sml.Identifier;
import sml.Kernel;
import sml.smlSystemEventId;

public class TimeIL implements InputLinkElement{
	private static final long NANO_PER_SEC = 1000000000L;
	private static final String SECONDS = "seconds";
	private static final String MICROSECONDS = "microseconds";

	private final IntWme secondswme;
	private final IntWme microsecondswme;
	private long offset;
	private long stopTime;
	
	public TimeIL(Identifier root, Adaptable app) {
		secondswme = IntWme.newInstance(root, SECONDS);
		microsecondswme = IntWme.newInstance(root, MICROSECONDS);

		updateExact(0);

		Kernel kernel = (Kernel)app.getAdapter(Kernel.class);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, handler, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, handler, null);
	}
	
	private Kernel.SystemEventInterface handler = new Kernel.SystemEventInterface() {
		public void systemEventHandler(int eventId, Object arg1, Kernel arg2) {
			if (eventId == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
				offset += System.nanoTime() - stopTime;
			} else if (eventId == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
				stopTime = System.nanoTime();
			}
		}
	};
	
	@Override
	public void update(Adaptable app) {
		updateExact(System.nanoTime() - offset);
	}
	
	private void updateExact(long nanoTime) {
		int seconds = (int) (nanoTime / NANO_PER_SEC);
		int microseconds = (int) (nanoTime % NANO_PER_SEC);
		microseconds /= 1000;

		secondswme.update(seconds);
		microsecondswme.update(microseconds);
	}
	
}

