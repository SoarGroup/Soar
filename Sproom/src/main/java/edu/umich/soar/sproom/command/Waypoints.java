package edu.umich.soar.sproom.command;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

import lcmtypes.pose_t;

public class Waypoints implements Iterable<Waypoint> {
	private Map<String, Waypoint> all = new HashMap<String, Waypoint>();
	private Set<Waypoint> enabled = new CopyOnWriteArraySet<Waypoint>();

	public synchronized void createWaypoint(String id, String type, pose_t pose) {
		if (id == null || pose == null) {
			throw new NullPointerException();
		}
		Waypoint waypoint = new Waypoint(id, type, pose);
		
		all.put(waypoint.getId(), waypoint);
		enabled.add(waypoint);
	}
	
	public synchronized boolean enableWaypoint(String id) {
		Waypoint waypoint = all.get(id);
		if (waypoint != null) {
			return enabled.add(waypoint);
		}
		return false;
	}
	
	public synchronized boolean disableWaypoint(String id) {
		Waypoint waypoint = all.get(id);
		if (waypoint != null) {
			return enabled.remove(waypoint);
		}
		return false;
	}
	
	public synchronized boolean removeWaypoint(String id) {
		Waypoint waypoint = all.remove(id);
		if (waypoint != null) {
			enabled.remove(waypoint);
			return true;
		}
		return false;
	}
	
	public synchronized void clearWaypoints() {
		all.clear();
		enabled.clear();
	}
	
	@Override
	public Iterator<Waypoint> iterator() {
		return enabled.iterator();
	}
}
