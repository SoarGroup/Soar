package org.msoar.gridmap2d;

public enum Game {
	TANKSOAR, 
	EATERS; 
//	KITCHEN, 
//	TAXI, 
//	ROOM;
	
	public String id() {
		return this.toString().toLowerCase();
	}
}
