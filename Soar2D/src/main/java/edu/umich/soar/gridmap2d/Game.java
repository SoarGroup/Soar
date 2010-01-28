package edu.umich.soar.gridmap2d;

public enum Game {
	TANKSOAR, 
	EATERS, 
	TAXI;
	
	public String id() {
		return this.toString().toLowerCase();
	}
}
