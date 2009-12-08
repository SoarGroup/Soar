package edu.umich.soar.sproom;

import java.util.TimerTask;

import org.apache.commons.logging.Log;

public class HzChecker extends TimerTask {
	private int ticks = 0;
	private long time = 0;
	private final Log logger;
	
	public HzChecker(Log logger) {
		this.logger = logger;
	}
	
	public synchronized void tick() {
		ticks += 1;
	}
	
	public synchronized void run() {
		double hz = 0;
		long dtime = System.currentTimeMillis() - time;
		if (dtime != 0) {
		    hz = ticks / (dtime / 1000.0);
		}
	    ticks = 0;
	    time += dtime;
	    logger.debug(String.format("Running at %.3f hz", hz));
	}
}
