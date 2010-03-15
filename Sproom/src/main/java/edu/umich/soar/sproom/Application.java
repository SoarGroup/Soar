package edu.umich.soar.sproom;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

//import lcm.spy.Spy;
import april.config.ConfigUtil;
import april.sim.Simulator;
import april.viewer.Viewer;
import edu.umich.soar.sproom.command.Command;

/**
 * Top level main class for Sproom project.
 *
 * @author voigtjr@gmail.com
 */
public class Application {

	private final ExecutorService exec = Executors.newSingleThreadExecutor();
	
	public Application(String[] args) {
		new Simulator(ConfigUtil.getDefaultConfig(args));
		new Viewer(ConfigUtil.getDefaultConfig(args));
		new Command(ConfigUtil.getDefaultConfig(args));
		exec.submit(new Runnable() {
			@Override
			public void run() {
				//Spy.main(new String[]{});
			}
		});
	}
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		new Application(args);
	}

}
