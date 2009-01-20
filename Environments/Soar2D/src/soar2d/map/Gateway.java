package soar2d.map;

import java.util.ArrayList;
import java.util.Arrays;

public class Gateway {

	ArrayList<Integer> locationIDs;
	int [] leftExtent;
	int [] rightExtent;

	Gateway(int [] leftExtent, int [] rightExtent) {
		this.leftExtent = Arrays.copyOf(leftExtent, leftExtent.length);
		this.rightExtent = Arrays.copyOf(rightExtent, rightExtent.length);
	}
	
	void addLocationID(int id) {
		locationIDs.add(id);
	}
	
	public ArrayList<Integer> getLocationIDs() {
		return locationIDs;
	}
}
