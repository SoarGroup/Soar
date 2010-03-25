package edu.umich.soar.gridmap2d;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.prefs.BackingStoreException;
import java.util.prefs.Preferences;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.SoarProperties;
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
	
	public static final Preferences preferences;
	public static final SoarProperties soarProperties = new SoarProperties();
	
	public static SimConfig config = null;
	public static final WindowManager wm = new WindowManager();
	public static final Simulation simulation = new Simulation();
	public static final Controller control = new Controller();

	public static final String parent = System.getProperty("user.dir") + File.separator + "soar2d";

	static {
		final String VERSION = "version";
		Preferences temp = Preferences.userNodeForPackage(Gridmap2D.class);
		String version = temp.get(VERSION, null);
		if (version == null || !version.equals(soarProperties.getVersion())) {
			// different version
			try {
				temp.removeNode();
			} catch (BackingStoreException e) {
				e.printStackTrace();
			}
			temp = Preferences.userNodeForPackage(Gridmap2D.class);
			temp.put(VERSION, soarProperties.getVersion());
		}
		preferences = temp;
	}
	
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
				configPath = wm.promptForConfig(parent + File.separator + "config");
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

	// It would be nice if I could just include everything in soar2d, but I haven't found a soln for that yet
	// in SoarSuite/Soar2D/src/main/resources
	// find soar2d -type f | grep  -v ".svn" | sed 's/.*/"&",/'
	private static final String[] resources = {
		"soar2d/agents/eaters/counter example/counter/_firstload.soar",
		"soar2d/agents/eaters/counter example/counter/count.soar",
		"soar2d/agents/eaters/counter example/counter/counter.dm",
		"soar2d/agents/eaters/counter example/counter/counter_source.soar",
		"soar2d/agents/eaters/counter example/counter/elaborations/elaborations.soar",
		"soar2d/agents/eaters/counter example/counter/elaborations/elaborations_source.soar",
		"soar2d/agents/eaters/counter example/counter/init-count.soar",
		"soar2d/agents/eaters/counter example/counter.soar",
		"soar2d/agents/eaters/counter example/counter.vsa",
		"soar2d/agents/eaters/scripts/eswn.txt",
		"soar2d/agents/eaters/template/eater-default/_firstload.soar",
		"soar2d/agents/eaters/template/eater-default/all/all_source.soar",
		"soar2d/agents/eaters/template/eater-default/comment.dm",
		"soar2d/agents/eaters/template/eater-default/eater-default.dm",
		"soar2d/agents/eaters/template/eater-default/eater-default_source.soar",
		"soar2d/agents/eaters/template/eater-default/elaborations/_all.soar",
		"soar2d/agents/eaters/template/eater-default/elaborations/elaborations_source.soar",
		"soar2d/agents/eaters/template/eater-default/elaborations/top-state.soar",
		"soar2d/agents/eaters/template/eater-default.soar",
		"soar2d/agents/eaters/template/eater-default.vsa",
		"soar2d/agents/eaters/tutorial/advanced-move.soar",
		"soar2d/agents/eaters/tutorial/hello-world-operator.soar",
		"soar2d/agents/eaters/tutorial/hello-world-rule.soar",
		"soar2d/agents/eaters/tutorial/jump-and-move.soar",
		"soar2d/agents/eaters/tutorial/jump.soar",
		"soar2d/agents/eaters/tutorial/move-north-2.soar",
		"soar2d/agents/eaters/tutorial/move-north.soar",
		"soar2d/agents/eaters/tutorial/move-to-food.soar",
		"soar2d/agents/eaters/tutorial/move.soar",
		"soar2d/agents/eaters/tutorial/semantic-errors.soar",
		"soar2d/agents/eaters/tutorial/syntax-error.soar",
		"soar2d/agents/halt.soar",
		"soar2d/agents/tanksoar/obscure-bot.soar",
		"soar2d/agents/tanksoar/obscure-bot.soarx",
		"soar2d/agents/tanksoar/obscure-bot.txt",
		"soar2d/agents/tanksoar/template/tanksoar-default/_firstload.soar",
		"soar2d/agents/tanksoar/template/tanksoar-default/all/all_source.soar",
		"soar2d/agents/tanksoar/template/tanksoar-default/all/output.soar",
		"soar2d/agents/tanksoar/template/tanksoar-default/comment.dm",
		"soar2d/agents/tanksoar/template/tanksoar-default/elaborations/_all.soar",
		"soar2d/agents/tanksoar/template/tanksoar-default/elaborations/elaborations_source.soar",
		"soar2d/agents/tanksoar/template/tanksoar-default/elaborations/top-state.soar",
		"soar2d/agents/tanksoar/template/tanksoar-default/initialize-tanksoar-default.soar",
		"soar2d/agents/tanksoar/template/tanksoar-default/tanksoar-default.dm",
		"soar2d/agents/tanksoar/template/tanksoar-default/tanksoar-default_source.soar",
		"soar2d/agents/tanksoar/template/tanksoar-default.soar",
		"soar2d/agents/tanksoar/template/tanksoar-default.vsa",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/_firstload.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/all/all_source.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/all/remove-sound.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/all/wait.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/attack/attack_source.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/attack/fire-missile.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/attack/move.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/attack/slide.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/attack/turn.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/attack.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/chase/chase_source.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/chase/elaborations.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/chase/move.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/chase/radar.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/chase/turn.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/chase.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/comment.dm",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/elaborations/_all.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/elaborations/elaborations_source.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/elaborations/map.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/elaborations/monitor.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/elaborations/output.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/elaborations/shields.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/elaborations/top-state.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/init-map.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/init.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/mapping-bot.dm",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/mapping-bot_source.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/retreat/elaborations.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/retreat/move.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/retreat/retreat_source.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/retreat/wait.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/retreat.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/wander/elaborations.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/wander/move.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/wander/record-sound.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/wander/turn.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/wander/wander_source.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot/wander.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot.soar",
		"soar2d/agents/tanksoar/tutorial/mapping-bot.vsa",
		"soar2d/agents/tanksoar/tutorial/simple-bot/_firstload.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/all/all_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/all/wait.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/attack/attack_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/attack/fire-missile.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/attack/move.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/attack/slide.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/attack/turn.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/attack.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/chase/chase_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/chase/elaborations.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/chase/move.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/chase/radar.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/chase/turn.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/chase.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/comment.dm",
		"soar2d/agents/tanksoar/tutorial/simple-bot/elaborations/_all.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/elaborations/elaborations_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/elaborations/monitor.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/elaborations/output.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/elaborations/reward.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/elaborations/shields.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/elaborations/top-state.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/retreat/elaborations.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/retreat/move.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/retreat/retreat_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/retreat/wait.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/retreat.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/simple-bot.dm",
		"soar2d/agents/tanksoar/tutorial/simple-bot/simple-bot_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/wander/elaborations.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/wander/move.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/wander/turn.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/wander/wander_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot/wander.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot.soar",
		"soar2d/agents/tanksoar/tutorial/simple-bot.vsa",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/_firstload.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/all/all_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/all/remove-sound.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/all/wait.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/attack/attack_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/attack/fire-missile.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/attack/move.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/attack/slide.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/attack/turn.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/attack.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/chase/chase_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/chase/elaborations.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/chase/move.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/chase/radar.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/chase/turn.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/chase.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/comment.dm",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/elaborations/_all.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/elaborations/elaborations_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/elaborations/monitor.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/elaborations/output.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/elaborations/shields.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/elaborations/top-state.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/retreat/elaborations.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/retreat/move.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/retreat/retreat_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/retreat/wait.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/retreat.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/simple-sound-bot.dm",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/simple-sound-bot_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/wander/elaborations.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/wander/move.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/wander/record-sound.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/wander/turn.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/wander/wander_source.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot/wander.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot.soar",
		"soar2d/agents/tanksoar/tutorial/simple-sound-bot.vsa",
		"soar2d/agents/tanksoar/tutorial/wander.soar",
		"soar2d/agents/taxi/wait.soar",
		"soar2d/config/eaters.cnf",
		"soar2d/config/maps/eaters/boxes5.txt",
		"soar2d/config/maps/eaters/circle17.txt",
		"soar2d/config/maps/eaters/decay10.txt",
		"soar2d/config/maps/eaters/jump-bug6.txt",
		"soar2d/config/maps/eaters/jump17.txt",
		"soar2d/config/maps/eaters/negative-food10.txt",
		"soar2d/config/maps/eaters/objects/objects.txt",
		"soar2d/config/maps/eaters/open17.txt",
		"soar2d/config/maps/eaters/random-food17.txt",
		"soar2d/config/maps/eaters/random-walls.txt",
		"soar2d/config/maps/eaters/random10.txt",
		"soar2d/config/maps/eaters/random17.txt",
		"soar2d/config/maps/eaters/random6.txt",
		"soar2d/config/maps/eaters/reward-boxes7.txt",
		"soar2d/config/maps/eaters/short-corridor5.txt",
		"soar2d/config/maps/eaters/tiny.txt",
		"soar2d/config/maps/tanksoar/chunky.txt",
		"soar2d/config/maps/tanksoar/clumps.txt",
		"soar2d/config/maps/tanksoar/corridor.txt",
		"soar2d/config/maps/tanksoar/default.txt",
		"soar2d/config/maps/tanksoar/empty.txt",
		"soar2d/config/maps/tanksoar/objects/objects.txt",
		"soar2d/config/maps/tanksoar/rooms.txt",
		"soar2d/config/maps/tanksoar/sparse.txt",
		"soar2d/config/maps/tanksoar/x.txt",
		"soar2d/config/maps/taxi/default.txt",
		"soar2d/config/maps/taxi/objects/objects.txt",
		"soar2d/config/tanksoar.cnf",
		"soar2d/config/taxi.cnf",
	};
}
