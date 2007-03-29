package soar2d.world;

import java.awt.Point;
import java.util.ArrayList;

public class Door {

	ArrayList<Integer> locationIDs;
	Point leftExtent;
	Point rightExtent;

	Door(Point leftExtent, Point rightExtent) {
		this.leftExtent = new Point(leftExtent);
		this.rightExtent = new Point(rightExtent);
	}
	
	void addLocationID(int id) {
		locationIDs.add(id);
	}
	
	public ArrayList<Integer> getLocationIDs() {
		return locationIDs;
	}
}
