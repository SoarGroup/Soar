package edu.umich.soar.gridmap2d.map;

import lcmtypes.pose_t;

public class RoomObject {
	private pose_t pose = new pose_t();
	private int area;
	private final CellObject object;
	
	public RoomObject(CellObject object) {
		this.object = object;
	}

	public int getArea() {
		return area;
	}

	public void setArea(int area) {
		this.area = area;
	}

	public void setPose(pose_t pose) {
		if (pose == null) {
			this.pose = null;
		}
		this.pose = pose.copy();
	}

	public pose_t getPose() {
		return pose.copy();
	}

	public CellObject getObject() {
		return object;
	}
}
