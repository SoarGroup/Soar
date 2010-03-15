package edu.umich.soar.sproom;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Debug class for monitoring the rate something is happening.
 *
 * @author voigtjr@gmail.com
 */
public class HzChecker {
	private static final Log logger = LogFactory.getLog(HzChecker.class);
	private static final List<HzChecker> checkers = new CopyOnWriteArrayList<HzChecker>();
	private static final ScheduledExecutorService schexec = Executors.newSingleThreadScheduledExecutor();
	private static final AtomicBoolean started = new AtomicBoolean(false);
	
	public static HzChecker newInstance(String component) {
		HzChecker checker = new HzChecker(component);
		checkers.add(checker);
		if (logger.isDebugEnabled() && !started.getAndSet(true)) {
			schexec.scheduleAtFixedRate(new Runnable() {
				@Override
				public void run() {
					for (HzChecker hc : checkers) {
					    logger.debug(String.format("%s running at %.3f hz", hc.component, hc.report()));
					}
				}
			}, 5, 5, TimeUnit.SECONDS);
		}
		return checker;
	}
	
	private final String component;
	private final AtomicInteger ticks = new AtomicInteger(0);
	private long time = 0;

	private HzChecker(String component) {
		this.component = component;
	}
	
	public void tick() {
		ticks.incrementAndGet();
	}
	
	private double report() {
		int t = ticks.getAndSet(0);
		double hz = 0;
		long dtime = System.currentTimeMillis() - time;
		if (dtime != 0) {
		    hz = t / (dtime / 1000.0);
		}
	    time += dtime;
	    return hz;
	}
}
