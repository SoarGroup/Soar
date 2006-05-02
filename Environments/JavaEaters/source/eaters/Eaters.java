package eaters;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import utilities.*;

import eaters.visuals.EatersWindowManager;

public class Eaters {
	
	public static final String kDefaultXMLSettingsFile = "eaters-default-settings.xml";
	private final String kDefaultFile = "EaterLog.txt";

	private boolean m_Quiet;
	private String m_SettingsFile;
	private boolean m_Console;
	private String m_LogFile;
	private boolean m_Append;
	private boolean m_NotRandom;

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
		if (!m_Console) {
			if (m_LogFile == null) {
				m_LogFile = kDefaultFile;
			}
			Logger.logger.toFile(m_LogFile, m_Append);
		}
		
		// Initialize the simulation
		EatersSimulation simulation = new EatersSimulation(m_SettingsFile, m_Quiet, m_NotRandom);
		
		// Initialize the window manager, if applicable.
		if(!m_Quiet) {
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

		m_Quiet = hasOption(args, "-quiet");
		m_SettingsFile = getOptionValue(args, "-settings");
		m_Console = hasOption(args, "-console");
		m_LogFile = getOptionValue(args, "-log");
		m_Append = hasOption(args, "-append");
		m_NotRandom = hasOption(args, "-notrandom");
	
		if (m_LogFile != null) {
			m_Console = false;
		}
		
		if (m_SettingsFile == null) {
			m_SettingsFile = kDefaultXMLSettingsFile;
		}
		
		return true;
	}
	
	protected void printCommandLineHelp() {
		System.out.println("Java Eaters help");
		System.out.println("\t-console: Send all log messages to console, overridden by -log.");
		System.out.println("\t-log: File name to log messages to (default: " + kDefaultFile + ").");
		System.out.println("\t-append: If logging to file, append.  Ignored if -console present.");
		System.out.println("\t-quiet: Disables all windows, runs simulation quietly.");
		System.out.println("\t-settings: XML file with with run settings.");
		System.out.println("\t-notrandom: Disable randomness by seeding the generator with 0.");
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
