package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;

import lcmtypes.differential_drive_command_t;

public class SoarInterface implements Kernel.UpdateEventInterface, Kernel.SystemEventInterface {
	private static Logger logger = Logger.getLogger(SoarInterface.class);

	private Kernel kernel;
	private Agent agent;
	private InputLinkManager input;
	private OutputLinkManager output;
	private boolean stopSoar = false;
	private boolean running = false;
	
	SoarInterface(String productions) {
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
		if (!agent.LoadProductions(productions)) {
			logger.error("Failed to load productions: " + productions);
			logger.error("Agent error: " + agent.GetLastErrorDescription());
			logger.error("Not shutting down, use debugger to fix.");
			agent.ExecuteCommandLine("waitsnc -e");
		}
		
		input = new InputLinkManager(agent);
		output = new OutputLinkManager(agent);
		
		kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, null);
	}
	
	void getDC(differential_drive_command_t dc) {
		output.getDC(dc, input.getYaw());
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

	@Override
	public void updateEventHandler(int arg0, Object arg1, Kernel arg2, int arg3) {
		if (stopSoar) {
			logger.debug("Stopping Soar");
			kernel.StopAllAgents();
		}

		input.update();
		output.update();
		
		agent.Commit();
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

	@Override
	public void systemEventHandler(int eventId, Object arg1, Kernel arg2) {
		if (eventId == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
			logger.info("Soar started.");
			running = true;
		} else if (eventId == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
			logger.info("Soar stopped.");
			running = false;
		}
	}
}
