package edu.umich.soar.gridmap2d.soar;

import lcmtypes.pose_t;
import sml.FloatElement;
import sml.Identifier;
import sml.StringElement;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.robot.PointRelationship;

class SoarRobotObjectIL {
	private Identifier parent;
	private FloatElement distance;
	private FloatElement yaw;
	private FloatElement relativeBearing;
	private FloatElement absRelativeBearing;
	private FloatElement x, y, z;
	private StringElement visible;
	
	private int cycleTouched;
	
	SoarRobotObjectIL(Identifier parent) {
		this.parent = parent;
	}
	
	void initialize(pose_t pose, PointRelationship r) {
		this.visible = this.parent.CreateStringWME("visible", "yes");

		initInternal(pose, r);
	}
	
	void addProperty(String key, String value) {
		this.parent.CreateStringWME(key, value);
	}
		
	void addProperty(String key, int value) {
		this.parent.CreateIntWME(key, value);
	}
		
	void addProperty(String key, double value) {
		this.parent.CreateFloatWME(key, value);
	}
		
	private void initInternal(pose_t pose, PointRelationship r) {
		this.x = this.parent.CreateFloatWME("x", pose.pos[0]);
		this.y = this.parent.CreateFloatWME("y", pose.pos[1]);
		this.z = this.parent.CreateFloatWME("z", pose.pos[2]);

		this.distance = this.parent.CreateFloatWME("distance", r.getDistance());
		this.yaw = this.parent.CreateFloatWME("yaw", Math.toDegrees(r.getYaw()));
		this.relativeBearing = this.parent.CreateFloatWME("relative-bearing", Math.toDegrees(r.getRelativeBearing()));
		this.absRelativeBearing = this.parent.CreateFloatWME("abs-relative-bearing", Math.abs(Math.toDegrees(r.getRelativeBearing())));
		
		touch(Gridmap2D.simulation.getWorldCount());
	}
	
	void update(pose_t pose, PointRelationship r) {
		this.x.Update(pose.pos[0]);
		this.y.Update(pose.pos[1]);
		this.z.Update(pose.pos[2]);
		
		this.distance.Update(r.getDistance());
		this.yaw.Update(Math.toDegrees(r.getYaw()));
		this.relativeBearing.Update(Math.toDegrees(r.getRelativeBearing()));
		this.absRelativeBearing.Update(Math.abs(Math.toDegrees(r.getRelativeBearing())));
		
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
