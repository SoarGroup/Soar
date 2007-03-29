package soar2d.world;

import java.awt.Point;

public class Wall {
	int locationId = -1;
	Point leftExtent;
	Point rightExtent;
	int direction;
	
	Wall(int locationId, Point leftExtent, Point rightExtent, int direction) {
		this.locationId = locationId;
		this.leftExtent = leftExtent;
		this.rightExtent = rightExtent;
		this.direction = direction;
	}
	
	public int getLocationID() {
		return locationId;
	}
	
	public Point getLeftExtent() {
		return leftExtent;
	}

	public Point getRightExtent() {
		return rightExtent;
	}
	
	public int getDirection() {
		return direction;
	}
}
