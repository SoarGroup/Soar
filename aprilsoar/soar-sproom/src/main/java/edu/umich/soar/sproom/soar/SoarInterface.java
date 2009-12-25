package edu.umich.soar.sproom.soar;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicBoolean;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.HzChecker;
import edu.umich.soar.sproom.command.Comm;
import edu.umich.soar.sproom.command.CommandConfig;
import edu.umich.soar.sproom.command.Pose;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.DriveListener;
import edu.umich.soar.sproom.soar.OutputLink.OutputLinkActions;
import edu.umich.soar.sproom.wp.Waypoints;

import sml.Agent;
import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;

public class SoarInterface implements SoarControlListener, Adaptable {
	private static final Log logger = LogFactory.getLog(SoarInterface.class);

	private final HzChecker hzChecker = HzChecker.newInstance(SoarInterface.class.toString());
	private final Kernel kernel;
	private final Agent agent;
	private final AtomicBoolean stopSoar = new AtomicBoolean(true);
	private final ExecutorService exec = Executors.newSingleThreadExecutor();
	private Future<?> soarTask;
	private DifferentialDriveCommand ddcPrev;
	private InputLink il;
	private OutputLink ol;
	private final Pose pose;
	private final Waypoints waypoints;
	private final Comm comm;
	private final Adaptable app; // self-reference for handler code
	
	public SoarInterface(Pose pose, Waypoints waypoints, Comm comm) {
		this.pose = pose;
		this.waypoints = waypoints;
		this.comm = comm;
		this.app = this;
		
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
		
		il = InputLink.newInstance(this);
		ol = OutputLink.newInstance(this);
		
		// load productions
		String productions = CommandConfig.CONFIG.getProductions();
		if (productions != null && !agent.LoadProductions(productions)) {
			logger.error("Failed to load productions: " + productions);
			logger.error("Agent error: " + agent.GetLastErrorDescription());
			logger.error("Not shutting down, use debugger to fix.");
			agent.ExecuteCommandLine("waitsnc -e");
		}
		
		kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, updateHandler, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, systemHandler, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, systemHandler, null);

		agent.Commit();
	}
	
	Kernel.SystemEventInterface systemHandler = new Kernel.SystemEventInterface() {
		@Override
		public void systemEventHandler(int eventId, Object arg1, Kernel arg2) {
			if (eventId == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
				if (ddcPrev != null) {
					fireDriveEvent(ddcPrev);
				}
				logger.info("Soar started.");
			} 
			else if (eventId == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
				fireDriveEvent(DifferentialDriveCommand.newEStopCommand());
				logger.info("Soar stopped.");
			}
		}
	};
	
	Kernel.UpdateEventInterface updateHandler = new Kernel.UpdateEventInterface() {
		public void updateEventHandler(int eventID, Object data, Kernel kernel, int arg3) {
			if (logger.isDebugEnabled()) {
				hzChecker.tick();
			}
			
			if (stopSoar.get()) {
				logger.debug("Stopping Soar");
				kernel.StopAllAgents();
			}

			OutputLinkActions actions = ol.update();
			if (actions.getDDC() != null) {
				ddcPrev = actions.getDDC();
				fireDriveEvent(actions.getDDC());
			}
			
			il.update(app);
			agent.Commit();
		}
	};
	
	private final List<DriveListener> driveListeners = new CopyOnWriteArrayList<DriveListener>();
	public void addDriveListener(DriveListener driveListener) {
		driveListeners.add(driveListener);
	}
	
	boolean removeDriveListener(DriveListener driveListener) {
		return driveListeners.remove(driveListener);
	}
	
	private void fireDriveEvent(DifferentialDriveCommand ddc) {
		for (DriveListener listener : driveListeners) {
			listener.handleDriveEvent(ddc);
		}
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

	@Override
	public void toggleRunState() {
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

	@Override
	public Object getAdapter(Class<?> klass) {
		if (klass == Agent.class) {
			return agent;
		} else if (klass == Kernel.class) {
			return kernel;
		} else if (klass == Pose.class) {
			return pose;
		} else if (klass == Waypoints.class) {
			return waypoints;
		} else if (klass == Comm.class) {
			return comm;
		}
		return null;
	}
}
