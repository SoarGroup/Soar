package edu.umich.soar.sproom.command;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.drive.Drive3;

public class Command {
	private static final Log logger = LogFactory.getLog(Command.class);
	
	public static double clamp(double value, double min, double max) {
		value = Math.max(value, min);
		value = Math.min(value, max);
		return value;
	}

	private final Pose pose = new Pose();
	private final Drive3 drive3 = Drive3.newInstance(pose);
	private final SoarInterface soar = new SoarInterface(pose);
	private final HttpController httpController = new HttpController();
	
	public Command() {
		logger.debug("Command started");

		httpController.addDriveListener(drive3);
		soar.addDriveListener(drive3);
		httpController.addSoarControlListener(soar);
	}
}
