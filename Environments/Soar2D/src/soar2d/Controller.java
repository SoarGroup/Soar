package soar2d;

import java.util.logging.*;

import sml.*;

public class Controller implements Kernel.UpdateEventInterface, Kernel.SystemEventInterface, Runnable {
	boolean stopSoar = false;
	boolean running = false;
	Thread runThread = null;
	boolean stopRequested = false;
	
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
	
	public void playerEvent() {
		if (Soar2D.wm.using()) {
			Soar2D.wm.agentEvent();
		}		
	}
	
	public void startSimulation(boolean inNewThread) {
		stopRequested = false;
		stopSoar = false;
		if (Soar2D.simulation.hasSoarAgents()) {
			if (inNewThread) {
				runThread = new Thread(this);
				runThread.start();
			} else {
				run();
			}
		} else {
			startEvent();
			while (!stopSoar) {
				tickEvent();
			}
			stopEvent();
		}
	}
	
	public void stepSimulation() {
		if (Soar2D.simulation.hasSoarAgents()) {
			Soar2D.simulation.runStep();
		} else {
			startEvent();
			tickEvent();
			stopEvent();
		}
	}
	
	public void stopSimulation() {
		stopRequested = true;
		stopSoar = true;
	}
	
	public void resetSimulation() {
		Soar2D.simulation.reset();
		if (Soar2D.wm.using()) {
			Soar2D.wm.reset();
		}
	}
	
	public void run() {
		stopSoar = false;
		Soar2D.simulation.runForever();
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

	public void shutdown() {
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
		if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Changing map.");
		Soar2D.config.map = map;
		resetSimulation();
	}
}
