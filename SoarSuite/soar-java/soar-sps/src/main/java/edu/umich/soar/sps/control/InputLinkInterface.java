package edu.umich.soar.sps.control;

import edu.umich.soar.robot.OffsetPose;


interface InputLinkInterface {
	void addWaypoint(double[] pos, String id, boolean useFloatYawWmes);
	void clearMessages();
	boolean disableWaypoint(String id);
	boolean enableWaypoint(String id, OffsetPose splinter);
	boolean removeMessage(int id);
	boolean removeWaypoint(String id);
}
