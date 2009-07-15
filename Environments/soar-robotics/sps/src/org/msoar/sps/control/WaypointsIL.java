package org.msoar.sps.control;

import java.util.HashMap;

import sml.Agent;
import sml.Identifier;

final class WaypointsIL {
	private final Agent agent;
	private final Identifier waypoints;
	private final HashMap<String, WaypointIL> waypointList = new HashMap<String, WaypointIL>();

	WaypointsIL(Agent agent, Identifier waypoints) {
		this.agent = agent;
		this.waypoints = waypoints;
	}

	void add(double[] waypointxyz, String name, boolean useFloatWmes) {
		WaypointIL waypoint = waypointList.remove(name);
		if (waypoint != null) {
			waypoint.disable();
		}

		waypointList.put(name, new WaypointIL(agent, waypointxyz, name, waypoints, useFloatWmes));
	}

	boolean remove(String name) {
		WaypointIL waypoint = waypointList.remove(name);
		if (waypoint == null) {
			return false;
		}
		waypoint.disable();
		return true;
	}

	boolean enable(String name, SplinterState splinter) {
		WaypointIL waypoint = waypointList.get(name);
		if (name == null) {
			return false;
		}

		waypoint.update(splinter);
		return true;
	}

	boolean disable(String name) {
		WaypointIL waypoint = waypointList.get(name);
		if (name == null) {
			return false;
		}

		waypoint.disable();
		return true;
	}

	void update(SplinterState splinter) {
		for (WaypointIL waypoint : waypointList.values()) {
			waypoint.update(splinter);
		}
	}
}
