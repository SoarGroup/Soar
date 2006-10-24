package eaters;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.logging.*;

import utilities.*;

import eaters.visuals.EatersWindowManager;

public class Eaters {
	private static Logger logger = Logger.getLogger("simulation");
	
	public static final String kDefaultXMLSettingsFile = "eaters-default-settings.xml";
	private final String kDefaultLogFilename = "EaterLog.txt";

	private Level logLevel = Level.INFO;
	private boolean quietSwitch;
	private String settingsFilename;
	private String logFilename;
	//private boolean appendSwitch;
	private boolean notRandomSwitch;
	private boolean remoteSwitch;
	
	public Eaters(String[] args) {
		
		// Install default settings file
		try {
			Install(kDefaultXMLSettingsFile);
		} catch (IOException e) {
			System.out.println("Error installing default settings: " + e.getMessage());
			System.exit(1);
		}
		
		// Deal with the command line
		if (!parseCommandLine(args)) {
			return;
		}
		
		// Initialize logger
		// TODO: Append switch
		if (logFilename == null) {
			logFilename = kDefaultLogFilename;
		}
		try {
			FileHandler handler = new FileHandler(logFilename);
			handler.setFormatter(new JonsFormatter());
			logger.addHandler(handler);
		} catch (IOException e) {
			System.err.println("Failed to create " + logFilename + ": " + e.getMessage());
			System.exit(1);
		}
//		// Console handler
//		ConsoleHandler handler = new ConsoleHandler();
//		handler.setLevel(Level.ALL);
//		rootLogger.addHandler(handler);
		
		// TODO: set log level via command line
		logger.setUseParentHandlers(false);
		logger.setLevel(logLevel);
		logger.info("Java Eaters started.");
		
		// Initialize the simulation
		if (logger.isLoggable(Level.FINE)) logger.fine("Initializing simulation.");
		EatersSimulation simulation = new EatersSimulation(settingsFilename, quietSwitch, notRandomSwitch, remoteSwitch);
		
		// Initialize the window manager, if applicable.
		if(!quietSwitch) {
			new EatersWindowManager(simulation);
		}
		System.exit(0);
	}

	private void Install(String file) throws IOException {	
		// We just work relative to the current directory because that's how
		// load library will work.
		File library = new File(file) ;

		if (library.exists()) {
			System.out.println(library + " already exists so not installing from the JAR file") ;
			return;
		}
		
		// Get the DLL from inside the JAR file
		// It should be placed at the root of the JAR (not in a subfolder)
		String jarpath = "/" + library.getPath() ;
		InputStream is = this.getClass().getResourceAsStream(jarpath) ;
		
		if (is == null) {
			System.out.println("Failed to find " + jarpath + " in the JAR file") ;
			return;
		}
		
		// Make sure we can delete the library.  This is actually here to cover the
		// case where we're running in Eclipse without a JAR file.  The getResourceAsStream()
		// call can end up loading the same library that we're trying to save to and we
		// end up with a blank file.  Explicitly trying to delete it first ensures that
		// we're not reading the same file that we're writing.
		if (library.exists() && !library.delete()) {
			System.out.println("Failed to remove the existing layout file " + library) ;
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

	public boolean parseCommandLine(String[] args) {
		if (hasOption(args, "-?") || hasOption(args, "-help") || hasOption(args, "-h")) {
			printCommandLineHelp();
			return false;
		}

		quietSwitch = hasOption(args, "-quiet");
		settingsFilename = getOptionValue(args, "-settings");
		logFilename = getOptionValue(args, "-log");
		//appendSwitch = hasOption(args, "-append");
		notRandomSwitch = hasOption(args, "-notrandom");
		remoteSwitch = hasOption(args, "-remote");
	
		if (hasOption(args, "-fine")) {
			logLevel = Level.FINE;
		} 
		if (hasOption(args, "-finer")) {
			logLevel = Level.FINER;
		} 
		if (hasOption(args, "-finest")) {
			logLevel = Level.FINEST;
		} 

		if (settingsFilename == null) {
			settingsFilename = kDefaultXMLSettingsFile;
		}
		
		return true;
	}
	
	protected void printCommandLineHelp() {
		System.out.println("Java Eaters help");
		System.out.println("\t-log: File name to log messages to (default: " + kDefaultLogFilename + ").");
		//System.out.println("\t-append: If logging to file, append.  Ignored if -console present.");
		System.out.println("\t-quiet: Disables all windows, runs simulation quietly.");
		System.out.println("\t-settings: XML file with with run settings.");
		System.out.println("\t-notrandom: Disable randomness by seeding the generator with 0.");
		System.out.println("\t-remote: Connect to remote kernel.");
	}
	
	// Returns true if a given option appears in the list
	// (Use this for simple flags like -remote)
	protected boolean hasOption(String[] args, String option) {
		for (int i = 0 ; i < args.length ; i++){
			if (args[i].equalsIgnoreCase(option))
				return true ;
		}
		return false ;
	}	
	
	// Returns the next argument after the matching option.
	// (Use this for parameters like -port ppp)
	protected String getOptionValue(String[] args, String option) {
		for (int i = 0 ; i < args.length-1 ; i++) {
			if (args[i].equalsIgnoreCase(option))
				return args[i+1] ;
		}		
		return null ;
	}

	public static void main(String[] args) {
		new Eaters(args);
	}
}
