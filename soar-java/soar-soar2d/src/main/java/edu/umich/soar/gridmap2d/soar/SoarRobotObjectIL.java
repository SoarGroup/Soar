package edu.umich.soar.gridmap2d.soar;

import lcmtypes.pose_t;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.players.RoomPlayer;

class SoarRobotObjectIL {
	private Identifier parent;
	
	private IntElement area;
	private Identifier position;
	private FloatElement angleOff;
	private FloatElement x, y;
	private StringElement visible;
	private FloatElement range;
	
	private int cycleTouched;
	
	SoarRobotObjectIL(Identifier parent) {
		this.parent = parent;
	}
	
	void initialize(RoomPlayer target, double range, double angleOffDouble) {
		parent.CreateStringWME("id", target.getName());
		parent.CreateStringWME("type", "player");
		this.area = parent.CreateIntWME("area", target.getState().getLocationId());
		this.angleOff = parent.CreateFloatWME("angle-off", angleOffDouble);
		this.position = parent.CreateIdWME("position");
		{
			this.x = position.CreateFloatWME("x", target.getState().getPose().pos[0]);
			this.y = position.CreateFloatWME("y", target.getState().getPose().pos[1]);
		}
		this.range = parent.CreateFloatWME("range", range);
		this.visible = parent.CreateStringWME("visible", "yes");
		
		touch(Gridmap2D.simulation.getWorldCount());
	}
	
	void initialize(int objectId, String type, pose_t targetPose, double range, double angleOffDouble) {
		parent.CreateIntWME("id", objectId);
		parent.CreateStringWME("type", type);
		this.angleOff = parent.CreateFloatWME("angle-off", angleOffDouble);
		this.position = parent.CreateIdWME("position");
		{
			this.x = position.CreateFloatWME("x", targetPose.pos[0]);
			this.y = position.CreateFloatWME("y", targetPose.pos[1]);
		}
		this.range = parent.CreateFloatWME("range", range);
		this.visible = parent.CreateStringWME("visible", "yes");
		
		touch(Gridmap2D.simulation.getWorldCount());
	}
		
	void update(int locationId, pose_t pose, double rangeValue, double angleOffValue) {
		area.Update(locationId);
		x.Update(pose.pos[0]);
		y.Update(pose.pos[1]);
		range.Update(rangeValue);
		angleOff.Update(angleOffValue);
		touch(Gridmap2D.simulation.getWorldCount());
	}
	
	void destroy() {
		parent.DestroyWME();
	}
	
	void touch(int cycle) {
		cycleTouched = cycle;
		if (visible.GetValue().equals("no")) {
			visible.Update("yes");
		}
	}
	
	int getCycleTouched() {
		return cycleTouched;
	}
	
	void makeInvisible() {
		if (visible.GetValue().equals("yes")) {
			visible.Update("no");
		}
	}

}
