package org.msoar.sps.control;

import java.util.Arrays;
import java.util.List;
import java.util.Timer;

import org.apache.log4j.Logger;
import org.msoar.sps.HzChecker;
import edu.umich.soar.config.Config;

import sml.Agent;
import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;

final class SoarInterface implements Kernel.UpdateEventInterface, Kernel.SystemEventInterface {
	private static final Logger logger = Logger.getLogger(SoarInterface.class);

	private final static int DEFAULT_RANGES_COUNT = 5;
	
	static SoarInterface newInstance(Config config, SplinterState splinter) {
		return new SoarInterface(config, splinter);
	}

	private final Timer timer = new Timer();
	private final HzChecker hzChecker = new HzChecker(logger);
	private final Kernel kernel;
	private final Agent agent;
	private final InputLinkManager input;
	private final OutputLinkManager output;

	private boolean stopSoar = false;
	private boolean running = false;
	private List<String> tokens;
	private DifferentialDriveCommand ddc;
	private DifferentialDriveCommand soarddc;
	
	private SoarInterface(Config config, SplinterState splinter) {
		kernel = Kernel.CreateKernelInNewThread();
		if (kernel.HadError()) {
			logger.error("Soar error: " + kernel.GetLastErrorDescription());
			System.exit(1);
		}

		kernel.SetAutoCommit(false);

		agent = kernel.CreateAgent("soar");
		if (kernel.HadError()) {
			logger.error("Soar error: " + kernel.GetLastErrorDescription());
			System.exit(1);
		}

		// load productions
		String productions = config.getString("productions");
		if (productions != null && !agent.LoadProductions(productions)) {
			logger.error("Failed to load productions: " + productions);
			logger.error("Agent error: " + agent.GetLastErrorDescription());
			logger.error("Not shutting down, use debugger to fix.");
			agent.ExecuteCommandLine("waitsnc -e");
		}
		
		int rangesCount = config.getInt("ranges_count", DEFAULT_RANGES_COUNT);
		input = new InputLinkManager(agent, kernel, rangesCount, splinter);
		output = new OutputLinkManager(agent, input.getInterface(), splinter);
		
		kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, null);
		
		if (logger.isDebugEnabled()) {
			timer.schedule(hzChecker, 0, 5000); 
		}
	}
	
	private class SoarRunner implements Runnable {
		public void run() {
			kernel.RunAllAgentsForever();
		}
	}

	void changeRunningState() {
		if (running) {
			if (!stopSoar) {
				stopSoar = true;
				logger.debug("Stop Soar requested");
			}
		} else {
			Thread soarThread = new Thread(new SoarRunner());
			stopSoar = false;
			soarThread.start();
			logger.debug("Start Soar requested");
		}
	}

	public void updateEventHandler(int eventID, Object data, Kernel kernel, int arg3) {
		try {
			if (logger.isDebugEnabled()) {
				hzChecker.tick();
			}
			
			if (stopSoar) {
				logger.debug("Stopping Soar");
				kernel.StopAllAgents();
			}
	
			DifferentialDriveCommand newddc = output.update();
			if (newddc != null) {
				soarddc = newddc;
				ddc = newddc;
			}
			
			synchronized (this) {
				input.update(tokens, output.getUseFloatYawWmes());
				tokens = null;
			}
			
			agent.Commit();
		} catch (Exception e) {
			e.printStackTrace();
			logger.error("Uncaught exception in Soar update thread: " + e);
		}
	}
	
	void shutdown() {
		stopSoar = true;
		try {
			Thread.sleep(1000);
		} catch (InterruptedException ignored) {
		}

		kernel.Shutdown();
		kernel.delete();
		logger.info("Soar interface down");
	}

	public void systemEventHandler(int eventId, Object arg1, Kernel arg2) {
		if (eventId == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
			logger.info("Soar started.");
			if (soarddc != null) {
				ddc = soarddc;
			}
			running = true;
		} else if (eventId == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
			logger.info("Soar stopped.");
			ddc = DifferentialDriveCommand.newEStopCommand();
			running = false;
		}
	}

	void setStringInput(List<String> tokens) {
		List<String> warn = null;
		synchronized (this) {
			warn = (this.tokens != null ? this.tokens : null);
			this.tokens = tokens;
		}
		if (warn != null) {
			logger.warn("Overwriting message " 
					+ Arrays.toString(warn.toArray(new String[warn.size()]))
					+ " with "
					+ Arrays.toString(tokens.toArray(new String[tokens.size()])));
		}
	}

	boolean hasDDCommand() {
		return ddc != null;
	}

	DifferentialDriveCommand getDDCommand() {
		DifferentialDriveCommand temp = ddc;
		ddc = null;
		return temp;
	}
}
