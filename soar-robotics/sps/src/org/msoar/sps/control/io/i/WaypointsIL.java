package org.msoar.sps.control.io.i;

import java.util.HashMap;

import lcmtypes.pose_t;
import sml.Agent;
import sml.Identifier;

public class WaypointsIL {
	private Agent agent;
	private Identifier waypoints;

	private HashMap<String, WaypointIL> waypointList = new HashMap<String, WaypointIL>();

	WaypointsIL(Agent agent, Identifier waypoints) {
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

	public boolean enable(String name, pose_t pose) {
		WaypointIL waypoint = waypointList.get(name);
		if (name == null) {
			return false;
		}

		waypoint.update(pose);
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

	void update(pose_t pose) {
		for (WaypointIL waypoint : waypointList.values()) {
			waypoint.update(pose);
		}
	}
}
