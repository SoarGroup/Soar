package soar2d;

import java.io.File;
import java.util.logging.*;

import sml.*;

/**
 * @author voigtjr
 *
 * Control keeps track of the simulation, if it is running or not, the update
 * process, etc.
 */
public class Controller implements Kernel.UpdateEventInterface, Kernel.SystemEventInterface, Runnable {
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
	
	/**
	 * Set to true when a stop is requested
	 */
	public synchronized boolean isStopped() { return this.stop ; }
	
	/**
	 * @param message The message to display to the screen, console and/or log
	 * 
	 * Call whenever there is a severe error. An attempt will be made to dump the
	 * error to a message box. The error will be logged to the console no matter what.
	 * It will also go to the log.
	 * 
	 * The program does not have to quit when this is called.
	 */
	public void severeError(String message) {
		System.err.println(message);
		Soar2D.logger.severe(message);
		
		if (Soar2D.wm.using()) {
			String title = null;
			switch(Soar2D.config.getType()) {
			case kEaters:
				title = "Eaters";
				break;
			case kTankSoar:
				title = "TankSoar";
				break;
			case kBook:
				title = "Book";
				break;
			}
			Soar2D.wm.errorMessage(title, message);
		}
	}
	
	/**
	 * @param message The message to display to the screen, console and/or log
	 * 
	 * Called to issue a pop-up message. Also sends a message to the log.
	 */
	public void infoPopUp(String message) {
		Soar2D.logger.info(message);
		
		if (Soar2D.wm.using()) {
			String title = null;
			switch(Soar2D.config.getType()) {
			case kEaters:
				title = "Eaters";
				break;
			case kTankSoar:
				title = "TankSoar";
				break;
			case kBook:
				title = "Book";
				break;
			}
			Soar2D.wm.infoMessage(title, message);
		}
	}
	
	/**
	 * Called when there is a change in the number of players.
	 */
	public void playerEvent() {
		if (Soar2D.wm.using()) {
			Soar2D.wm.playerEvent();
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
			Soar2D.wm.setStatus("Stepping");
		} else {
			Soar2D.wm.setStatus("Running");
		}
		
		stop = false;

		// TOSCA patch -- try a call to tosca code
		//soar2d.tosca2d.Tosca.test() ;
		
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
	}
	
	/**
	 * @return true if the reset completed without error
	 * 
	 * Called to reset the simulation.
	 */
	public boolean resetSimulation() {
		if (!Soar2D.simulation.reset()) {
			return false;
		}
		if (Soar2D.wm.using()) {
			Soar2D.wm.reset();
		}
		return true;
	}
	
	/** 
	 * The thread, do not call directly!
	 */
	public void run() {
		// if there are soar agents
		if (Soar2D.simulation.hasSoarAgents()) {
			
			// have soar control things
			// it will call startEvent, tickEvent, and stopEvent in callbacks.
			if (step) {
				Soar2D.simulation.runStep();
			} else {
				Soar2D.simulation.runForever();
			}
		} else if (soar2d.player.ToscaEater.kToscaEnabled)
		{
			if (step)
				soar2d.tosca2d.ToscaInterface.getTosca().runStep() ;
			else
				soar2d.tosca2d.ToscaInterface.getTosca().runForever() ;
		} else {
			
			// there are no soar agents, call the start event
			startEvent();
			
			// run as necessary
			if (!step) {
				while (!stop) {
					tickEvent();
				}
			} else {
				tickEvent();
			}
			
			// call the stop event
			stopEvent();
		}
		
		// reset the status message
		Soar2D.wm.setStatus("Ready");
	}
	
	/**
	 * Called internally to signal the actual start of the run.
	 * If soar is controlling things, soar calls this during the system start event.
	 * If soar is not controlling things, this gets called by run() before
	 * starting the sim.
	 */
	public void startEvent() {
		if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Start event.");
		running = true;

		if (Soar2D.wm.using()) {
			// this updates buttons and what-not
			Soar2D.wm.start();
		}
	}
	
	/**
	 * Fires a world update. If soar is running things, this is called during the 
	 * output callback. If soar is not running things, this is called by run() in 
	 * a loop if necessary.
	 */
	public void tickEvent() {
		Soar2D.simulation.update();
		if (Soar2D.wm.using()) {
			Soar2D.wm.update();
		}
	}
	
	/**
	 * Signals the actual end of the run. If soar is running things, this is called
	 * by soar during the system stop event. Otherwise, this gets called by run
	 * after a stop is requested.
	 */
	public void stopEvent() {
		if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Stop event.");
		running = false;
		
		if (Soar2D.wm.using()) {
//			 this updates buttons and what-not
			Soar2D.wm.stop();
		}
	}
	
  	/**
  	 * Handle an update event from Soar, do not call directly.
  	 */
  	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
  		
  		// check for override
  		int dontUpdate = runFlags & smlRunFlags.sml_DONT_UPDATE_WORLD.swigValue();
  		if (dontUpdate != 0) {
  			Soar2D.logger.warning("Not updating world due to run flags!");
  			return;
  		}
  		
  		// this updates the world
  		tickEvent();
  		
		// Test this after the world has been updated, in case it's asking us to stop
		if (stop) {
			// the world has asked us to kindly stop running
  			if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Stop requested during update.");
  			
  			// note that soar actually controls when we stop
  			kernel.StopAllAgents();
  		}
  	}
  	
  	/**
  	 * Handle a system event from Soar, do not call directly
  	 */
   public void systemEventHandler(int eventID, Object data, Kernel kernel) {
  		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
  			// soar says go
  			startEvent();
  		} else if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
  			// soar says stop
  			stopEvent();
  		} else {
  			// soar says something we weren't expecting
  			Soar2D.logger.warning("Unknown system event received from kernel, ignoring: " + eventID);
 		}
    }

	/**
	 * Create the GUI and show it, and run its loop. Does not return until the 
	 * GUI is disposed. 
	 */
	public void runGUI() {
		if (Soar2D.wm.using()) {
			// creates, displays and loops the window. returns on shutdown *hopefully
			Soar2D.wm.run();
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
		Soar2D.logger.info("Shutdown called.");
		if (Soar2D.wm.using()) {
			// closes out the window manager
			Soar2D.wm.shutdown();
		}
		// closes out the simulation
		Soar2D.simulation.shutdown();
	}

	/**
	 * @return True if the simulation is currently running.
	 */
	public boolean isRunning() {
		return running;
	}

	/**
	 * @param map the name of the map to change to
	 */
	public void changeMap(String map) {
		
		//TODO: this should take in a File object
		
		// make sure it is somewhat valid
		if ((map == null) || (map.length() <= 0)) {
			severeError("map not specified, is required");
		}
		if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Changing map to " + map);
		
		File mapFile = new File(map);
		// check as absolute
		if (!mapFile.exists()) {
			
			// doesn't exist as absolute, check relative to map dir
			mapFile = new File(Soar2D.config.getMapPath() + map);
			if (!mapFile.exists()) {
				
				// doesn't exist there either
				severeError("Error finding map " + map);
				return;
			}
		}
		
		// save the old map in case the new one is screwed up
		File oldMap = Soar2D.config.map;
		Soar2D.config.map = mapFile;

		// the reset will fail if the map fails to load
		if (!resetSimulation()) {
			
			// and if it fails the map will remain unchanged, set it back
			Soar2D.config.map = oldMap;
		}
	}
}
