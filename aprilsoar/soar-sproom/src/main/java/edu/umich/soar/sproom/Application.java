package edu.umich.soar.sproom;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import lcm.spy.Spy;
import april.config.ConfigUtil;
import april.sim.Simulator;
import april.viewer.Viewer;
import edu.umich.soar.sproom.control.Controller;

public class Application {

	private Simulator sim;
	private Viewer viewer;
	private Controller controller;
	private final ExecutorService exec = Executors.newSingleThreadExecutor();
	
	public Application(String[] args) {
		sim = new Simulator(ConfigUtil.getDefaultConfig(args));
		viewer = new Viewer(ConfigUtil.getDefaultConfig(args));
		controller = new Controller(ConfigUtil.getDefaultConfig(args));
		exec.submit(new Runnable() {
			@Override
			public void run() {
				Spy.main(new String[]{});
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
