package org.msoar.sps;

public final class SharedNames {
	public static final String DRIVE_CHANNEL = "DIFFERENTIAL_DRIVE_COMMAND";
	public static final String POSE_CHANNEL = "POSE";
	public static final String LASER_CHANNEL = "LIDAR_FRONT";
	
	public enum ClientCommands { 
		OUTPUT, 		// output coming
		DONE, 			// process ended
		CLOSE; 			// close the connection
	}
	
	public enum ServerCommands { 
		COMMAND, 		// use this command line
		CONFIG, 		// use this configuration file
		ENVIRONMENT, 	// use this environment
		START, 			// start the process
		STOP, 			// stop the process
		CLOSE;			// close the connection
	}

	private SharedNames() {
		throw new AssertionError();
	}
}
