package soar2d;

import java.util.*;
import java.util.logging.*;

import sml.*;

public class Simulation {
	boolean runTilOutput = false;
	Kernel kernel = null;
	public static Random random = null;
	World world = new World();
	String currentMap = null;
	
	public boolean initialize() {
		// Tanksoar uses run til output
		runTilOutput = Soar2D.config.tanksoar;
		
		// Initialize Soar
		if (Soar2D.config.remote) {
			kernel = Kernel.CreateRemoteConnection(true);
		} else {
			// Create kernel
			kernel = Kernel.CreateKernelInNewThread("SoarKernelSML", 12121);
			//kernel = Kernel.CreateKernelInCurrentThread("SoarKernelSML", true);
		}

		if (kernel.HadError()) {
			Soar2D.control.severeError("Error creating kernel: " + kernel.GetLastErrorDescription());
			return false;
		}
		
		// We want the most performance
		if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Setting auto commit false.");
		kernel.SetAutoCommit(false);

		// Make all runs non-random if asked
		// For debugging, set this to make all random calls follow the same sequence
		if (Soar2D.config.random) {
			if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Not seeding generator.");
			random = new Random();
		} else {
			if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Seeding generator 0.");
			kernel.ExecuteCommandLine("srand 0", null) ;
			random = new Random(0);
		}
		
		// Register for events
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, Soar2D.control, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, Soar2D.control, null);
		if (runTilOutput) {
			if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Registering for: smlEVENT_AFTER_ALL_GENERATED_OUTPUT");
			kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT, Soar2D.control, null);
		} else {
			if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Registering for: smlEVENT_AFTER_ALL_OUTPUT_PHASES");
			kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, Soar2D.control, null);
		}
		return true;
	}

	public void update() {
		world.update();
	}

	public void runForever() {
		if (runTilOutput) {
			kernel.RunAllAgentsForever(smlInterleaveStepSize.sml_INTERLEAVE_UNTIL_OUTPUT);
		} else {
			kernel.RunAllAgentsForever();
		}
		
	}

	public void runStep() {
		if (runTilOutput) {
			kernel.RunAllTilOutput();
		} else {
			kernel.RunAllAgents(1);
		}
	}

	public void reset() {
		if (Soar2D.logger.isLoggable(Level.INFO)) Soar2D.logger.info("Resetting simulation.");
		if (!world.load(currentMap)) {
			Soar2D.control.severeError("Error loading map " + currentMap);
		}
		world.reset();
	}

	public void shutdown() {
		if (world != null) {
			world.shutdown();
		}
		if (kernel != null) {
			if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Shutting down kernel.");
			kernel.Shutdown();
			kernel.delete();
		}
	}
	
	public boolean hasSoarAgents() {
		// FIXME: implement
		return true;
	}
	
	public boolean hasEntities() {
		// FIXME: implement
		return false;
	}
	
	public void destroyEntity(Entity selectedEntity) {
		// TODO Auto-generated method stub
		
	}

	public boolean reachedMaxUpdates() {
		// TODO Auto-generated method stub
		return false;
	}

	public Entity[] getEntities() {
		// TODO Auto-generated method stub
		return null;
	}

	public void createEntity(String text, String productions, String text2, Object object, Object object2, int i, int j, int k) {
		// TODO Auto-generated method stub
	}
	public void changeMap(String map) {
		// TODO Auto-generated method stub
	}

}
