package edu.umich.soar.sproom.command;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicBoolean;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.HzChecker;

import sml.Agent;
import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;

class SoarInterface implements Kernel.UpdateEventInterface, Kernel.SystemEventInterface {
	private static final Log logger = LogFactory.getLog(SoarInterface.class);

	private final HzChecker hzChecker = HzChecker.newInstance(SoarInterface.class.toString());
	private final Kernel kernel;
	private final Agent agent;

	private AtomicBoolean stopSoar = new AtomicBoolean(true);

	private DifferentialDriveCommand ddc;
	private DifferentialDriveCommand soarddc;
	
	private final ExecutorService exec = Executors.newSingleThreadExecutor();
	private Future<?> soarTask;
	
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
		
		agent.SetBlinkIfNoChange(false);
		
		// load productions
		String productions = CommandConfig.CONFIG.getProductions();
		if (productions != null && !agent.LoadProductions(productions)) {
			logger.error("Failed to load productions: " + productions);
			logger.error("Agent error: " + agent.GetLastErrorDescription());
			logger.error("Not shutting down, use debugger to fix.");
			agent.ExecuteCommandLine("waitsnc -e");
		}
		
		kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, null);

		agent.Commit();
	}
	
	void changeRunningState() {
		synchronized(exec) {
			if (soarTask == null || soarTask.isDone()) {
				logger.debug("Start Soar requested");
				stopSoar.set(false);
				soarTask = exec.submit(new Runnable() {
					@Override
					public void run() {
						kernel.RunAllAgentsForever();
					}
				});
			} else {
				if (stopSoar.compareAndSet(false, true)) {
					logger.debug("Stop Soar requested");
				}
			}
		}
	}
	
	public void systemEventHandler(int eventId, Object arg1, Kernel arg2) {
		if (eventId == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
			if (soarddc != null) {
				ddc = soarddc;
			}
			logger.info("Soar started.");
		} 
		else if (eventId == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
			ddc = DifferentialDriveCommand.newEStopCommand();
			logger.info("Soar stopped.");
		}
	}

	public void updateEventHandler(int eventID, Object data, Kernel kernel, int arg3) {
		if (logger.isDebugEnabled()) {
			hzChecker.tick();
		}
		
		if (stopSoar.get()) {
			logger.debug("Stopping Soar");
			kernel.StopAllAgents();
		}

		DifferentialDriveCommand newddc = null; //output.update();
		if (newddc != null) {
			soarddc = newddc;
			ddc = newddc;
		}
		
		//input.update();
		
		agent.Commit();
	}
	
	void shutdown() {
		synchronized(exec) {
			stopSoar.set(true);
			exec.shutdown();
		}
		
		kernel.Shutdown();
		kernel.delete();
		logger.info("Soar interface down");
	}

	DifferentialDriveCommand getDDCommand() {
		DifferentialDriveCommand temp = ddc;
		ddc = null;
		return temp;
	}
}
