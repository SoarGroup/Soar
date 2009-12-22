package edu.umich.soar.wp;

import lcmtypes.pose_t;

public class Waypoint {

	private final String id;
	private final String type;	
	private final pose_t pose;
	
	Waypoint(String id, String type, pose_t pose) {
		if (id == null || type == null) {
			throw new NullPointerException();
		}
		
		this.id = id;
		this.type = type;
		this.pose = pose.copy();
	}
	
	public String getId() {
		return id;
	}
	
	public String getType() {
		return type;
	}
	
	public pose_t getPose() {
		return pose.copy();
	}
	
	@Override
	public boolean equals(Object obj) {
		if (obj instanceof Waypoint) {
			return ((Waypoint) obj).id.equals(id);
		}
		return super.equals(obj);
	}
	
	@Override
	public int hashCode() {
		return id.hashCode();
	}
}
