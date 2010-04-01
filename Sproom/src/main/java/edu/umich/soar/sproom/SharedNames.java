package edu.umich.soar.sproom;

/**
 * Shared string constants.
 * 
 * @author voigtjr
 */
public enum SharedNames {
	;
	
	private SharedNames() {
		throw new AssertionError();
	}

    public static final String DRIVE_CHANNEL = "DIFFERENTIAL_DRIVE_COMMAND";
    public static final String POSE_CHANNEL = "POSE";
    public static final String SIM_LASER_CHANNEL = "SIM_LIDAR_FRONT";
    public static final String SICK_LASER_CHANNEL = "SICK_LIDAR_FRONT";
    public static final String LASER_LOWRES_CHANNEL = "LIDAR_LOWRES";

	// i/o links
	public static final String ABS_RELATIVE_BEARING = "abs-relative-bearing";
	public static final String ANGLE_RESOLUTION = "angle-resolution";
	public static final String ANGLE_UNITS = "angle-units";
	public static final String ANGULAR = "angular";
	public static final String AREA_DESCRIPTION = "area-description";
	public static final String COLOR = "color";
	public static final String CONFIGURATION = "configuration";
	public static final String D = "d";
	public static final String DIRECTION = "direction";
	public static final String DISTANCE = "distance";
	public static final String DOOR = "door";
	public static final String FALSE = "false";
	public static final String FIELD_OF_VIEW = "field-of-view";
	public static final String FIRST = "first";
	public static final String FLOAT = "float";
	public static final String FROM = "from";
	public static final String GATEWAY = "gateway";
	public static final String HEADING = "heading";
	public static final String I = "i";
	public static final String ID = "id";
	public static final String INT = "int";
	public static final String LIDAR = "lidar";
	public static final String LINEAR = "linear";
	public static final String LENGTH_UNITS = "length-units";
	public static final String MANIPULATION_DISTANCE = "manipulation-distance";
	public static final String MESSAGE = "message";
	public static final String NEXT = "next";
	public static final String NIL = "nil";
	public static final String OBJECT = "object";
	public static final String OBJECTS = "objects";
	public static final String P = "p";
	public static final String POSE = "pose";
	public static final String POSE_TRANSLATION = "pose_translation";
	public static final String RANGE = "range";
	public static final String RANGE_COUNT = "range-count";
	public static final String RECEIVED_MESSAGES = "received-messages";
	public static final String RELATIVE_BEARING = "relative-bearing";
	public static final String ROOM = "room";
	public static final String SELF = "self";
	public static final String SPEED_UNITS = "speed-units";
	public static final String TIME = "time";
	public static final String TO = "to";
	public static final String TRUE = "true";
	public static final String TYPE = "type";
	public static final String VISIBLE = "visible";
	public static final String VISIBLE_TIME = "visible-time";
	public static final String WALL = "wall";
	public static final String WAYPOINT = "waypoint";
	public static final String WAYPOINTS = "waypoints";
	public static final String WORD = "waypoint";
	public static final String X = "x";
	public static final String X_VELOCITY = "x-velocity";
	public static final String Y = "y";
	public static final String Y_VELOCITY = "y-velocity";
	public static final String YAW = "yaw";
	public static final String YAW_VELOCITY = "yaw-velocity";
	public static final String Z = "z";
	public static final String Z_VELOCITY = "z-velocity";
}
