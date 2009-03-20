package org.msoar.gridmap2d;

public enum Game {
	TANKSOAR, 
	EATERS, 
	TAXI; 
//	KITCHEN, 
//	ROOM;
	
	public String id() {
		return this.toString().toLowerCase();
	}
}
