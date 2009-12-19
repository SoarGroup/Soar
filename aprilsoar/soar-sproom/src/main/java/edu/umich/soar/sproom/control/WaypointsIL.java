package edu.umich.soar.sproom.control;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ConcurrentSkipListMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import lcm.lcm.LCM;
import lcmtypes.waypoint_list_t;
import lcmtypes.waypoint_t;

import sml.Identifier;

public class WaypointsIL implements WaypointInterface {
	private final Identifier waypoints;
	private final ConcurrentMap<String, WaypointIL> waypointList = new ConcurrentSkipListMap<String, WaypointIL>();
	private final OffsetPose opose;
	private final ConfigureInterface configure;
	private final ScheduledExecutorService schexec = Executors.newSingleThreadScheduledExecutor();
	private final LCM lcm = LCM.getSingleton();
	
	private final Runnable broadcaster = new Runnable() {
		public void run() {
			waypoint_list_t wplist = new waypoint_list_t();
			wplist.nwaypoints = 0;
			List<waypoint_t> temp = new ArrayList<waypoint_t>(waypointList.size());
			for (WaypointIL wpil : waypointList.values()) {
				if (wpil.isDisabled()) {
					continue;
				}
				wplist.nwaypoints += 1;
				waypoint_t wp = new waypoint_t();
				wp.xLocal = wpil.getPos()[0];
				wp.yLocal = wpil.getPos()[1];
				temp.add(wp);
			}
			wplist.waypoints = temp.toArray(new waypoint_t[temp.size()]);
			wplist.utime = System.nanoTime();
			lcm.publish("WAYPOINTS", wplist);
		}
	};

	public WaypointsIL(Identifier waypoints, OffsetPose opose, ConfigureInterface configure) {
		this.waypoints = waypoints;
		this.opose = opose;
		this.configure = configure;
		
		schexec.scheduleAtFixedRate(broadcaster, 1, 1, TimeUnit.SECONDS);
	}
	
	@Override
	public void addWaypoint(double[] waypointxyz, String name, String type) {
		WaypointIL waypoint = waypointList.remove(name);
		if (waypoint != null) {
			waypoint.disable();
		}

		waypointList.put(name, new WaypointIL(waypointxyz, name, type, waypoints, configure.isFloatYawWmes(), opose));
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
