/**
 * 
 */
package edu.umich.soar.sproom.metamap;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Gateway {
	final int id;
	private final double[] pos;
	final List<Area> to = new ArrayList<Area>(2);
	
	Gateway(int id, double[] pos) {
		this.id = id;
		this.pos = pos;
	}
	
	public int getId() {
		return id;
	}
	
	public List<Area> getTo() {
		return Collections.unmodifiableList(to);
	}
	
	public double[] getPos() {
		return new double[] { pos[0], pos[1], 0 };
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder("Gateway ");
		sb.append(id);
		sb.append(" to:");
		for (Area area : to) {
			sb.append(" ");
			sb.append(area.id);
		}
		sb.append(String.format(" [%2.2f, %2.2f]", pos[0], pos[1]));
		return sb.toString();
	}
}