package edu.umich.soar.sps;

import java.util.TimerTask;

import org.apache.log4j.Logger;

public class HzChecker extends TimerTask {
	private int ticks = 0;
	private long time = 0;
	private final Logger logger;
	
	public HzChecker(Logger logger) {
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
