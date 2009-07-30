package edu.umich.soar.robot;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import sml.Identifier;

public final class WaypointsIL implements WaypointInterface {
	private final Identifier waypoints;
	private final HashMap<String, WaypointIL> waypointList = new HashMap<String, WaypointIL>();
	private final OffsetPose opose;
	private final ConfigureInterface configure;
	
	public WaypointsIL(Identifier waypoints, OffsetPose opose, ConfigureInterface configure) {
		this.waypoints = waypoints;
		this.opose = opose;
		this.configure = configure;
	}

	@Override
	public void addWaypoint(double[] waypointxyz, String name) {
		WaypointIL waypoint = waypointList.remove(name);
		if (waypoint != null) {
			waypoint.disable();
		}

		waypointList.put(name, new WaypointIL(waypointxyz, name, waypoints, configure.isFloatYawWmes(), opose));
	}

	@Override
	public boolean removeWaypoint(String name) {
		WaypointIL waypoint = waypointList.remove(name);
		if (waypoint == null) {
			return false;
		}
		waypoint.disable();
		return true;
	}

	@Override
	public boolean enableWaypoint(String name) {
		WaypointIL waypoint = waypointList.get(name);
		if (name == null) {
			return false;
		}

		waypoint.update();
		return true;
	}

	@Override
	public boolean disableWaypoint(String name) {
		WaypointIL waypoint = waypointList.get(name);
		if (name == null) {
			return false;
		}

		waypoint.disable();
		return true;
	}

	public void update() {
		for (WaypointIL waypoint : waypointList.values()) {
			waypoint.update();
		}
	}

	@Override
	public List<double[]> getWaypointList() {
		List<double[]> list = new ArrayList<double[]>(waypointList.size());
		for (WaypointIL wp : this.waypointList.values()) {
			list.add(wp.getPos());
		}
		return list;
	}
}
