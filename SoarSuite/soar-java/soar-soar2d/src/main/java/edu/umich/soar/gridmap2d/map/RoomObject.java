package edu.umich.soar.gridmap2d.map;

import org.eclipse.swt.graphics.Color;

import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.visuals.WindowManager;
import edu.umich.soar.gridmap2d.world.RoomWorld;
import lcmtypes.pose_t;

public class RoomObject {
	private pose_t pose = new pose_t();
	private int area;
	private final int id;
	private final CellObject object;
	private Color color;
	
	public RoomObject(CellObject object, int id) {
		this.object = object;
		this.id = id;
		
		// TODO: make colors enum, use valueOf 
		String color = object.getProperty("color");
		if (color.equals("red")) {
			this.color = WindowManager.red;
		}
		else if (color.equals("green")) {
			this.color = WindowManager.green;
		} 
		else if (color.equals("gray")) {
			this.color = WindowManager.darkGray;
		}
	}
	
	void update(GridMapCells cells) {
		int[] location = object.getLocation();
		Cell container = cells.getCell(location);

		pose_t pose = new pose_t();
		pose.pos[0] = location[0] * RoomWorld.CELL_SIZE;
        pose.pos[1] = location[1] * RoomWorld.CELL_SIZE; 
        pose.pos[0] += RoomWorld.CELL_SIZE / 2.0;
        pose.pos[1] += RoomWorld.CELL_SIZE / 2.0;
        setPose(pose);
        setArea(container.getFirstObjectWithProperty(Names.kRoomID).getIntProperty(Names.kPropertyNumber, -1));
	}

	public int getArea() {
		return area;
	}

	public void setArea(int area) {
		this.area = area;
	}
	
	public int getId() {
		return id;
	}
	
	public void setPose(pose_t pose) {
		if (pose == null) {
			this.pose = null;
		}
		this.pose = pose.copy();
	}

	public pose_t getPose() {
		return pose.copy();
	}

	public CellObject getCellObject() {
		return object;
	}
	
	public Color getColor() {
		return color;
	}
}
