package edu.umich.soar.sproom.command;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import april.config.Config;

import edu.umich.soar.sproom.drive.Drive3;
import edu.umich.soar.sproom.soar.SoarInterface;

public class Command {
	private static final Log logger = LogFactory.getLog(Command.class);
	
	public static double clamp(double value, double min, double max) {
		value = Math.max(value, min);
		value = Math.min(value, max);
		return value;
	}

	private final Pose pose = new Pose();
	private final Drive3 drive3 = Drive3.newInstance(pose);
	private final Waypoints waypoints = new Waypoints();
	private final Comm comm = new Comm();
	private final Lidar lidar = new Lidar();
	private final MapMetadata metadata;
	private final SoarInterface soar;
	private final HttpController httpController = new HttpController();
	
	public Command(Config config) {
		logger.debug("Command started");

		metadata = new MapMetadata(config);
		soar = new SoarInterface(pose, waypoints, comm, lidar, metadata);
		httpController.addDriveListener(drive3);
		soar.addDriveListener(drive3);
		httpController.addSoarControlListener(soar);
	}
}
