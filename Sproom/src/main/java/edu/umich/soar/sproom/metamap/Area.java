/**
 * 
 */
package edu.umich.soar.sproom.metamap;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;


public class Area {
	final int id;
	private final boolean door;
	final double[] pos;
	final double[] xySize;
	final List<Gateway> gateways = new ArrayList<Gateway>();
	final List<WallDir> dirs = new ArrayList<WallDir>();
	private final Walls walls;
	
	Area(int id, boolean door, double[] pos, double[] xySize) {
		this.id = id;
		this.door = door;
		this.pos = pos;
		this.xySize = xySize;
		
		this.walls = new Walls(pos, xySize);
	}
	
	public int getId() {
		return id;
	}
	
	public boolean isDoor() {
		return door;
	}
	
	public List<Gateway> getGateways() {
		return Collections.unmodifiableList(gateways);
	}
	
	public List<WallDir> getDirs() {
		return Collections.unmodifiableList(dirs);
	}
	
	public Walls getWalls() {
		return walls;
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder("Area ");
		sb.append(id);
		if (door) {
			sb.append(" (door)");
		}
		sb.append(String.format(" [%2.2f, %2.2f]", pos[0], pos[1]));
		sb.append(String.format(" [%2.2f, %2.2f]", xySize[0], xySize[1]));
		sb.append(" gateway ids:");
		for (int i = 0; i < gateways.size(); ++i) {
			sb.append(" ");
			sb.append(gateways.get(i).id);
			sb.append(dirs.get(i).toString().charAt(0));
		}
		return sb.toString();
	}
}