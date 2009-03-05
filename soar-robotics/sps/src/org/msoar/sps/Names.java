package org.msoar.sps;

public final class Names {
	public static final String DRIVE_CHANNEL = "DIFFERENTIAL_DRIVE_COMMAND";
	public static final String POSE_CHANNEL = "POSE";
	public static final String LASER_CHANNEL = "LIDAR_FRONT";
	
	public static final String NET_OK = "ok";
	public static final String NET_CONFIGURE = "configure";
	public static final String NET_CONFIG_NO = "config-no";
	public static final String NET_CONFIG_YES = "config-yes";
	public static final String NET_ENVIRONMENT_NO = "environment-no";
	public static final String NET_ENVIRONMENT_YES = "environment-yes";
	public static final String NET_START = "start";
	public static final String NET_ALIVE = "alive";
	public static final String NET_STOP = "destroy";
	public static final String NET_QUIT = "quit";
	
	public static final Byte TYPE_COMPONENT = Byte.parseByte("0");
	public static final Byte TYPE_OUTPUT = Byte.parseByte("1");
	
	private Names() {
		assert false;
	}
}
