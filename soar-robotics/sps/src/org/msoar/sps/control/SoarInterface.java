package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Kernel;

import lcmtypes.differential_drive_command_t;

public class SoarInterface implements Kernel.UpdateEventInterface {
	private static Logger logger = Logger.getLogger(SoarInterface.class);

	private Kernel kernel;
	private Agent agent;
	private InputLinkManager input;
	private OutputLinkManager output;
	private boolean stopSoar = false;
	
	SoarInterface() {
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

		input = new InputLinkManager(agent);
		output = new OutputLinkManager(agent);
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
		if (kernel.IsSoarRunning()) {
			if (!stopSoar) {
				stopSoar = true;
				logger.info("Stop Soar requested");
			}
		} else {
			Thread soarThread = new Thread(new SoarRunner());
			stopSoar = false;
			soarThread.start();
			logger.info("Start Soar requested");
		}
	}

	@Override
	public void updateEventHandler(int arg0, Object arg1, Kernel arg2, int arg3) {
		if (stopSoar) {
			logger.info("Stopping Soar");
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
}
