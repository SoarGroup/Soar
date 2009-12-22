package edu.umich.soar.wp;

import java.util.HashMap;
import java.util.Map;

import lcmtypes.node_t;

public class Waypoints {
	private Map<String, node_t> all = new HashMap<String, node_t>();
	private Map<String, node_t> enabled = new HashMap<String, node_t>();

	public synchronized void createWaypoint(String id, node_t node) {
		if (id == null || node == null) {
			throw new NullPointerException();
		}
		all.put(id, node);
		enabled.put(id, node);
	}
	
	public synchronized boolean enableWaypoint(String id) {
		node_t waypoint = all.get(id);
		if (waypoint != null) {
			enabled.put(id, all.get(id));
			return true;
		}
		return false;
	}
	
	public synchronized boolean disableWaypoint(String id) {
		return enabled.remove(id) != null;
	}
	
	public synchronized boolean removeWaypoint(String id) {
		enabled.remove(id);
		return all.remove(id) != null;
	}
	
	public synchronized void clearWaypoints() {
		all.clear();
		enabled.clear();
	}
}
