package org.msoar.sps.control;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;

import org.apache.log4j.Logger;
import org.msoar.sps.Names;
import org.msoar.sps.control.i.InputLinkManager;
import org.msoar.sps.control.o.OutputLinkManager;

import sml.Agent;
import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.differential_drive_command_t;
import lcmtypes.laser_t;
import lcmtypes.pose_t;

public class SoarInterface implements Kernel.UpdateEventInterface, Kernel.SystemEventInterface, LCMSubscriber {
	private static final Logger logger = Logger.getLogger(SoarInterface.class);

	private Kernel kernel;
	private Agent agent;
	private InputLinkManager input;
	private OutputLinkManager output;
	private boolean stopSoar = false;
	private boolean running = false;
	private pose_t pose;
	private laser_t laser;
	private static long DCDELAY_THRESHOLD_USEC = 250000L; // 250 ms
	private boolean failsafeSpew = false;
	private List<String> tokens;
	
	SoarInterface(String productions, int rangesCount) {
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
		if (productions != null && !agent.LoadProductions(productions)) {
			logger.error("Failed to load productions: " + productions);
			logger.error("Agent error: " + agent.GetLastErrorDescription());
			logger.error("Not shutting down, use debugger to fix.");
			agent.ExecuteCommandLine("waitsnc -e");
		}
		
		LCM lcm = LCM.getSingleton();
		lcm.subscribe(Names.POSE_CHANNEL, this);
		lcm.subscribe(Names.LASER_CHANNEL, this);
		
		input = new InputLinkManager(agent, rangesCount);
		output = new OutputLinkManager(agent, input.getWaypointsIL(), input.getMessagesIL());
		
		kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, null);
	}
	
	private void failSafe(differential_drive_command_t dc) {
		dc.left_enabled = true;
		dc.right_enabled = true;
		dc.left = 0;
		dc.right = 0;
		dc.utime = getCurrentUtime();
	}
	void getDC(differential_drive_command_t dc) {
		if (!output.getDC(dc, input.getYawRadians())) {
			failSafe(dc);
			return;
		}

		// is it timely
		long dcDelay = getCurrentUtime() - dc.utime;
		if (dcDelay > DCDELAY_THRESHOLD_USEC) {
			// not timely, fail-safe
			if (failsafeSpew == false) {
				logger.error("Obsolete drive command " + dcDelay + " usec");
				failsafeSpew = true;
			}
			failSafe(dc);
			return;
		}
		
		if (failsafeSpew) {
			logger.info("Receiving valid commands again, leaving fail-safe.");
			failsafeSpew = false;
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

	public void updateEventHandler(int arg0, Object arg1, Kernel arg2, int arg3) {
		if (stopSoar) {
			logger.debug("Stopping Soar");
			kernel.StopAllAgents();
		}

		output.update(pose, getCurrentUtime());
		
		synchronized (this) {
			input.update(pose, laser, tokens, output.getUseFloatYawWmes());
			tokens = null;
		}
		
		agent.Commit();
	}
	
	long getCurrentUtime() {
		return System.nanoTime() / 1000L;
	}

	public void shutdown() {
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
			running = true;
		} else if (eventId == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
			logger.info("Soar stopped.");
			running = false;
		}
	}

	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(Names.POSE_CHANNEL)) {
			try {
				pose = new pose_t(ins);
			} catch (IOException e) {
				logger.error("Error decoding pose_t message: " + e.getMessage());
			}
		} else if (channel.equals(Names.LASER_CHANNEL)) {
			try {
				laser = new laser_t(ins);
			} catch (IOException e) {
				logger.error("Error decoding laser_t message: " + e.getMessage());
			}
		}
	}

	public void setStringInput(List<String> tokens) {
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

}
