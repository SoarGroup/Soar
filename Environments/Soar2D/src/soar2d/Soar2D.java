package soar2d;

import java.io.*;
import java.util.logging.*;

import soar2d.config.Config;
import soar2d.config.ConfigUtil;
import soar2d.config.SimConfig;
import soar2d.config.Soar2DKeys;
import soar2d.visuals.*;
/*
 * A note about log levels:
 * 
 * severe:
 * critical errors that impede the running of the program
 * 
 * warning:
 * stuff that should not happen, malfunctioning agents, etc.
 * 
 * info:
 * all information necessary to accurately reproduce a run, plus a minimal
 * amount of status data (score)
 * includes: score, moves, spawns, terminal messages
 * 
 * fine:
 * more information regarding the current state, more information on agent behavior
 * helpful for debugging agents
 * 
 * finer:
 * another step of information possibly helpful with agent debugging
 * 
 * finest:
 * usually only useful for debugging Soar2D
 */

public class Soar2D {
	public static final Logger logger = Logger.getLogger("soar2d");
	public static Config config = null;
	public static SimConfig simConfig = null;
	public static final WindowManager wm = new WindowManager();
	public static final Simulation simulation = new Simulation();
	public static final Controller control = new Controller();

	public Soar2D(String[] args) {

		// Try to install default config files
		install(Names.configs.tanksoarCnf);
		install(Names.configs.tanksoarConsoleCnf);
		install(Names.configs.eatersCnf);
		install(Names.configs.eatersConsoleCnf);

		loadConfig(args);
		initializeLogger();
		
		// if using gui
		boolean usingGUI = !config.getBoolean(Soar2DKeys.general.nogui, false);
		if (usingGUI) {
			// initialize wm
			if (!wm.initialize()) {
				control.severeError("Failed to initialize display.");
				System.exit(1);
			}
		}

		// Initialize simulation
		if (!simulation.initialize()) {
			System.exit(1);
			wm.shutdown();
		}
		
		if (usingGUI) {
			// Run GUI
			control.runGUI();
		} else {
			// Run simulation
			control.startSimulation(false, false);
		}
		
		// calls wm.shutdown()
		control.shutdown();
	}
	
	private void loadConfig(String [] args) {
		String configPath = null;
		if (args.length > 0) {
			configPath = args[0];
		} else {
			if (wm.initialize()) {
				configPath = wm.promptForConfig();
			} else {
				System.err.println("No configuration file specified. Please specify a configuration file on the command line.");
			}
		}
		if (configPath == null) {
			wm.shutdown();
			System.exit(1);
		}

		// Read config file
		config = ConfigUtil.tryLoadConfig(configPath);
		if(config == null) {
			wm.initialize();
			control.severeError("Error loading configuration file");
			wm.shutdown();
			System.exit(1);
		}
		simConfig = SimConfig.create(config.getString("general.game"));
		if (simConfig == null) {
			wm.initialize();
			control.severeError("Illegal game: " + config.getString("general.game"));
			wm.shutdown();
			System.exit(1);
		}
	}
	
	private void initializeLogger() {
		Level level = null;
		try {
			level = Level.parse(config.getString(Soar2DKeys.general.logging.level));
		} catch (IllegalArgumentException e) {
			wm.initialize();
			control.severeError(Soar2DKeys.general.logging.level + ": " + e.getMessage());
			wm.shutdown();
			System.exit(1);
		} catch (NullPointerException e) {
			level = Level.INFO;
		}

		logger.setLevel(level);
		boolean logTime = config.getBoolean(Soar2DKeys.general.logging.time, false);
		boolean notLogging = true;

		// Start logger
		String loggingFile = config.getString(Soar2DKeys.general.logging.file);
		if (loggingFile != null) {
			FileHandler handler = null;
			try {
				handler = new FileHandler(new File(loggingFile).getAbsolutePath());
			} catch (IOException e) {
				wm.initialize();
				control.severeError("IOException creating " + loggingFile + ": " + e.getMessage());
				wm.shutdown();
				System.exit(1);
			}
			handler.setLevel(level);
			if (logTime) {
				handler.setFormatter(new TextFormatter());
			} else {
				handler.setFormatter(new NoTimeTextFormatter());
			}
			logger.addHandler(handler);
			notLogging = false;
		}
		
		if (config.getBoolean(Soar2DKeys.general.logging.console, true)) {
			// Console handler
			ConsoleHandler handler = new ConsoleHandler();
			handler.setLevel(level);
			if (logTime) {
				handler.setFormatter(new TextFormatter());
			} else {
				handler.setFormatter(new NoTimeTextFormatter());
			}
			logger.addHandler(handler);
			notLogging = false;
		}
		
		logger.setUseParentHandlers(false);
		if (notLogging) {
			logger.setLevel(Level.OFF);
		}
	}

	private void install(String file) {	
		try {
			// We just work relative to the current directory because that's how
			// load library will work.
			File library = new File(file) ;
	
			if (library.exists()) {
				//System.out.println(library + " already exists so not installing from the JAR file") ;
				return;
			}
			
			// Get the DLL from inside the JAR file
			// It should be placed at the root of the JAR (not in a subfolder)
			String jarpath = "/" + library.getPath() ;
			InputStream is = this.getClass().getResourceAsStream(jarpath) ;
			
			if (is == null) {
				System.err.println("Failed to find " + jarpath + " in the JAR file") ;
				return;
			}
			
			// Make sure we can delete the library.  This is actually here to cover the
			// case where we're running in Eclipse without a JAR file.  The getResourceAsStream()
			// call can end up loading the same library that we're trying to save to and we
			// end up with a blank file.  Explicitly trying to delete it first ensures that
			// we're not reading the same file that we're writing.
			if (library.exists() && !library.delete()) {
				System.err.println("Failed to remove the existing layout file " + library) ;
				return;
			}
			
			// Create the new file on disk
			FileOutputStream os = new FileOutputStream(library) ;
			
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
			
			System.out.println("Installed " + library + " onto the local disk from JAR file") ;
		} catch (IOException e) {
			wm.initialize();
			control.severeError("IOException installing " + file + ": " + e.getMessage());
			wm.shutdown();
			System.exit(1);
		}
	}

	public static void main(String[] args) {
		new Soar2D(args);
		
		// Ensure all threads clean up properly.  This is a bit heavy handed but helps ensure we are completely stopped.
		System.exit(0) ;
	}

}
