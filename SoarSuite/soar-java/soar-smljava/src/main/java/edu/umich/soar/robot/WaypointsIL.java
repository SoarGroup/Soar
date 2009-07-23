package edu.umich.soar.robot;

import java.util.HashMap;

import sml.Agent;
import sml.Identifier;

public final class WaypointsIL {
	private final Agent agent;
	private final Identifier waypoints;
	private final HashMap<String, WaypointIL> waypointList = new HashMap<String, WaypointIL>();

	public WaypointsIL(Agent agent, Identifier waypoints) {
		this.agent = agent;
		this.waypoints = waypoints;
	}

	public void add(double[] waypointxyz, String name, boolean useFloatWmes) {
		WaypointIL waypoint = waypointList.remove(name);
		if (waypoint != null) {
			waypoint.disable();
		}

		waypointList.put(name, new WaypointIL(agent, waypointxyz, name, waypoints, useFloatWmes));
	}

	public boolean remove(String name) {
		WaypointIL waypoint = waypointList.remove(name);
		if (waypoint == null) {
			return false;
		}
		waypoint.disable();
		return true;
	}

	public boolean enable(String name, OffsetPose splinter) {
		WaypointIL waypoint = waypointList.get(name);
		if (name == null) {
			return false;
		}

		waypoint.update(splinter);
		return true;
	}

	public boolean disable(String name) {
		WaypointIL waypoint = waypointList.get(name);
		if (name == null) {
			return false;
		}

		waypoint.disable();
		return true;
	}

	public void update(OffsetPose splinter) {
		for (WaypointIL waypoint : waypointList.values()) {
			waypoint.update(splinter);
		}
	}
}
