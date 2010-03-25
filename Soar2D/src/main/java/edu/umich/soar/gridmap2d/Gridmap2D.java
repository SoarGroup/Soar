package edu.umich.soar.gridmap2d;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.prefs.Preferences;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.config.ParseError;
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
	private static final Log logger = LogFactory.getLog(Gridmap2D.class);
	
	public static Preferences preferences = Preferences.userNodeForPackage(Gridmap2D.class);
	
	public static SimConfig config = null;
	public static final WindowManager wm = new WindowManager();
	public static final Simulation simulation = new Simulation();
	public static final Controller control = new Controller();

	// find config -type f | grep  -v ".svn"
	private static final String[] resources = {
		"config/eaters.cnf",
		"config/maps/eaters/boxes5.txt",
		"config/maps/eaters/circle17.txt",
		"config/maps/eaters/decay10.txt",
		"config/maps/eaters/jump-bug6.txt",
		"config/maps/eaters/jump17.txt",
		"config/maps/eaters/negative-food10.txt",
		"config/maps/eaters/objects/objects.txt",
		"config/maps/eaters/open17.txt",
		"config/maps/eaters/random-food17.txt",
		"config/maps/eaters/random-walls.txt",
		"config/maps/eaters/random10.txt",
		"config/maps/eaters/random17.txt",
		"config/maps/eaters/random6.txt",
		"config/maps/eaters/reward-boxes7.txt",
		"config/maps/eaters/short-corridor5.txt",
		"config/maps/eaters/tiny.txt",
		"config/maps/tanksoar/chunky.txt",
		"config/maps/tanksoar/clumps.txt",
		"config/maps/tanksoar/corridor.txt",
		"config/maps/tanksoar/default.txt",
		"config/maps/tanksoar/empty.txt",
		"config/maps/tanksoar/objects/objects.txt",
		"config/maps/tanksoar/rooms.txt",
		"config/maps/tanksoar/sparse.txt",
		"config/maps/tanksoar/x.txt",
		"config/maps/taxi/default.txt",
		"config/maps/taxi/objects/objects.txt",
		"config/tanksoar.cnf",
		"config/taxi.cnf",
	};
	private static final String parent = System.getProperty("user.dir") + File.separator + "config";

	public Gridmap2D(String[] args) {
		boolean installed = false;
		for (String s : resources) {
			install(s);
			installed = true;
		}
		if (installed) {
			logger.info("Installed resources from jar to working directory " + System.getProperty("user.dir"));
		}
		
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
		World world = simulation.initialize(config);
		
		if (usingGUI) {
			// Run GUI
			logger.trace(Names.Trace.startGUI);
			control.runGUI(world);
		} else {
			// Run simulation
			logger.trace(Names.Trace.startSimulation);
			control.startSimulation(false, false);
		}
		
		// calls wm.shutdown()
		logger.trace(Names.Trace.shutdown);
		control.shutdown();
	}
	
	private void fatalError(String message) {
		logger.fatal(message);
		System.err.println(message);
		if (wm.initialize()) {
			String title = "Soar2D";
			if (config != null && config.title() != null) {
				title = config.title();
			}
			wm.errorMessage(title, message);
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
				configPath = wm.promptForConfig(parent);
			} else {
				fatalError(Names.Errors.noConfig);
			}
		}
		
		// can be null if canceled in gui prompt
		if (configPath == null) {
			fatalError(Names.Errors.noConfig);
		}
		
		// See if it exists:
		File configFile = new File(configPath);
		if (!configFile.exists()) {
			fatalError("Couldn't find " + configPath);
		}
		
		if (!configFile.isFile()) {
			fatalError("Not a file: " + configPath);
		}

		// Read config file
		try {
			config = SimConfig.newInstance(configPath);
		} catch (IOException e) {
			fatalError("Error loading configuration file: " + e.getMessage());
		} catch (ParseError e) {
			fatalError("Error loading configuration file: " + e.getMessage());
		} catch (IllegalArgumentException e) {
			fatalError("Error loading configuration file: " + e.getMessage());
		}
		assert config != null;
	}
	
	private void install(String file) {	
		try {
			File r = new File(file) ;
			if (r.exists()) return;
			
			String jarpath = "/" + file;
			InputStream is = this.getClass().getResourceAsStream(jarpath) ;
			if (is == null) {
				logger.error("Resource not found: " + jarpath) ;
				return;
			}
			
			// Create the new file on disk
			File parent = r.getParentFile();
			if (parent != null)
			{
				if (!parent.exists())
					parent.mkdirs();
			}
			FileOutputStream os = new FileOutputStream(r) ;
			
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
			fatalError(Names.Errors.installingResource + file + ": " + e.getMessage());
		}
	}

	public static void main(String[] args) {
		new Gridmap2D(args);
		
		// Ensure all threads clean up properly.  This is a bit heavy handed but helps ensure we are completely stopped.
		logger.trace(Names.Trace.exitErrorLevel + 0);
		System.exit(0) ;
	}

}
