package org.msoar.sps.control;

import lcmtypes.pose_t;

interface InputLinkInterface {
	void addWaypoint(double[] pos, String id, boolean useFloatYawWmes);
	void clearMessages();
	boolean disableWaypoint(String id);
	boolean enableWaypoint(String id, pose_t pose);
	boolean removeMessage(int id);
	boolean removeWaypoint(String id);
}
