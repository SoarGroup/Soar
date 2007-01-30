package soar2d;

import java.io.*;
import java.util.logging.*;

import soar2d.visuals.*;
import soar2d.xml.ConfigurationLoader;
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
 * usually only useful for debugging  Soar2D
 */
public class Soar2D {
	String configFile = null;

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
			install(config.kDefaultXMLEatersSettingsFile);
		} catch (IOException e) {
			control.severeError("IOException installing " + config.kDefaultXMLEatersSettingsFile + ": " + e.getMessage());
			wm.shutdown();
			System.exit(1);
		}
		try {
			install(config.kDefaultXMLTankSoarSettingsFile);
		} catch (IOException e) {
			control.severeError("IOException installing " + config.kDefaultXMLTankSoarSettingsFile + ": " + e.getMessage());
			wm.shutdown();
			System.exit(1);
		}

		if (args.length > 0) {
			configFile = args[0];
		} else {
			if (wmSuccess) {
				String file = wm.promptForConfig();
				if (file != null) {
					configFile = file;
				} else {
					configFile = config.kDefaultXMLEatersSettingsFile;
				}
			} else {
				System.out.println("No configuration file specified, using " + config.kDefaultXMLEatersSettingsFile);
				configFile = config.kDefaultXMLEatersSettingsFile;
			}
		}
		
		// Read config file
		ConfigurationLoader configLoader = new ConfigurationLoader();
		if (!configLoader.load(configFile)) {
			wm.shutdown();
			System.exit(1);
		}
		config = configLoader.getConfig();
		
		logger.setLevel(config.logLevel);
		// Start logger
		if (config.logToFile) {
			try {
				FileHandler handler = new FileHandler(config.logFile.getAbsolutePath());
				handler.setLevel(config.logLevel);
				if (config.logTime) {
					handler.setFormatter(new TextFormatter());
				} else {
					handler.setFormatter(new NoTimeTextFormatter());
				}
				logger.addHandler(handler);
			} catch (IOException e) {
				control.severeError("IOException creating " + config.logToFile + ": " + e.getMessage());
				wm.shutdown();
				System.exit(1);
			}
		}
		if (config.logConsole) {
			// Console handler
			ConsoleHandler handler = new ConsoleHandler();
			handler.setLevel(config.logLevel);
			if (config.logTime) {
				handler.setFormatter(new TextFormatter());
			} else {
				handler.setFormatter(new NoTimeTextFormatter());
			}
			logger.addHandler(handler);
		}
		if (config.logToFile || config.logConsole) {
			logger.setUseParentHandlers(false);
		} else {
			System.out.println("Warning: not logging anything");
			logger.setLevel(Level.OFF);
		}
		
		if (config.graphical) {
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
		
		if (config.graphical) {
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
