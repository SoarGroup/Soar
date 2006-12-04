package soar2d;

import java.io.*;
import java.util.logging.*;

import soar2d.visuals.*;
import soar2d.xml.ConfigurationLoader;

public class Soar2D {
	String configFile = null;

	public static final Logger logger = Logger.getLogger("soar2d");
	public static Configuration config = new Configuration();
	public static final WindowManager wm = new WindowManager();
	public static final Simulation simulation = new Simulation();
	public static final Controller control = new Controller();

	public Soar2D(String[] args) {
		if (args.length > 0) {
			configFile = args[0];
		} else {
			configFile = config.kDefaultXMLSettingsFile;

			// Try to install default config file
			try {
				install(config.kDefaultXMLSettingsFile);
			} catch (IOException e) {
				System.err.println("IOException installing " + config.kDefaultXMLSettingsFile + ": " + e.getMessage());
				System.exit(1);
			}
		}
		
		// Read config file
		ConfigurationLoader configLoader = new ConfigurationLoader();
		if (!configLoader.load(configFile)) {
			System.exit(1);
		}
		config = configLoader.getConfig();
		
		// Start logger
		if (config.logFile) {
			try {
				FileHandler handler = new FileHandler(config.logFileName);
				handler.setFormatter(new TextFormatter());
				logger.addHandler(handler);
			} catch (IOException e) {
				System.err.println("IOException creating " + config.logFile + ": " + e.getMessage());
				System.exit(1);
			}
		}
		if (config.logConsole) {
			// Console handler
			ConsoleHandler handler = new ConsoleHandler();
			handler.setFormatter(new TextFormatter());
			logger.addHandler(handler);
		}
		if (config.logFile || config.logConsole) {
			logger.setUseParentHandlers(false);
			logger.setLevel(config.logLevel);
		} else {
			System.out.println("Warning: not logging anything");
			logger.setLevel(Level.OFF);
		}
		
		// Fire up view
		if (config.graphical) {
			if (!wm.initialize()) {
				System.exit(1);
			}
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
			control.startSimulation(false);
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
	}

}
