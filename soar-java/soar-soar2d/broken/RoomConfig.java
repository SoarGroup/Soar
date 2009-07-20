package org.msoar.gridmap2d.config;


public class RoomConfig implements GameConfig {
	public boolean colored_rooms = false;
	public double speed = 16;
	public int cell_size = 16;
	public double vision_cone = Math.PI;
	public double rotate_speed = Math.PI / 4.0;
	public boolean blocks_block = true;
	public boolean continuous = true;
	public boolean zero_is_east = false;

	public String title() {
		return "Room";
	}
}
