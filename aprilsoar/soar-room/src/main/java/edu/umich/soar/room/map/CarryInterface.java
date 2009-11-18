package edu.umich.soar.room.map;


public interface CarryInterface {
	public boolean hasObject();
	public RoomObject getRoomObject();
	public boolean isMalfunctioning();	// TODO: doesn't really belong here.
}
