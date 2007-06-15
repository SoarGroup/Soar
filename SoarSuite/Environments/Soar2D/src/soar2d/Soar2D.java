package soar2d;

import java.io.*;
import java.util.logging.*;

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
	String configFile = null;

	public static final String kDefaultXMLEatersSettingsFile = "eaters.xml";
	public static final String kDefaultXMLTankSoarSettingsFile = "tanksoar.xml";
	public static final String kDefaultXMLEatersConsoleSettingsFile = "eaters-console.xml";
	public static final String kDefaultXMLTankSoarConsoleSettingsFile = "tanksoar-console.xml";
	public static final String kDefaultXMLBookSettingsFile = "book.xml";

	public static final Logger logger = Logger.getLogger("soar2d");
	public static Configuration config = new Configuration();
	public static final WindowManager wm = new WindowManager();
	public static final Simulation simulation = new Simulation();
	public static final Controller control = new Controller();

	public Soar2D(String[] args) {
		// Fire up view
		boolean wmSuccess = wm.initialize();

		// Try to install default config files
		try {
			install(kDefaultXMLEatersSettingsFile);
		} catch (IOException e) {
			control.severeError("IOException installing " + kDefaultXMLEatersSettingsFile + ": " + e.getMessage());
			wm.shutdown();
			System.exit(1);
		}
		try {
			install(kDefaultXMLTankSoarSettingsFile);
		} catch (IOException e) {
			control.severeError("IOException installing " + kDefaultXMLTankSoarSettingsFile + ": " + e.getMessage());
			wm.shutdown();
			System.exit(1);
		}
		try {
			install(kDefaultXMLEatersConsoleSettingsFile);
		} catch (IOException e) {
			control.severeError("IOException installing " + kDefaultXMLEatersConsoleSettingsFile + ": " + e.getMessage());
			wm.shutdown();
			System.exit(1);
		}
		try {
			install(kDefaultXMLTankSoarConsoleSettingsFile);
		} catch (IOException e) {
			control.severeError("IOException installing " + kDefaultXMLTankSoarConsoleSettingsFile + ": " + e.getMessage());
			wm.shutdown();
			System.exit(1);
		}
		try {
			install(kDefaultXMLBookSettingsFile);
		} catch (IOException e) {
			control.severeError("IOException installing " + kDefaultXMLBookSettingsFile + ": " + e.getMessage());
			wm.shutdown();
			System.exit(1);
		}

		if (args.length > 0) {
			configFile = args[0];
		} else {
			if (wmSuccess) {
				configFile = wm.promptForConfig();
			} else {
				System.err.println("No configuration file specified. Please specify a configuration file on the command line.");
			}
		}
		if (configFile == null) {
			wm.shutdown();
			System.exit(1);
		}
		
		// Read config file
		config = new Configuration();
		try {
			config.load(new File(configFile));
		} catch (Configuration.LoadError e) {
			control.severeError("Error loading configuration file: " + e.getMessage());
			wm.shutdown();
			System.exit(1);
		}
		
		logger.setLevel(config.getLogLevel());
		// Start logger
		if (config.getLogFile() != null) {
			try {
				FileHandler handler = new FileHandler(config.getLogFile().getAbsolutePath());
				handler.setLevel(config.getLogLevel());
				if (config.getLogTime()) {
					handler.setFormatter(new TextFormatter());
				} else {
					handler.setFormatter(new NoTimeTextFormatter());
				}
				logger.addHandler(handler);
			} catch (IOException e) {
				control.severeError("IOException creating " + config.getLogFile().getAbsolutePath() + ": " + e.getMessage());
				wm.shutdown();
				System.exit(1);
			}
		}
		if (config.getLogConsole()) {
			// Console handler
			ConsoleHandler handler = new ConsoleHandler();
			handler.setLevel(config.getLogLevel());
			if (config.getLogTime()) {
				handler.setFormatter(new TextFormatter());
			} else {
				handler.setFormatter(new NoTimeTextFormatter());
			}
			logger.addHandler(handler);
		}
		if ((config.getLogFile() != null) || config.getLogConsole()) {
			logger.setUseParentHandlers(false);
		} else {
			System.out.println("Warning: not logging anything");
			logger.setLevel(Level.OFF);
		}
		
		if (!config.getNoGUI()) {
			if (!wmSuccess) {
				control.severeError("Failed to initialize display.");
				System.exit(1);
			}
		} else {
			// shut down visuals
			wm.shutdown();
		}

		// Initialize simulation
		if (!simulation.initialize()) {
			System.exit(1);
		}
		
		if (!config.getNoGUI()) {
			// Run GUI
			control.runGUI();
		} else {
			// Run simulation
			control.startSimulation(false, false);
		}
		
		control.shutdown();
	}

	private void install(String file) throws IOException {	
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
	}

	public static void main(String[] args) {
		new Soar2D(args);
		
		// Ensure all threads clean up properly.  This is a bit heavy handed but helps ensure we are completely stopped.
		System.exit(0) ;
	}

}
