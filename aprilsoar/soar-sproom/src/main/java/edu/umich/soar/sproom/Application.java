package edu.umich.soar.sproom;

import april.config.ConfigUtil;
import april.sim.Simulator;
import april.viewer.Viewer;
import edu.umich.soar.sproom.control.Controller;

public class Application {

	private Simulator sim;
	private Viewer viewer;
	private Controller controller;
	
	public Application(String[] args) {
		sim = new Simulator(ConfigUtil.getDefaultConfig(args));
		viewer = new Viewer(ConfigUtil.getDefaultConfig(args));
		controller = new Controller(ConfigUtil.getDefaultConfig(args));
	}
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		new Application(args);
	}

}
