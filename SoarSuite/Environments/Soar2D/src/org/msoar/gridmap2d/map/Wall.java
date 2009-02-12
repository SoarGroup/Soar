package org.msoar.gridmap2d.map;

public class Wall {
	int locationId = -1;
	int [] leftExtent;
	int [] rightExtent;
	int direction;
	
	Wall(int locationId, int [] leftExtent, int [] rightExtent, int direction) {
		this.locationId = locationId;
		this.leftExtent = leftExtent;
		this.rightExtent = rightExtent;
		this.direction = direction;
	}
	
	public int getLocationID() {
		return locationId;
	}
	
	public int [] getLeftExtent() {
		return leftExtent;
	}

	public int [] getRightExtent() {
		return rightExtent;
	}
	
	public int getDirection() {
		return direction;
	}
}
