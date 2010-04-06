package edu.umich.soar.sproom.soar;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicBoolean;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.SoarProperties;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.HzChecker;
import edu.umich.soar.sproom.command.Comm;
import edu.umich.soar.sproom.command.CommandConfig;
import edu.umich.soar.sproom.command.Lidar;
import edu.umich.soar.sproom.command.MapMetadata;
import edu.umich.soar.sproom.command.Pose;
import edu.umich.soar.sproom.command.VirtualObjects;
import edu.umich.soar.sproom.command.Waypoints;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.DriveListener;

import sml.Agent;
import sml.Kernel;
import sml.smlAgentEventId;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;
import sml.Kernel.AgentEventInterface;
import sml.Kernel.SystemEventInterface;
import sml.Kernel.UpdateEventInterface;

/**
 * Top level container for Soar interface.
 *
 * @author voigtjr@gmail.com
 */
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
	private final Lidar lidar;
	private final MapMetadata metadata;
	private final VirtualObjects vobjs;
	private final Cargo cargo = new Cargo();
	
	public SoarInterface(Pose pose, Waypoints waypoints, Comm comm, Lidar lidar, MapMetadata metadata, VirtualObjects vobjs) {
		this.pose = pose;
		this.waypoints = waypoints;
		this.comm = comm;
		this.lidar = lidar;
		this.metadata = metadata;
		this.vobjs = vobjs;
		
		kernel = Kernel.CreateKernelInNewThread(Kernel.GetDefaultLibraryName(), Kernel.kUseAnyPort);
		if (kernel.HadError()) {
			logger.error("Soar error: " + kernel.GetLastErrorDescription());
			System.exit(1);
		}
		
		SoarProperties sp = new SoarProperties();
		logger.warn(String.format("Kernel port: %d, pid: %d", kernel.GetListenerPort(), sp.getPid()));

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
			productions = null;
		}
		
		if (productions == null) {
			logger.info("No productions, executing waitsnc and watch 0");
			agent.ExecuteCommandLine("waitsnc -e");
			agent.ExecuteCommandLine("w 0");
		}
		
		kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, new UpdateEventInterface() {
			public void updateEventHandler(int eventID, Object data, Kernel kernel, int arg3) {
				logger.trace("smlEVENT_AFTER_ALL_OUTPUT_PHASES");
				if (logger.isDebugEnabled()) {
					hzChecker.tick();
				}
				
				if (stopSoar.get()) {
					logger.debug("Stopping Soar");
					kernel.StopAllAgents();
				}

				if (ol.getDDC() != null) {
					ddcPrev = ol.getDDC();
					fireDriveEvent(ddcPrev);
				}
				
				il.update(SoarInterface.this);
				agent.Commit();
				logger.trace("smlEVENT_AFTER_ALL_OUTPUT_PHASES done");
			}
		}, null);
		
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, systemHandler, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, systemHandler, null);
		
		kernel.RegisterForAgentEvent(smlAgentEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED, agentHandler, null);
		kernel.RegisterForAgentEvent(smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED, agentHandler, null);

		agent.Commit();
		
		if (CommandConfig.CONFIG.getSpawnDebugger()) {
			spawnDebugger();
		}
	}
	
	void spawnDebugger() {
		SoarProperties sp = new SoarProperties();
		sp.spawnDebugger(kernel, agent);
	}
	
	AgentEventInterface agentHandler = new AgentEventInterface() {
		@Override
		public void agentEventHandler(int eventID, Object data, String agentName) {
			if (eventID == smlAgentEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED.swigValue()) {
				logger.trace("smlEVENT_BEFORE_AGENT_REINITIALIZED");
				il.destroy();

				agent.Commit();
				logger.trace("smlEVENT_BEFORE_AGENT_REINITIALIZED done");
			} else if (eventID == smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED.swigValue()) {			
				logger.trace("smlEVENT_AFTER_AGENT_REINITIALIZED");
				il = InputLink.newInstance(SoarInterface.this);
				ol = OutputLink.newInstance(SoarInterface.this);
				
				agent.Commit();
				logger.trace("smlEVENT_AFTER_AGENT_REINITIALIZED done");
			}			
		}
	};
	
	SystemEventInterface systemHandler = new Kernel.SystemEventInterface() {
		@Override
		public void systemEventHandler(int eventId, Object arg1, Kernel arg2) {
			if (eventId == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
				logger.trace("smlEVENT_SYSTEM_START");
				if (ddcPrev != null) {
					fireDriveEvent(ddcPrev);
				}
				stopSoar.set(false);
				logger.info("Soar started.");
			} 
			else if (eventId == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
				logger.trace("smlEVENT_SYSTEM_STOP");
				fireDriveEvent(DifferentialDriveCommand.newEStopCommand());
				logger.info("Soar stopped.");
			}
		}
	};
	
	private final List<DriveListener> driveListeners = new CopyOnWriteArrayList<DriveListener>();
	public void addDriveListener(DriveListener driveListener) {
		driveListeners.add(driveListener);
	}
	
	public boolean removeDriveListener(DriveListener driveListener) {
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
		} else if (klass == Lidar.class) {
			return lidar;
		} else if (klass == MapMetadata.class) {
			return metadata;
		} else if (klass == VirtualObjects.class) {
			return vobjs;
		} else if (klass == Cargo.class) {
			return cargo;
		}
		return null;
	}
}
