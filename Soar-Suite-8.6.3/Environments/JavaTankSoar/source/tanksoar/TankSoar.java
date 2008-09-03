package tanksoar;

import java.io.*;
import java.util.logging.*;

import tanksoar.visuals.*;
import utilities.*;

public class TankSoar {
	
	public static final String kDefaultXMLSettingsFile = "tanksoar-default-settings.xml";
	private final String kDefaultLogFilename = "TankSoarLog.txt";

	private boolean quietSwitch;
	private String settingsFilename;
	private String logFilename;
	private boolean notRandomSwitch;
	private boolean noLogSwitch;
	private Level logLevel = Level.INFO;
	private boolean remoteSwitch;
	
	private static Logger logger = Logger.getLogger("simulation");

	public TankSoar(String[] args) {
		
		// Deal with the command line
		parseCommandLine(args);
		
		// Initialize logger
		// TODO: Append switch
		if (!noLogSwitch) {
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

			logger.setUseParentHandlers(false);
			logger.setLevel(logLevel);
			logger.info("Java TankSoar started (logging " + logger.getLevel().getName() + " and up).");
			String commandLine = new String();
			for (int i = 0; i < args.length; ++i) {
				commandLine += args[i];
				commandLine += " ";
			}
			if (logger.isLoggable(Level.FINER)) logger.finer("Command line: " + commandLine);
		}

		// Install default settings file if it doesn't exist
		try {
			Install(kDefaultXMLSettingsFile);
		} catch (IOException e) {
			logger.severe("Error installing default settings: " + e.getMessage());
			System.exit(1);
		}
		
		
		// Initialize the simulation
		if (logger.isLoggable(Level.FINER)) logger.finer("Initializing simulation.");
		TankSoarSimulation simulation = new TankSoarSimulation(settingsFilename, quietSwitch, notRandomSwitch, remoteSwitch);
		
		// Initialize the window manager, if applicable.
		if(!quietSwitch) {
			if (logger.isLoggable(Level.FINER)) logger.finer("Initializing window manager.");
			new TankSoarWindowManager(simulation);
		}
		
		if (logger.isLoggable(Level.FINER)) logger.finer("Exiting.");
		System.exit(0);
	}

	private void Install(String file) throws IOException {	
		// We just work relative to the current directory because that's how
		// load library will work.
		File library = new File(file) ;

		if (library.exists()) {
			if (logger.isLoggable(Level.FINER)) logger.finer(library + " already exists so not installing from the JAR file");
			return;
		}
		
		String jarpath = "/" + library.getPath() ;
		InputStream is = this.getClass().getResourceAsStream(jarpath) ;
		
		if (is == null) {
			logger.severe("Failed to find " + jarpath + " in the JAR file") ;
			System.exit(1);
		}
		
		// Make sure we can delete the library.  This is actually here to cover the
		// case where we're running in Eclipse without a JAR file.  The getResourceAsStream()
		// call can end up loading the same library that we're trying to save to and we
		// end up with a blank file.  Explicitly trying to delete it first ensures that
		// we're not reading the same file that we're writing.
		if (library.exists() && !library.delete()) {
			logger.severe("Failed to remove the existing layout file " + library) ;
			System.exit(1);
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
		
		if (logger.isLoggable(Level.FINER)) logger.finer("Installed " + library + " onto the local disk from JAR file") ;
	}

	public void parseCommandLine(String[] args) {
		if (hasOption(args, "-?") || hasOption(args, "-help") || hasOption(args, "-h")) {
			printCommandLineHelp();
			System.exit(0);
		}

		quietSwitch = hasOption(args, "-quiet");
		settingsFilename = getOptionValue(args, "-settings", kDefaultXMLSettingsFile);
		logFilename = getOptionValue(args, "-log", kDefaultLogFilename);
		notRandomSwitch = hasOption(args, "-notrandom");
		noLogSwitch = hasOption(args, "-nolog");
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
	}
	
	protected void printCommandLineHelp() {
		System.out.println("Java TankSoar help");
		System.out.println("\t-quiet: Disables all windows, runs simulation quietly.");
		System.out.println("\t-settings: XML file with with run settings (default: " + kDefaultXMLSettingsFile + ").");
		System.out.println("\t-notrandom: Disable randomness by seeding the generator with 0.");

		System.out.println("\t-log: File name to log messages to (default: " + kDefaultLogFilename + ").");
		System.out.println("\t-nolog: Disable logging (default: logging enabled)");
		System.out.println("\t-fine: Log verbosely");
		System.out.println("\t-finer: Log very verbosely");
		System.out.println("\t-finest: Log extremely verbosely (use at own risk!)");
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
	protected String getOptionValue(String[] args, String option, String defaultValue) {
		for (int i = 0 ; i < args.length-1 ; i++) {
			if (args[i].equalsIgnoreCase(option))
				return args[i+1] ;
		}		
		return defaultValue ;
	}

	public static void main(String[] args) {
		new TankSoar(args);
	}
}
