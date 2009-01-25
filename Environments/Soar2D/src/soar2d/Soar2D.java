package soar2d;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import soar2d.config.SimConfig;
import soar2d.visuals.WindowManager;

import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

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

public class Soar2D {
	private static Logger logger = Logger.getLogger(Soar2D.class);
	
	public static SimConfig config = null;
	public static final WindowManager wm = new WindowManager();
	public static final Simulation simulation = new Simulation();
	public static final Controller control = new Controller();

	public Soar2D(String[] args) {

		// Try to install default config files
		install(Names.configs.tanksoarCnf);
		install(Names.configs.tanksoarConsoleCnf);
		install(Names.configs.eatersCnf);
		install(Names.configs.eatersConsoleCnf);
		install(Names.configs.roomCnf);
		install(Names.configs.roomDiscreteCnf);
		install(Names.configs.kitchenCnf);
		install(Names.configs.taxiCnf);

		BasicConfigurator.configure();
		loadConfig(args);
		initializeLogger(config.loggingConfig());
		
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
		if (!simulation.initialize()) {
			fatalError(Names.Errors.simulationInitFail);
		}
		
		if (usingGUI) {
			// Run GUI
			logger.trace(Names.Trace.startGUI);
			control.runGUI();
		} else {
			// Run simulation
			logger.trace(Names.Trace.startSimulation);
			control.startSimulation(false, false);
		}
		
		// calls wm.shutdown()
		logger.trace(Names.Trace.shutdown);
		control.shutdown();
		
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
				fatalError(Names.Errors.loadingConfig + e.getMessage());
			}
		}
	}
	
	private void initializeLogger(String path) {
		if (path != null) {
			PropertyConfigurator.configure(path);
		}
		logger.trace(Names.Trace.loggerInitialized);
	}

	private void install(String file) {	
		try {
			// We just work relative to the current directory because that's how
			// load library will work.
			File library = new File(file) ;
	
			// library.exists() not working correctly in linux (!?)
			if (library.isFile() || library.isDirectory()) {
				//System.out.println("exists: " + library.exists() + ", isFile: " + library.isFile() + ", canRead: " + library.canRead());
				//System.out.println(library.getAbsolutePath() + " already exists so not installing from the JAR file");
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
			fatalError(Names.Errors.installingConfig + file + ": " + e.getMessage());
		}
	}

	public static void main(String[] args) {
		new Soar2D(args);
		
		// Ensure all threads clean up properly.  This is a bit heavy handed but helps ensure we are completely stopped.
		logger.trace(Names.Trace.exitErrorLevel + 0);
		System.exit(0) ;
	}

}
