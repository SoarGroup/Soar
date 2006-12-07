package soar2d;

import java.io.File;
import java.util.logging.*;

import sml.*;

public class Controller implements Kernel.UpdateEventInterface, Kernel.SystemEventInterface, Runnable {
	boolean stopSoar = false;
	boolean step = false;
	boolean running = false;
	Thread runThread = null;
	boolean stopRequested = false;
	boolean shuttingDown = false;
	
	public void severeError(String message) {
		System.err.println(message);
		Soar2D.logger.severe(message);
		
		if (Soar2D.wm.using()) {
			String title = null;
			if (Soar2D.config.eaters) {
				title = "Eaters";
			} else if (Soar2D.config.tanksoar) {
				title = "TankSoar";
			} else {
				title = "Error";
			}
			Soar2D.wm.errorMessage(title, message);
		}
	}
	
	public void infoPopUp(String message) {
		Soar2D.logger.info(message);
		
		if (Soar2D.wm.using()) {
			String title = null;
			if (Soar2D.config.eaters) {
				title = "Eaters";
			} else if (Soar2D.config.tanksoar) {
				title = "TankSoar";
			} else {
				title = "Information";
			}
			Soar2D.wm.infoMessage(title, message);
		}
	}
	
	public void playerEvent() {
		if (Soar2D.wm.using()) {
			Soar2D.wm.agentEvent();
		}		
	}
	
	public void startSimulation(boolean step, boolean newThread) {
		this.step = step;
		if (step) {
			Soar2D.wm.setStatus("Stepping");
		} else {
			Soar2D.wm.setStatus("Running");
		}
		stopRequested = false;
		stopSoar = false;
		step = false;
		if (newThread) {
			runThread = new Thread(this);
			runThread.start();
		} else {
			run();
		}
	}
	
	public void stopSimulation() {
		stopRequested = true;
		stopSoar = true;
		step = false;
	}
	
	public boolean resetSimulation() {
		if (!Soar2D.simulation.reset()) {
			return false;
		}
		if (Soar2D.wm.using()) {
			Soar2D.wm.reset();
		}
		return true;
	}
	
	public void run() {
		if (Soar2D.simulation.hasSoarAgents()) {
			if (step) {
				Soar2D.simulation.runStep();
			} else {
				Soar2D.simulation.runForever();
			}
		} else {
			startEvent();
			if (!step) {
				while (!stopSoar) {
					tickEvent();
				}
			} else {
				tickEvent();
			}
			stopEvent();
		}
		Soar2D.wm.setStatus("Ready");
	}
	
	private void startEvent() {
		if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Start event.");
		running = true;
		if (Soar2D.wm.using()) {
			Soar2D.wm.start();
		}
	}
	
	private void tickEvent() {
		Soar2D.logger.info("---");
		Soar2D.simulation.update();
		if (Soar2D.wm.using()) {
			Soar2D.wm.update();
		}
	}
	
	private void stopEvent() {
		if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Stop event.");
		running = false;
		if (Soar2D.wm.using()) {
			Soar2D.wm.stop();
		}
	}
	
  	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
  		int dontUpdate = runFlags & smlRunFlags.sml_DONT_UPDATE_WORLD.swigValue();
  		if (dontUpdate != 0) {
  			Soar2D.logger.warning("Not updating world due to run flags!");
  			return;
  		}
  		
  		tickEvent();
  		
		// Test this after the world has been updated, in case it's asking us to stop
		if (stopSoar) {
  			stopSoar = false;
  			if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Stop requested during update.");
  			kernel.StopAllAgents();
  		}
  	}
  	
    public void systemEventHandler(int eventID, Object data, Kernel kernel) {
  		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
  			startEvent();
  		} else if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
  			stopEvent();
  		} else {
  			Soar2D.logger.warning("Unknown system event received from kernel, ignoring: " + eventID);
 		}
    }

	public void runGUI() {
		if (Soar2D.wm.using()) {
			Soar2D.wm.run();
		}
	}

	public boolean isShuttingDown() {
		return shuttingDown;
	}
	public void shutdown() {
		shuttingDown = true;
		stopSimulation();
		if (Soar2D.logger.isLoggable(Level.INFO)) Soar2D.logger.info("Shutdown called.");
		if (Soar2D.wm.using()) {
			Soar2D.wm.shutdown();
		}
		Soar2D.simulation.shutdown();
	}

	public boolean isRunning() {
		return running;
	}

	public void changeMap(String map) {
		if ((map == null) || (map.length() <= 0)) {
			severeError("map not specified, is required");
		}
		if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Changing map to " + map);
		File mapFile = new File(map);
		if (!mapFile.exists()) {
			mapFile = new File(Soar2D.config.mapPath + map);
			if (!mapFile.exists()) {
				severeError("Error finding map " + map);
				return;
			}
		}
		File oldMap = Soar2D.config.map;
		Soar2D.config.map = mapFile;

		if (!resetSimulation()) {
			Soar2D.config.map = oldMap;
		}
	}
}
