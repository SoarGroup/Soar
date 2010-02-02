package edu.umich.soar.sproom;

import net.java.games.input.Controller;
import net.java.games.input.ControllerEnvironment;

public class JoyTest {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		ControllerEnvironment ce = ControllerEnvironment.getDefaultEnvironment();
		
		Controller[] controllers = ce.getControllers();
		System.out.println("found " + controllers.length + " controllers");
	}

}
