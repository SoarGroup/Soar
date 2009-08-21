package edu.umich.soar.gridmap2d;

import org.apache.log4j.Logger;

import com.commsen.stopwatch.Report;
import com.commsen.stopwatch.Stopwatch;

import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.visuals.WindowManager;
import edu.umich.soar.gridmap2d.world.World;

/**
 * @author voigtjr
 *
 * Control keeps track of the simulation, if it is running or not, the update
 * process, etc.
 */
public class Controller implements Runnable {
	private static Logger logger = Logger.getLogger(Controller.class);

	/**
	 * Set to true when a stop is requested
	 */
	private boolean stop = false;
	/**
	 * Set to true when we're only stepping as opposed to running
	 */
	private boolean step = false;
	/**
	 * Set to true if the simulation is running. This is true also when stepping.
	 */
	private boolean running = false;
	/**
	 * This is not null when there is a thread running the simulation.
	 */
	private Thread runThread = null;
	/**
	 * This is true when things are in the process of shutting down
	 */
	private boolean shuttingDown = false;
	
	private double timeSlice = 0;
	private CognitiveArchitecture cogArch;

	/**
	 * Set to true when a stop is requested
	 */
	public synchronized boolean isStopped() { return this.stop ; }
	
	public void errorPopUp(String message) {
		if (Gridmap2D.wm.using()) {
			Gridmap2D.wm.errorMessage(Gridmap2D.config.title(), message);
		} else {
			logger.error(message);
		}
	}
	
	public void infoPopUp(String message) {
		if (Gridmap2D.wm.using()) {
			Gridmap2D.wm.infoMessage(Gridmap2D.config.title(), message);
		} else {
			logger.info(message);
		}
	}
	
	/**
	 * Called when there is a change in the number of players.
	 */
	public void playerEvent() {
		if (Gridmap2D.wm.using()) {
			Gridmap2D.wm.playerEvent();
		}		
	}
	
	/**
	 * @param step true indicates run only one step
	 * @param newThread true indicates do the run in a new thread
	 * 
	 * Called to start the simulation.
	 */
	public void startSimulation(boolean step, boolean newThread) {
		this.step = step;
		
		// update the status line in the gui
		if (step) {
			Gridmap2D.wm.setStatus("Stepping", WindowManager.black);
		} else {
			Gridmap2D.wm.setStatus("Running", WindowManager.black);
		}
		
		// spawn a thread or just run it in this one
		if (newThread) {
			runThread = new Thread(this);
			runThread.start();
		} else {
			run();
		}
	}
	
	/**
	 * Called to stop the simulation. Requests soar to stop or tells the 
	 * run loop to stop executing. The current update will finish.
	 */
	public void stopSimulation() {
		// requests a stop
		stop = true;

		for (Report report : Stopwatch.getAllReports()) {
			System.out.println(report);
		}

	}
	
	/**
	 * @return true if the reset completed without error
	 * 
	 * Called to reset the simulation.
	 */
	public void resetSimulation() {
//		try {
			Gridmap2D.simulation.reset();
//		} catch (Exception e) {
//			e.printStackTrace();
//			error(e.getMessage());
//		}
		
		if (Gridmap2D.wm.using()) {
			Gridmap2D.wm.reset();
		}
	}
	
	/** 
	 * The thread, do not call directly!
	 */
	public void run() {
		
		do {
			// if there are soar agents
			if (cogArch.haveAgents()) {
				
				// have soar control things
				// it will call startEvent, tickEvent, and stopEvent in callbacks.
				if (step) {
					cogArch.runStep();
				} else {
					cogArch.runForever();
				}
			} else {
				
				// there are no soar agents, call the start event
				startEvent();
				
				// run as necessary
//				try {
					if (!step) {
						while (!stop) {
							tickEvent();
						}
					} else {
						tickEvent();
					}
//				} catch (Exception e) {
//					e.printStackTrace();
//					error(e.getMessage());
//				}
			
				// call the stop event
				stopEvent();
			}
			if (this.runsTerminal > 0) {
//				try {
					Gridmap2D.simulation.reset();
//				} catch (Exception e) {
//					logger.error("Exception thrown resetting simulation");
//					this.runsTerminal = 0;
//					if (Gridmap2D.wm.using()) {
//						// we're stopped, this updates buttons
//						Gridmap2D.wm.stop();
//					}
//				}
			}
		} while (this.runsTerminal > 0);

		// reset the status message
		Gridmap2D.wm.setStatus("Ready", WindowManager.black);
	}
	
	/**
	 * Called internally to signal the actual start of the run.
	 * If soar is controlling things, soar calls this during the system start event.
	 * If soar is not controlling things, this gets called by run() before
	 * starting the sim.
	 */
	public void startEvent() {
		logger.trace(Names.Trace.startEvent);
		stop = false;
		running = true;

		if (Gridmap2D.wm.using()) {
			// this updates buttons and what-not
			Gridmap2D.wm.start();
		}
	}
	
	/**
	 * Fires a world update. If soar is running things, this is called during the 
	 * output callback. If soar is not running things, this is called by run() in 
	 * a loop if necessary.
	 */
	public void tickEvent() {
		logger.trace("Tick event.");
		// this is 50 except for room, where it is configurable
		timeSlice = Gridmap2D.config.generalConfig().cycle_time_slice / 1000.0f;

		{
			long id = Stopwatch.start("tickEvent", "simulation update");
			Gridmap2D.simulation.update();
			Stopwatch.stop(id);
		}
		if (Gridmap2D.wm.using()) {
			long id = Stopwatch.start("tickEvent", "wm update");
			Gridmap2D.wm.update();
			Stopwatch.stop(id);
		}
	}
	
	public double getTimeSlice() {
		return timeSlice;
	}
	
	public void resetTime() {
		timeSlice = 0;
	}
	
	/**
	 * Signals the actual end of the run. If soar is running things, this is called
	 * by soar during the system stop event. Otherwise, this gets called by run
	 * after a stop is requested.
	 */
	public void stopEvent() {
		logger.trace(Names.Trace.stopEvent);
		running = false;
		
		if (checkRunsTerminal()) {
			if (Gridmap2D.wm.using()) {
				// we're stopped, this updates buttons
				Gridmap2D.wm.stop();
			}
		}
	}
	
	/**
	 * Create the GUI and show it, and run its loop. Does not return until the 
	 * GUI is disposed. 
	 */
	public void runGUI(World world) {
		if (Gridmap2D.wm.using()) {
			// creates, displays and loops the window. returns on shutdown *hopefully
			Gridmap2D.wm.run(world);
		}
	}

	/**
	 * @return true if the simulation is on its way down, as in closing to exit
	 * to OS (as opposed to stopping).
	 */
	public boolean isShuttingDown() {
		return shuttingDown;
	}
	/**
	 * Call to shutdown the simulation.
	 */
	public void shutdown() {
		// we're shutting down
		shuttingDown = true;
		
		// make sure things are stopped, doesn't hurt to call this when stopped
		stopSimulation();
		logger.info(Names.Info.shutdown);
		if (Gridmap2D.wm.using()) {
			// closes out the window manager
			Gridmap2D.wm.shutdown();
		}
		
		// closes out the simulation
		Gridmap2D.simulation.shutdown();
	}

	public boolean isRunning() {
		return running;
	}
	
	public CommandInfo getHumanCommand(Player player) {
		return Gridmap2D.wm.getHumanCommand(player);
	}
	
	private int runsTerminal = 0;

	public void setRunsTerminal(int runsTerminal) {
		this.runsTerminal = runsTerminal;
	}
	public int getRunsTerminal() {
		return this.runsTerminal;
	}
	private boolean checkRunsTerminal() {
		boolean stopNow = true;
		
		if (this.runsTerminal > 0) {
			this.runsTerminal -= 1;
			if (this.runsTerminal > 0) {
				stopNow = false;
			}
		}
		if (logger.isTraceEnabled()) {
			logger.trace("runs terminal: " + this.runsTerminal);
		}
		return stopNow;
	}

	public void setCogArch(CognitiveArchitecture cogArch) {
		this.cogArch = cogArch;
	}
}
