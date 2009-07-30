package edu.umich.soar.robot;

import java.util.List;

public interface WaypointInterface {
	void addWaypoint(double[] pos, String id);
	boolean disableWaypoint(String id);
	boolean enableWaypoint(String id);
	boolean removeWaypoint(String id);
	List<double []> getWaypointList();
}
