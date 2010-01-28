package edu.umich.soar.room.map;

public class Points {
	private int points;
	
	void setPoints(Points points) {
		this.points = points.points;
	}
	
	public int getPoints() {
		return points;
	}
	
	void setPoints(int points) {
		this.points = points;
	}
	
	void adjustPoints(int delta) {
		this.points += delta;
	}
	
	void reset() {
		this.points = 0;
	}
	
	@Override
	public String toString() {
		return Integer.toString(this.points);
	}
}
