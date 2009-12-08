package edu.umich.soar.sproom.control;

import java.util.List;

public interface WaypointInterface {
	void addWaypoint(double[] pos, String id, String idType);
	boolean disableWaypoint(String id);
	boolean enableWaypoint(String id);
	boolean removeWaypoint(String id);
	List<double []> getWaypointList();
}
