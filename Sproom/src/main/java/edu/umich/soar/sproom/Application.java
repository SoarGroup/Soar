package edu.umich.soar.sproom;

import java.util.Arrays;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import lcm.spy.Spy;

//import lcm.spy.Spy;
import april.config.Config;
import april.config.ConfigUtil;
import april.sim.Simulator;
import april.viewer.Viewer;
import edu.umich.soar.sproom.comm.Messages;
import edu.umich.soar.sproom.command.Command;
import edu.umich.soar.sproom.splinter.Splinter;

/**
 * Top level main class for Sproom project.
 *
 * @author voigtjr@gmail.com
 */
public class Application {
	private static final Log logger = LogFactory.getLog(Application.class);

	private final ExecutorService exec = Executors.newSingleThreadExecutor();
	private final Messages messages = new Messages();
	
	public Application(String[] args) {
		Config config = ConfigUtil.getDefaultConfig(args);
		if (logger.isDebugEnabled()) {
			logger.debug("Config file:");
			for (String key : config.getKeys()) {
				logger.debug(String.format("%s = %s", key, Arrays.toString(config.getStrings(key))));
			}
		}
		
		new Viewer(config);
		if (config.getBoolean("mixed-mode", false)) {
			config.setStrings("simulator.simobjects", new String[] { "SIM_LIDAR_FRONT" });
			new Simulator(config);
			new Splinter(config);
		} else {
			new Simulator(config);
		}
		new Command(config, messages);
		
		if (config.getBoolean("lcm-spy.enabled", false)) {
			final String[] lcmArgs = config.getStrings("lcm-spy.args", new String[]{});
			exec.submit(new Runnable() {
				@Override
				public void run() {
					logger.info("Starting lcm-spy");
					Spy.main(lcmArgs);
				}
			});
		}
	}
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		new Application(args);
	}

}
