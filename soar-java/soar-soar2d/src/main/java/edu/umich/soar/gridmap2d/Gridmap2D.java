package edu.umich.soar.gridmap2d;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;


import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.config.SimConfig;
import edu.umich.soar.gridmap2d.visuals.WindowManager;
import edu.umich.soar.gridmap2d.world.World;

/*
 * A note about log levels:
 * 
 * fatal:
 * critical errors that abort the program
 * 
 * error:
 * major errors that do not abort the program
 * 
 * warn:
 * things that should not happen but don't impede running
 * 
 * info:
 * all information necessary to accurately reproduce a run
 * 
 * debug:
 * more information, possibly useful for agent debugging
 * 
 * trace:
 * the most information, for debugging soar2d
 */

public class Gridmap2D {
	private static Logger logger = Logger.getLogger(Gridmap2D.class);
	
	public static SimConfig config = null;
	public static final WindowManager wm = new WindowManager();
	public static final Simulation simulation = new Simulation();
	public static final Controller control = new Controller();

	private boolean installedAConfig = false;
	
	public Gridmap2D(String[] args) {
		// Try to install default config files
		install(Names.configs.tanksoarCnf);
		install(Names.configs.tanksoarConsoleCnf);
		install(Names.configs.eatersCnf);
		install(Names.configs.eatersConsoleCnf);
		install(Names.configs.roomCnf);
		install(Names.configs.taxiCnf);

		loadConfig(args);

		// if using gui
		boolean usingGUI = !config.generalConfig().headless;
		if (usingGUI) {
			// initialize wm
			logger.trace(Names.Trace.initDisplay);
			if (!wm.initialize()) {
				fatalError(Names.Errors.initDisplayfail);
			}
		}

		// Initialize simulation
		logger.trace(Names.Trace.initSimulation);
		World world = null;
		try {
			world = simulation.initialize(config);
		} catch (Exception e) {
			fatalError(Names.Errors.simulationInitFail + e.getMessage());
			e.printStackTrace();
		}
		
		if (usingGUI) {
			// Run GUI
			logger.trace(Names.Trace.startGUI);
			control.runGUI(world);
		} else {
			// Run simulation
			logger.trace(Names.Trace.startSimulation);
			try {
				control.startSimulation(false, false);
			} catch (Exception e) {
				fatalError("Simulation exception: " + e.getMessage());
			}
		}
		
		// calls wm.shutdown()
		logger.trace(Names.Trace.shutdown);
		try {
			control.shutdown();
		} catch (Exception e) {
			fatalError(e.getMessage());
		}
		
		logger.trace(Names.Trace.savingPreferences);
		config.savePreferences();
	}
	
	private void fatalError(String message) {
		logger.fatal(message);
		System.err.println(message);
		if (wm != null && wm.using()) {
			wm.errorMessage(config.title(), message);
			wm.shutdown();
		}
		logger.fatal(Names.Trace.exitErrorLevel + 1);
		System.exit(1);
	}
	
	private void loadConfig(String [] args) {
		String configPath = null;
		if (args.length > 0) {
			configPath = args[0];
		} else {
			if (wm.initialize()) {
				configPath = wm.promptForConfig();
			} else {
				fatalError(Names.Errors.noConfig);
			}
		}
		
		// can be null if canceled in gui prompt
		if (configPath == null) {
			fatalError(Names.Errors.noConfig);
		}

		// Read config file
		try {
			config = SimConfig.load(configPath);
		} catch (IOException e) {
			if(config == null) {
				wm.initialize();
				fatalError(Names.Errors.loadingConfig);
			}
		}
	}
	
	private void install(String file) {	
		try {
			// We just work relative to the current directory because that's how
			// load library will work.
			File cnf = new File(file) ;
			File cnfDest = new File("config" + System.getProperty("file.separator") + file) ;
	
			if (cnfDest.exists()) {
				return;
			}
			
			// Get the DLL from inside the JAR file
			// It should be placed at the root of the JAR (not in a subfolder)
			String jarpath = "/" + cnf.getPath() ;
			InputStream is = this.getClass().getResourceAsStream(jarpath) ;
			
			if (is == null) {
				System.err.println("Failed to find " + jarpath + " in the JAR file") ;
				return;
			}
			
			// Create the new file on disk
			FileOutputStream os = new FileOutputStream(cnfDest) ;
			
			// Copy the file onto disk
			byte bytes[] = new byte[2048];
			int read;
			while (true) {
				read = is.read( bytes) ;
				
				// EOF
				if ( read == -1 ) break;
				
				os.write( bytes, 0, read);
			}
	
			is.close() ;
			os.close() ;
		} catch (IOException e) {
			wm.initialize();
			fatalError(Names.Errors.installingConfig + file + ": " + e.getMessage());
		}
		
		if (!installedAConfig) {
			installedAConfig = true;
			System.err.println("Installed at least one config file.\nYou may need to refresh the project if you are working inside of Eclipse.");
		}
	}

	public static void main(String[] args) {
		new Gridmap2D(args);
		
		// Ensure all threads clean up properly.  This is a bit heavy handed but helps ensure we are completely stopped.
		logger.trace(Names.Trace.exitErrorLevel + 0);
		System.exit(0) ;
	}

}
